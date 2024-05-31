#pragma once

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

class StackAddressPass {
public:
    StackAddressPass(MIRProgram& program, target::TargetMachine* targetMachine);

    void Run();

private:
    void replaceStackOperand(MFunction* function, MInstruction& instr);

private:
    MIRProgram& m_Program;
    target::TargetMachine* m_TargetMachine = nullptr;
};

}  // namespace gen
