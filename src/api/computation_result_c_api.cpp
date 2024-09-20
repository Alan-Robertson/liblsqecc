#include <lsqecc/dag/domain_dags.hpp>
#include <lsqecc/gates/parse_gates.hpp>
#include <lsqecc/gates/decompose_rotation_stream.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/ls_instructions/boundary_rotation_injection_stream.hpp>
#include <lsqecc/ls_instructions/teleported_s_gate_injection_stream.hpp>
#include <lsqecc/ls_instructions/catalytic_s_gate_injection_stream.hpp>
#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <lsqecc/layout/router.hpp>
#include <lsqecc/layout/dynamic_layouts/compact_layout.hpp>
#include <lsqecc/layout/dynamic_layouts/edpc_layout.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/slice_stats.hpp>
#include <lsqecc/patches/dense_patch_computation.hpp>
#include <lsqecc/patches/slice_variant.hpp>


namespace lsqecc{

// Prevent mangling of api function names
extern "C" {

/*
 * api_create_router
 * Returns a router pointer
 * Default uses A*
 * TODO: Make the router type an argument  
 */
void* api_computation_result(void* layout, void* router) 
{

    PipelineMode pipeline_mode = PipelineMode::Stream;

    auto timeout = std::nullopt;
    DenseSliceVisitor slice_visitor = [](const DenseSlice& s) -> void {LSTK_UNUSED(s);};
    LSInstructionVisitor instruction_visitor{[&](const LSInstruction& i){}};

    bool parser_exists_graceful = true; // parser.exists("graceful") 

    PatchComputationResult computation_result = 
        DensePatchComputationResult(run_through_dense_slices(
            std::move(*instruction_stream),
            pipeline_mode,
            compile_mode == CompilationMode::Local,
            *layout,
            *router,
            timeout, //timeout,
            slice_visitor,
            instruction_visitor,
            true 
    ));
    return (void*)computation_result;
}

} // extern "C"
} // namespace lsqecc
