#pragma once

#include <Ancl/AnclIR/IR.hpp>


namespace ir {

/*
    Useless Control Flow and Unreachable Code Elimination
*/
class CleanPass {
public:
    CleanPass(Function* function);

    void Run();

private:
    bool cleanControlFlowGraph(const std::vector<ir::BasicBlock*>& postorder);

    bool foldRedundantBranch(BranchInstruction* branch);

    bool removeEmptyBlock(BasicBlock* basicBlock, BasicBlock* successor);

    bool combineBlocks(BasicBlock* basicBlock, BasicBlock* successor);

    bool hoistBranch(BasicBlock* basicBlock, BasicBlock* successor,
                     BranchInstruction* successorBranch);

    void eliminateUnreachableBlocks();

    void traversePostorder(BasicBlock* block, std::vector<BasicBlock*>& postorder,
                           std::unordered_map<BasicBlock*, bool>& visited);

private:
    Function* m_Function = nullptr;
};

}  // namespace
