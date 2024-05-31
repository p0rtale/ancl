#pragma once

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

/*
    Instr def, use1, use2
    ---------------------
    Copy def, use1
    Instr def, def, use2
*/
class DestructiveInstructionPass {
public:
    DestructiveInstructionPass(MIRProgram& program,
                               target::TargetMachine* targetMachine);

    void Run();

private:
    MIRProgram& m_Program;
    target::TargetMachine* m_TargetMachine = nullptr;
};

}  // namespace gen
