#include <Ancl/Optimization/DCEPass.hpp>


namespace ir {

DCEPass::DCEPass(Function* function)
    : m_Function(function),
      m_ReverseDomTree(function, /*isReverse=*/true) {}

void DCEPass::Run() {
    runMark();
    runSweep();
}

void DCEPass::runMark() {
    std::vector<Instruction*> workList;

    for (BasicBlock* basicBlock : m_Function->GetBasicBlocks()) {
        for (Instruction* instruction : basicBlock->GetInstructions()) {
            if (isCriticalInstruction(instruction)) {
                markInstruction(instruction);
                workList.push_back(instruction);
            }
        }
    }

    while (!workList.empty()) {
        Instruction* instruction = std::move(workList.back());
        workList.pop_back();

        for (Value* operand : instruction->GetOperands()) {
            auto* instruction = dynamic_cast<Instruction*>(operand);
            if (instruction && !isMarkedInstruction(instruction)) {
                markInstruction(instruction);
                workList.push_back(instruction);
            }
        }

        BasicBlock* basicBlock = instruction->GetBasicBlock();
        for (BasicBlock* reverseFrontier : m_ReverseDomTree.GetDominanceFrontier(basicBlock)) {
            TerminatorInstruction* terminator = reverseFrontier->GetTerminator();
            if (auto* branch = dynamic_cast<BranchInstruction*>(terminator)) {
                if (!isMarkedInstruction(branch)) {
                    markInstruction(branch);
                    workList.push_back(branch);
                }
            }
        }
    }
}

void DCEPass::runSweep() {
    for (BasicBlock* basicBlock : m_Function->GetBasicBlocks()) {
        std::list<Instruction*>& instructions = basicBlock->GetInstructionsRef();
        for (auto it = instructions.begin(); it != instructions.end();) {
            Instruction* instruction = *it;

            if (isMarkedInstruction(instruction)) {
                ++it;
                continue;
            }

            if (auto* terminator = dynamic_cast<TerminatorInstruction*>(instruction)) {
                auto* branch = dynamic_cast<BranchInstruction*>(terminator);
                if (branch && branch->IsConditional()) {
                    // NB: Iteration will end because the exit block is marked
                    BasicBlock* nearestMarkedDominator = m_ReverseDomTree.GetImmediateDominator(basicBlock);
                    while (!isMarkedBasicBlock(nearestMarkedDominator)) {
                        nearestMarkedDominator = m_ReverseDomTree.GetImmediateDominator(nearestMarkedDominator);
                    }

                    branch->ToUnconditional(nearestMarkedDominator);
                }
                ++it;
            } else {
                it = instructions.erase(it);
            }
        }
    }
}

bool DCEPass::isCriticalInstruction(Instruction* instruction) const {
    if (auto* storeInstr = dynamic_cast<StoreInstruction*>(instruction)) {
        return true;
    }
    if (auto* loadInstr = dynamic_cast<LoadInstruction*>(instruction)) {
        if (loadInstr->IsVolatile()) {
            return true;
        }
    }
    if (auto* retInstr = dynamic_cast<ReturnInstruction*>(instruction)) {
        return true;
    }
    if (auto* callInstr = dynamic_cast<CallInstruction*>(instruction)) {
        return true;
    }

    return false;
}

bool DCEPass::isMarkedInstruction(Instruction* instruction) const {
    return m_InstructionMarks.contains(instruction);
}

void DCEPass::markInstruction(Instruction* instruction) {
    m_InstructionMarks.insert(instruction);
    markBasicBlock(instruction->GetBasicBlock());
}

bool DCEPass::isMarkedBasicBlock(BasicBlock* basicBlock) const {
    return m_BlocksMarks.contains(basicBlock);
}

void DCEPass::markBasicBlock(BasicBlock* basicBlock) {
    m_BlocksMarks.insert(basicBlock);
}

}  // namespace ir
