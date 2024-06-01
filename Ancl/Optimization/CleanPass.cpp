#include <Ancl/Optimization/CleanPass.hpp>

#include <unordered_set>
#include <queue>

#include <Ancl/AnclIR/IRProgram.hpp>


namespace ir {

CleanPass::CleanPass(Function* function)
    : m_Function(function) {}

void CleanPass::Run() {
    bool isChanged = true;
    while (isChanged) {
        std::unordered_map<ir::BasicBlock*, bool> visited;
        std::vector<ir::BasicBlock*> postorder;
        traversePostorder(m_Function->GetEntryBlock(), postorder, visited);

        isChanged = cleanControlFlowGraph(postorder);
    }

    eliminateUnreachableBlocks();
}

bool CleanPass::cleanControlFlowGraph(const std::vector<ir::BasicBlock*>& postorder) {
    bool isChanged = false;

    for (BasicBlock* basicBlock : postorder) {
        TerminatorInstruction* terminator = basicBlock->GetTerminator();
        assert(terminator);

        auto* branch = dynamic_cast<BranchInstruction*>(terminator);
        if (!branch) {
            continue;
        }

        if (branch->IsConditional()) {
            isChanged |= foldRedundantBranch(branch);
        } else {  // Jump
            BasicBlock* successor = basicBlock->GetSuccessors().at(0);

            if (basicBlock->IsEmpty()) {
                isChanged |= removeEmptyBlock(basicBlock, successor);
            } else if (successor->GetPredecessorsNumber() == 1 &&
                            successor != m_Function->GetEntryBlock()) {
                isChanged |= combineBlocks(basicBlock, successor);
            } else if (successor->IsEmpty()) {
                TerminatorInstruction* successorTerminator = successor->GetTerminator();
                auto* successorBranch = dynamic_cast<BranchInstruction*>(successorTerminator);
                if (successorBranch && successorBranch->IsConditional()) {
                    isChanged |= hoistBranch(basicBlock, successor, successorBranch);
                }
            }
        }
    }

    return isChanged;
}

bool CleanPass::foldRedundantBranch(BranchInstruction* branch) {
    if (branch->GetTrueBasicBlock() == branch->GetFalseBasicBlock()) {
        branch->ToUnconditionalTrue();
        return true;
    }

    return false;
}

bool CleanPass::removeEmptyBlock(BasicBlock* basicBlock, BasicBlock* successor) {
    if (successor->HasPhiFunctions()) {
        return false;
    }

    if (m_Function->GetEntryBlock() == basicBlock) {
        successor->RemovePredecessor(basicBlock);
        m_Function->SetEntryBlock(successor);
        return true;
    }

    // size_t blockPredIndex = 0;
    // for (BasicBlock* predecessor : successor->GetPredecessors()) {
    //     if (predecessor == basicBlock) {
    //         break;
    //     }
    //     ++blockPredIndex;
    // }

    // std::vector<Value*> blockPhiArguments;
    // std::vector<PhiInstruction*> successorPhis = successor->GetPhiFunctions();
    // for (PhiInstruction* phi : successorPhis) {
    //     blockPhiArguments.push_back(phi->GetIncomingValue(blockPredIndex));
    // }

    for (BasicBlock* predecessor : basicBlock->GetPredecessors()) {
        TerminatorInstruction* terminator = predecessor->GetTerminator();
        if (auto* branch = dynamic_cast<BranchInstruction*>(terminator)) {
            if (branch->IsUnconditional()) {
                branch->ToUnconditional(successor);
            } else {
                if (branch->GetTrueBasicBlock() == basicBlock) {
                    branch->SetTrueBasicBlock(successor);
                } else {
                    branch->SetFalseBasicBlock(successor);
                }
                successor->AddPredecessor(predecessor);
            }
        } else {
            // TODO: Handle switch
        }
    }

    successor->RemovePredecessor(basicBlock);

    return true;
}

bool CleanPass::combineBlocks(BasicBlock* basicBlock, BasicBlock* successor) {
    if (successor->HasPhiFunctions()) {
        return false;
    }

    TerminatorInstruction* terminator = successor->GetTerminator();
    for (Instruction* instruction : successor->GetInstructions()) {
        if (instruction == terminator) {
            break;
        }
        basicBlock->InsertInstructionBeforeTerminator(instruction);
    }

    for (BasicBlock* newSuccessor : successor->GetSuccessors()) {
        newSuccessor->ReplacePredecessor(successor, basicBlock);
    }

    basicBlock->ReplaceTerminator(terminator);

    return true;
}

bool CleanPass::hoistBranch(BasicBlock* basicBlock, BasicBlock* successor,
                            BranchInstruction* successorBranch) {
    IRProgram& program = m_Function->GetProgram();
    auto* newTerminator = program.CreateValue<BranchInstruction>(
                            successorBranch->GetCondition(), successorBranch->GetTrueBasicBlock(),
                            successorBranch->GetFalseBasicBlock(), basicBlock);
    basicBlock->ReplaceTerminator(newTerminator);

    successor->RemovePredecessor(basicBlock);

    for (BasicBlock* newSuccessor : successor->GetSuccessors()) {
        newSuccessor->AddPredecessor(basicBlock);
    }

    return true;
}

void CleanPass::eliminateUnreachableBlocks() {
    std::queue<BasicBlock*> blocksQueue;
    blocksQueue.push(m_Function->GetEntryBlock());

    std::vector<BasicBlock*> newBlockList;
    std::unordered_set<BasicBlock*> visited;

    size_t lastBlockIdx = 0;
    size_t blockIdx = 0;

    visited.insert(m_Function->GetEntryBlock());
    while (!blocksQueue.empty()) {
        BasicBlock* block = blocksQueue.front();
        blocksQueue.pop();

        newBlockList.push_back(block);

        if (dynamic_cast<ReturnInstruction*>(block->GetTerminator())) {
            lastBlockIdx = blockIdx;
        }
        ++blockIdx;

        for (BasicBlock* successor : block->GetSuccessors()) {
            if (!visited.contains(successor)) {
                visited.insert(successor);
                blocksQueue.push(successor);
            }
        }
    }

    for (BasicBlock* block : newBlockList) {
        for (BasicBlock* predecessor : block->GetPredecessors()) {
            if (!visited.contains(predecessor)) {
                block->RemovePredecessor(predecessor);
            }
        }
    }

    m_Function->SetBasicBlocks(newBlockList);
    m_Function->SetLastBlock(lastBlockIdx);
}

void CleanPass::traversePostorder(BasicBlock* block, std::vector<BasicBlock*>& postorder,
                                  std::unordered_map<BasicBlock*, bool>& visited) {
    visited[block] = true;
    for (BasicBlock* next : block->GetSuccessors()) {
        if (!visited[next]) {
            traversePostorder(next, postorder, visited);
        }
    }
    postorder.push_back(block);
}

}  // namespace
