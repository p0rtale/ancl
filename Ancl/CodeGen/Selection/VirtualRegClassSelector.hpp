#pragma once

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

class VirtualRegClassSelector {
public:
    VirtualRegClassSelector(MIRProgram& mirProgram,
                            target::TargetMachine* targetMachine);

    void Select();

    static void SelectOperandClass(MOperand& operand, target::TargetMachine* targetMachine);

private:
    void processOperand(MOperand& operand);
    void processBasicBlock(MBasicBlock& basicBlock);

private:
    MIRProgram& m_MIRProgram;
    target::TargetMachine* m_TargetMachine = nullptr;
};

}  // namespace gen
