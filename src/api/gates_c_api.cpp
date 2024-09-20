#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>

namespace lsqecc{

class LSInstructionStreamFromCAPI : public LSInstructionStream {
public:
    LSInstructionStreamFromCAPI(
            gates::Gate* gate_arr, // TODO why is this not rvalue ref?
            size_t n_gates,
            CNOTCorrectionMode cnot_correction_mode,
            IdGenerator& id_generator,
            bool local_instructions
            );

    LSInstruction get_next_instruction() override;

//    bool has_next_instruction() const override {
//        return next_instructions_.size() || gate_stream_.has_next_gate();
//    };
//
    const tsl::ordered_set<PatchId>& core_qubits() const override;

private:
    
    // Directly wrap the gate array 
    size_t curr_gate;   
    size_t n_gates; 
    gates::Gate* gate_arr;

    

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
void* api_create_gate_stream() 
{

    GateStream gate_stream;

    LSInstructionStream instruction_stream = LSInstructionStreamFromGateStream(*gate_stream, cnot_correction_mode, id_generator, compile_mode == CompilationMode::Local);    

    return (void*)instruction_stream;
}

} // extern "C"
} // namespace lsqecc
