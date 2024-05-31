#pragma once

#include <unordered_set>

#include <Ancl/AnclIR/IR.hpp>
#include <Ancl/Graph/DominatorTree.hpp>


namespace ir {

/*
    Dead Code Elimination
*/
class DCEPass {
public:
    DCEPass(Function* function);

    void Run();

private:
    void runMark();
    void runSweep();

    bool isCriticalInstruction(Instruction* instruction) const;

    bool isMarkedInstruction(Instruction* instruction) const;
    void markInstruction(Instruction* instruction);

    bool isMarkedBasicBlock(BasicBlock* basicBlock) const;
    void markBasicBlock(BasicBlock* basicBlock);

private:
    Function* m_Function = nullptr;
    DominatorTree m_ReverseDomTree;

    std::unordered_set<Instruction*> m_InstructionMarks;
    std::unordered_set<BasicBlock*> m_BlocksMarks;
};

}  // namespace ir
