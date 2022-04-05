#include <lsqecc/patches/patches.hpp>
#include <lsqecc/patches/patch_computation.hpp>
#include <lsqecc/layout/router.hpp>

#include <lstk/lstk.hpp>

#include <stdexcept>
#include <iterator>
#include <ranges>
#include <iostream>
#include <sstream>



namespace lsqecc {


std::optional<Cell> PatchComputation::find_place_for_magic_state(size_t distillation_region_idx) const
{
    for(const auto& cell: layout_->distilled_state_locations(distillation_region_idx))
        if(is_cell_free_[cell.row][cell.col])
            return cell;

    return std::nullopt;
}


Slice first_slice_from_layout(const Layout& layout, const tsl::ordered_set<PatchId>& core_qubit_ids)
{
    Slice slice{.qubit_patches={}, .routing_regions={}, .layout={layout}, .time_to_next_magic_state_by_distillation_region={}};

    for (const Patch& p : layout.core_patches())
        slice.qubit_patches.push_back(p);

    size_t distillation_time_offset = 0;
    for(auto t : layout.distillation_times())
        slice.time_to_next_magic_state_by_distillation_region.push_back(t+distillation_time_offset++);


    // Map initial patches to ids
    if (slice.qubit_patches.size()<core_qubit_ids.size())
        throw std::logic_error("Not enough Init patches for all ids");


    auto patch_itr = slice.qubit_patches.begin();
    for (const auto& id: core_qubit_ids)
    {
        (patch_itr++)->id = id;
    }

    return slice;
}


Slice& PatchComputation::make_new_slice()
{

    slice_visitor_(slice_store_.last_slice());

    // This is an expensive copy. To avoid doing it twice we do in in place on the heap
    // TODO avoid this copy by doing a move and updating things

    Slice new_slice{
        .qubit_patches = {},
        .unbound_magic_states = slice_store_.last_slice().unbound_magic_states,
        .layout=*layout_, // TODO should be able to take out
        .time_to_next_magic_state_by_distillation_region={}};

    const Slice& old_slice = slice_store_.last_slice();

    // Copy patches over
    for (const auto& old_patch : old_slice.qubit_patches)
    {
        // Skip patches that were measured in the previous timestep
        if(old_patch.activity!=PatchActivity::Measurement)
        {
            new_slice.qubit_patches.push_back(old_patch);
            auto& new_patch = new_slice.qubit_patches.back();

            // Clear Unitary Operator activity
            if (new_patch.activity==PatchActivity::Unitary)
                new_patch.activity = PatchActivity::None;

            new_patch.visit_individual_cells([](SingleCellOccupiedByPatch& c)
            {
               c.top.is_active = false;
               c.bottom.is_active = false;
               c.left.is_active = false;
               c.right.is_active = false;
            });
        }
        else
        {
            for (const Cell &cell : old_patch.get_cells())
                is_cell_free_[cell.row][cell.col] = true;
        }
    }


    // Make magic states appear:
    for (size_t i = 0; i<old_slice.time_to_next_magic_state_by_distillation_region.size(); ++i)
    {
        new_slice.time_to_next_magic_state_by_distillation_region.push_back(
                old_slice.time_to_next_magic_state_by_distillation_region[i]-1);
        if(new_slice.time_to_next_magic_state_by_distillation_region.back() == 0){

            auto magic_state_cell = find_place_for_magic_state(i);
            if(magic_state_cell)
            {
                Patch magic_state_patch = LayoutHelpers::basic_square_patch(*magic_state_cell);
                magic_state_patch.type = PatchType::PreparedState;
                new_slice.unbound_magic_states.push_back(magic_state_patch);
                is_cell_free_[magic_state_cell->row][magic_state_cell->col] = false;
            }
#if false
            else
            {
                std::cout<< "Could not find place for magic state produced by distillation region " << i <<std::endl;
            }
#endif
            new_slice.time_to_next_magic_state_by_distillation_region.back() = layout_->distillation_times()[i];
        }
    }
    slice_store_.accept_new_slice(std::move(new_slice));
    return slice_store_.last_slice();
}



std::optional<Cell> find_free_ancilla_location(const Layout& layout, const Slice& slice)
{
    for(const Cell& possible_ancilla_location : layout.ancilla_location())
        if(slice.is_cell_free(possible_ancilla_location))
            return possible_ancilla_location;
    return std::nullopt;
}


void stitch_boundaries(Slice& slice, PatchId source, PatchId target, RoutingRegion& routing_region)
{
    auto& source_patch = slice.get_single_cell_occupied_by_patch_by_id_mut(source);
    auto& target_patch = slice.get_single_cell_occupied_by_patch_by_id_mut(target);

    if (routing_region.cells.empty())
    {
        source_patch.get_mut_boundary_with(target_patch.cell)->get().is_active=true;
        target_patch.get_mut_boundary_with(source_patch.cell)->get().is_active=true;
    }
    else
    {
        source_patch.get_mut_boundary_with(routing_region.cells.back().cell)->get().is_active=true;
        target_patch.get_mut_boundary_with(routing_region.cells.front().cell)->get().is_active=true;
    }
}

// TODO this file ha several helpers like this one, that could be moved to a separate file
/*
 * Returns true iff merge was susccesful
 */
bool merge_patches(
        Slice& slice,
        Router& router,
        PatchId source,
        PauliOperator source_op,
        PatchId target,
        PauliOperator target_op)
{

    if(slice.get_patch_by_id(source).activity != PatchActivity::None
    || slice.get_patch_by_id(source).activity != PatchActivity::None)
        return false;

    auto source_occupied_cell = slice.get_single_cell_occupied_by_patch_by_id(source);
    auto target_occupied_cell = slice.get_single_cell_occupied_by_patch_by_id(target);
    if(source_occupied_cell.has_active_boundary() || target_occupied_cell.has_active_boundary())
        return false;

    auto routing_region = router.find_routing_ancilla(slice, source, source_op, target, target_op);
    if(!routing_region)
        return false;


    stitch_boundaries(slice, source, target, *routing_region);
    if(!routing_region->cells.empty())
        slice.routing_regions.push_back(std::move(*routing_region));

    return true;
}




void PatchComputation::make_slices(
        LSInstructionStream&& instruction_stream,
        std::optional<std::chrono::seconds> timeout)
{
    slice_store_.accept_new_slice(first_slice_from_layout(*layout_, instruction_stream.core_qubits()));
    ls_instructions_count_++;
    compute_free_cells();

    auto start = std::chrono::steady_clock::now();
    size_t ls_op_counter = 0;

    while (instruction_stream.has_next_instruction())
    {
        LSInstruction instruction = instruction_stream.get_next_instruction();
        ls_instructions_count_++;

        if (const auto* s = std::get_if<SinglePatchMeasurement>(&instruction.operation))
        {
            Slice& slice = make_new_slice();
            slice.get_patch_by_id_mut(s->target).activity = PatchActivity::Measurement;
        }
        else if (const auto* p = std::get_if<SingleQubitOp>(&instruction.operation))
        {
            if(p->op == SingleQubitOp::Operator::S)
            {
                Slice& twist_merge_slice = make_new_slice();
                merge_patches(twist_merge_slice, *router_, p->target, PauliOperator::X, p->target, PauliOperator::Z);
            }
            Slice& slice = make_new_slice();
            slice.get_patch_by_id_mut(p->target).activity = PatchActivity::Unitary;
        }
        else if (const auto* m = std::get_if<MultiPatchMeasurement>(&instruction.operation))
        {

            if(m->observable.size()!=2)
                throw std::logic_error("Multi patch measurement only supports 2 patches currently");
            auto pairs = m->observable.begin();
            const auto& [source_id, source_op] = *pairs++;
            const auto& [target_id, target_op] = *pairs;

            if(!merge_patches(slice_store_.last_slice(), *router_, source_id, source_op, target_id, target_op))
            {
                Slice& slice = make_new_slice();
                if(!merge_patches(slice_store_.last_slice(), *router_, source_id, source_op, target_id, target_op))
                    throw std::runtime_error{ lstk::cat("Couldn't find room to route: ",
                            source_id , ":" , PauliOperator_to_string(source_op), ",",
                            target_id , ":" , PauliOperator_to_string(target_op))};
            }
        }
        else if (const auto* init = std::get_if<PatchInit>(&instruction.operation))
        {
            Slice& slice = make_new_slice();
            auto location = find_free_ancilla_location(*layout_, slice);
            if(!location) throw std::logic_error("Could not allocate ancilla");

            slice.qubit_patches.push_back(LayoutHelpers::basic_square_patch(*location));
            slice.qubit_patches.back().id = init->target;
            is_cell_free_[location->row][location->col] = false;
        }
        else if (const auto* rotation = std::get_if<RotateSingleCellPatch>(&instruction.operation))
        {
            const Patch target_patch{slice_store_.last_slice().get_patch_by_id(rotation->target)};
            if(const auto* target_occupied_cell = std::get_if<SingleCellOccupiedByPatch>(&target_patch.cells))
            {
                std::optional<Cell> free_neighbour;
                for(auto neighbour_cell :slice_store_.last_slice().get_neigbours_within_slice(target_occupied_cell->cell))
                    if(slice_store_.last_slice().is_cell_free(neighbour_cell))
                        free_neighbour = neighbour_cell;

                if(!free_neighbour)
                    throw std::runtime_error(lstk::cat(
                            "Cannot rotate patch ", rotation->target, ": has no free neighbour"));

                slice_store_.last_slice().delete_qubit_patch(rotation->target);

                auto stages{LayoutHelpers::single_patch_rotation_a_la_litinski(target_patch, *free_neighbour)};
                make_new_slice().routing_regions.push_back(std::move(stages.stage_1));
                make_new_slice().routing_regions.push_back(std::move(stages.stage_2));
                make_new_slice().qubit_patches.push_back(stages.final_state);
            }
            else
                throw std::runtime_error(lstk::cat(
                        "Cannot rotate patch ", rotation->target, ": is not single cell"));
        }
        else
        {
            if (!std::holds_alternative<MagicStateRequest>(instruction.operation))
                throw std::logic_error{"Unhandled LS instruction in PatchComputation"};
            const auto& mr = std::get<MagicStateRequest>(instruction.operation);

            const auto& d_times = layout_->distillation_times();
            if(!d_times.size()) throw std::logic_error("No distillation times");
            size_t max_wait_for_magic_state = *std::max_element(d_times.begin(), d_times.end());

            std::optional<Patch> newly_bound_magic_state;
            for (size_t i = 0; i<max_wait_for_magic_state; i++)
            {
                auto& slice_with_magic_state = make_new_slice();
                if(slice_with_magic_state.unbound_magic_states.size()>0)
                {
                    newly_bound_magic_state = slice_with_magic_state.unbound_magic_states.front();
                    slice_with_magic_state.unbound_magic_states.pop_front();
                    break;
                }
            }

            if(newly_bound_magic_state)
            {
                newly_bound_magic_state->id = mr.target;
                newly_bound_magic_state->type = PatchType::Qubit;
                slice_store_.last_slice().qubit_patches.push_back(*newly_bound_magic_state);
            }
            else
            {
                throw std::logic_error(
                        std::string{"Could not get magic state after waiting for steps: "}
                        +std::to_string(max_wait_for_magic_state));
            }

            ls_op_counter++;
            if(timeout && lstk::since(start) > *timeout)
            {
                auto timeout_str = std::string{"Out of time after "}+std::to_string(timeout->count())+std::string{"s. "}
                    + std::string{"Consumed "} + std::to_string(ls_op_counter) + std::string{" Instructions. "}
                    + std::string{"Generated "} + std::to_string(slice_store_.slice_count()) + std::string{"Slices."};

                throw std::runtime_error{timeout_str};
            }

        }

#if false
        for(const auto p: last_slice().patches)
        {
            auto cell = std::get<SingleCellOccupiedByPatch>(p.cells);
            std::cout << absl::StrFormat("{%d, %d, id=%d} ",cell.cell.row, cell.cell.col, p.id.value_or(99999));
        }
        std::cout<<std::endl;
#endif
    }

    slice_visitor_(slice_store_.last_slice());

}


void PatchComputation::compute_free_cells()
{
    layout_->for_each_cell([&](const Cell& cell){
        is_cell_free_[cell.row][cell.col] = !slice_store_.last_slice().get_any_patch_on_cell(cell);
    });
}


PatchComputation::PatchComputation(
        LSInstructionStream&& instruction_stream,
        std::unique_ptr<Layout>&& layout,
        std::unique_ptr<Router>&& router,
        std::optional<std::chrono::seconds> timeout,
        SliceVisitorFunction slice_visitor)
        :slice_store_(*layout), slice_visitor_(slice_visitor)
        {
    layout_ = std::move(layout);
    router_ = std::move(router);

    for(Cell::CoordinateType row = 0; row<=layout_->furthest_cell().row; row++ )
        is_cell_free_.push_back(std::vector<lstk::bool8>(static_cast<size_t>(layout_->furthest_cell().col+1), false));

    try
    {
        make_slices(std::move(instruction_stream), timeout);
    }
    catch (const std::exception& e)
    {
        std::cout << "Encountered exception: " << e.what() << std::endl;
        std::cout << "Halting slicing" << std::endl;
    }

}


void SliceStore::accept_new_slice(Slice&& slice)
{
    second_last_slice_ = std::move(last_slice_);
    last_slice_ = std::move(slice);
    slice_count_++;
}


SliceStore::SliceStore(const Layout& layout)
    :last_slice_(Slice::make_blank_slice(layout)), second_last_slice_(Slice::make_blank_slice(layout))
{}

}
