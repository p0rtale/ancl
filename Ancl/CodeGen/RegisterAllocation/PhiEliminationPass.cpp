#include <Ancl/CodeGen/RegisterAllocation/PhiEliminationPass.hpp>

#include <Ancl/CodeGen/Selection/InstructionSelector.hpp>


namespace gen {

PhiEliminationPass::PhiEliminationPass(MIRProgram& program, target::TargetMachine* targetMachine)
    : m_Program(program), m_TargetMachine(targetMachine) {}

void PhiEliminationPass::Run() {
    for (auto& function : m_Program.GetFunctions()) {
        for (auto& basicBlock : function->GetBasicBlocks()) {
            std::list<MInstruction>& instructions = basicBlock->GetInstructions();
            for (auto it = instructions.begin(); it != instructions.end(); ++it) {
                MInstruction& instruction = *it;
                if (instruction.IsPhi()) {
                    MInstruction::TOperandIt phiVReg = instruction.GetDefinition();

                    MType phiType = phiVReg->GetType();
                    auto copyType = MInstruction::OpType::kMov;
                    if (phiType.IsFloat()) {
                        copyType = MInstruction::OpType::kFMov;
                    }

                    auto newPhiVReg = MOperand::CreateRegister(function->NextVReg(), phiType);
                    newPhiVReg.SetRegisterClass(phiVReg->GetRegisterClass());

                    for (size_t i = 0; i < basicBlock->GetPredecessorsNumber(); ++i) {
                        MInstruction::TOperandIt use = instruction.GetUse(i);

                        // TODO: Replace with INVALID_REGNUM
                        if (use->IsRegister() && use->IsInvalidRegister()) {
                            continue;
                        }

                        // TODO: Remove the crutch
                        if (use->IsImmFloat()) {
                            static uint64_t counter = 0;

                            std::string labelName = ".L.float.phi";
                            if (counter > 0) {
                                labelName += "." + std::to_string(counter);
                            }

                            GlobalDataArea globalDataArea{labelName};
                            globalDataArea.SetConst();
                            globalDataArea.SetLocal();

                            if (phiType.GetBytes() == 8) {
                                globalDataArea.AddDoubleSlot(use->GetImmFloat());
                            } else {
                                globalDataArea.AddFloatSlot(static_cast<float>(use->GetImmFloat()));
                            }

                            m_Program.AddGlobalDataArea(globalDataArea);

                            MBasicBlock* pred = basicBlock->GetPredecessor(i);
                            MInstruction copyInstr{MInstruction::OpType::kLoad};
                            copyInstr.AddOperand(newPhiVReg);
                            copyInstr.AddGlobalSymbol(labelName);

                            MInstruction targetCopy = InstructionSelector::SelectInstruction(
                                                                copyInstr, m_TargetMachine);
                            pred->InsertBeforeLastInstruction(targetCopy);

                            continue;
                        }

                        MBasicBlock* pred = basicBlock->GetPredecessor(i);
                        MInstruction copyInstr{copyType};
                        copyInstr.AddOperand(newPhiVReg);
                        copyInstr.AddOperand(*use);

                        MInstruction targetCopy = InstructionSelector::SelectInstruction(
                                                            copyInstr, m_TargetMachine);
                        pred->InsertBeforeLastInstruction(targetCopy);
                    }

                    MInstruction phiCopy{copyType};
                    phiCopy.AddOperand(*phiVReg);
                    phiCopy.AddOperand(newPhiVReg);
                    MInstruction targetCopy = InstructionSelector::SelectInstruction(
                                                        phiCopy, m_TargetMachine);

                    targetCopy.SetBasicBlock(basicBlock.get());
                    *it = targetCopy;
                }
            }
        }
    }
}

}  // namespace gen
