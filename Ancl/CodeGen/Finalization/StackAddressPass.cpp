#include <Ancl/CodeGen/Finalization/StackAddressPass.hpp>


namespace gen {

StackAddressPass::StackAddressPass(MIRProgram& program, target::TargetMachine* targetMachine)
    : m_Program(program), m_TargetMachine(targetMachine) {}

void StackAddressPass::Run() {
    for (auto& function : m_Program.GetFunctions()) {
        for (auto& basicBlock : function->GetBasicBlocks()) {
            for (MInstruction& instr : basicBlock->GetInstructions()) {
                replaceStackOperand(function.get(), instr);
            }
        }
    }
}

void StackAddressPass::replaceStackOperand(MFunction* function, MInstruction& instr) {
    MInstruction::TOperandIt operandIt = instr.GetOpBegin();
    while (!operandIt->IsStackIndex() && operandIt != instr.GetOpEnd()) {
        ++operandIt;
    }

    if (operandIt == instr.GetOpEnd()) {
        return;
    }

    uint64_t index = operandIt->GetIndex();
    LocalDataArea& localData = function->GetLocalDataArea();
    if (!localData.HasSlot(index)) {
        ANCL_CRITICAL("StackAddressPass: Unknown stack index");
    }

    target::RegisterSet* registers = m_TargetMachine->GetRegisterSet();

    target::Register arpReg = registers->GetARP();
    MType type(MType::Kind::kInteger, arpReg.GetBytes());
    *operandIt = MOperand::CreateRegister(arpReg.GetNumber(), type, /*isVirtual=*/false);

    uint64_t offset = localData.GetSlotOffset(index);
    ++operandIt;  // ScaleImm
    ++operandIt;  // IndexReg
    ++operandIt;  // DispImm

    operandIt->SetImmInteger(operandIt->GetImmInteger() - offset);
}

}  // namespace gen
