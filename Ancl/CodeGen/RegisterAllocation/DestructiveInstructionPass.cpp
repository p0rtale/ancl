#include <Ancl/CodeGen/RegisterAllocation/DestructiveInstructionPass.hpp>

#include <Ancl/CodeGen/Selection/InstructionSelector.hpp>


namespace gen {

DestructiveInstructionPass::DestructiveInstructionPass(MIRProgram& program,
                                                       target::TargetMachine* targetMachine)
    : m_Program(program),
      m_TargetMachine(targetMachine) {}

void DestructiveInstructionPass::Run() {
    for (auto& function : m_Program.GetFunctions()) {
        for (auto& basicBlock : function->GetBasicBlocks()) {
            auto& instructions = basicBlock->GetInstructions();
            for (auto it = instructions.begin(); it != instructions.end(); ++it) {
                auto& instruction = *it;

                uint64_t targetInstrCode = instruction.GetTargetInstructionCode();
                target::InstructionSet* targetInstrSet = m_TargetMachine->GetInstructionSet();
                target::TargetInstruction targetInstr = targetInstrSet->GetInstruction(targetInstrCode);
                if (targetInstr.IsDestructive()) {
                    MInstruction::TOperandIt defReg = instruction.GetDefinition();
                    MInstruction::TOperandIt firstUse = instruction.GetUse(0);

                    if (firstUse->IsRegister() && firstUse->GetRegister() == defReg->GetRegister()) {
                        continue;
                    }

                    MType regsType = defReg->GetType();
                    auto movType = MInstruction::OpType::kMov;
                    if (regsType.IsFloat()) {
                        movType =  MInstruction::OpType::kFMov;
                    }

                    MInstruction copyInstr{movType};
                    copyInstr.AddOperand(*defReg);
                    copyInstr.AddOperand(*firstUse);

                    MInstruction targetCopy = InstructionSelector::SelectInstruction(copyInstr, m_TargetMachine);
                    basicBlock->InsertBefore(targetCopy, it);

                    *firstUse = *defReg;
                }
            }
        }
    }
}

}  // namespace gen
