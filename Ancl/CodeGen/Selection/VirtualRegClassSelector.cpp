#include <Ancl/CodeGen/Selection/VirtualRegClassSelector.hpp>


namespace gen {

VirtualRegClassSelector::VirtualRegClassSelector(MIRProgram& mirProgram,
                                                 target::TargetMachine* targetMachine)
    : m_MIRProgram(mirProgram), m_TargetMachine(targetMachine) {}

void VirtualRegClassSelector::Select() {
    for (auto& function : m_MIRProgram.GetFunctions()) {
        for (auto& basicBlock : function->GetBasicBlocks()) {
            processBasicBlock(*basicBlock);
        }
    }
}

void VirtualRegClassSelector::SelectOperandClass(MOperand& operand, target::TargetMachine* targetMachine) {
    if (operand.IsVRegister() || operand.IsImmediate()) {
        target::RegisterSet* regSet = targetMachine->GetRegisterSet();
        MType opType = operand.GetType();
        unsigned int regClass = regSet->GetRegisterClass(opType.GetBytes(), opType.IsFloat());
        operand.SetRegisterClass(regClass);
    } else if (operand.IsPRegister()) {
        target::RegisterSet* regSet = targetMachine->GetRegisterSet();
        target::Register preg = regSet->GetRegister(operand.GetRegister());
        operand.SetRegisterClass(regSet->GetRegisterClass(preg));
    }
}

void VirtualRegClassSelector::processOperand(MOperand& operand) {
    SelectOperandClass(operand, m_TargetMachine);
}

void VirtualRegClassSelector::processBasicBlock(MBasicBlock& basicBlock) {
    for (MInstruction& instr : basicBlock.GetInstructions()) {
        for (MOperand& operand : instr.GetOperands()) {
            processOperand(operand);
        }
    }
}

}  // namespace gen
