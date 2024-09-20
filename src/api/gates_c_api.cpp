#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/ls_instructions/id_generator.hpp>


namespace lsqecc{

class LSInstructionStreamFromCAPI : public LSInstructionStream {
public:
    LSInstructionStreamFromCAPI(
            gates::Gate* gate_arr,
            size_t n_gates
            );

    LSInstruction get_next_instruction() override;

    bool has_next_instruction() const override {
        return next_instructions_.size();
    };

    const tsl::ordered_set<PatchId>& core_qubits() const override;


    void queue_instruction_from_gate(
        gates::Gate next_gate,
        std::queue<LSInstruction> next_instructions_,
        LSIinstructionFromGatesGenerator instruction_generator_
    ){};


private:
    
    // Directly wrap the gate array 
    size_t curr_gate;   
    size_t n_gates; 
    gates::Gate* gate_arr;
    std::queue<LSInstruction> next_instructions_;
    IdGenerator id_generator;

};


LSInstruction LSInstructionStreamFromCAPI::get_next_instruction()
{
    if(next_instructions_.empty())
    {
        if (curr_gate >= n_gates)      
        {
            throw std::logic_error{"LSInstructionStreamFromCAPI: No more gates but requesting instructions"};
        }
          gates::Gate next_gate = gate_arr[curr_gate];

    }
}

// Prevent mangling of api function names
extern "C" {

/*
 * api_create_router
 * Returns a router pointer
 * Default uses A*
 * TODO: Make the router type an argument  
 */
void* api_create_gate_stream(size_t n_gates, void* gates) 
{

    LSInstructionStream instruction_stream = LSInstructionStreamFromCAPI(
        (gates::Gate*)gates,
        n_gates);

    return (void*)instruction_stream;
}

} // extern "C"
} // namespace lsqecc
