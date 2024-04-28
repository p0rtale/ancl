#pragma once

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

class VRegClassSelector {
public:
    VRegClassSelector(MIRProgram* mirProgram,
                      target::TargetMachine* targetMachine)
        : m_MIRProgram(mirProgram), m_TargetMachine(targetMachine) {}

    void Select() {
        for (auto& function : m_MIRProgram->GetFunctions()) {
            for (auto& basicBlock : function->GetBasicBlocks()) {
                processBasicBlock(*basicBlock);
            }
        }
    }

private:
    void processOperand(MOperand& operand) {
        if (operand.IsVRegister()) {
            uint opRegister = operand.GetRegister();
            auto* regSet = m_TargetMachine->GetRegisterSet();
            auto opType = operand.GetType();
            uint regClass = regSet->GetRegisterClass(opType.GetBytes(), opType.IsFloat());
            operand.SetRegisterClass(regClass);
        }
    }

    void processBasicBlock(MBasicBlock& basicBlock) {
        for (auto& instr : basicBlock.GetInstructions()) {
            for (auto& operand : instr.GetOperands()) {
                processOperand(operand);
            }
        }
    }

public:
    MIRProgram* m_MIRProgram;
    target::TargetMachine* m_TargetMachine;
};

}  // namespace gen
