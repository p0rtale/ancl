#pragma once

#include <cstdint>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Constant/Function.hpp>


// http://www.hipersoft.rice.edu/grads/publications/dom14.pdf
class DominatorTree {
public:
    DominatorTree(ir::Function* function, bool isReverse = false);

    ir::BasicBlock* GetImmediateDominator(ir::BasicBlock* block) const;

    std::vector<ir::BasicBlock*> GetChildren(ir::BasicBlock* block) const;

    std::unordered_set<ir::BasicBlock*> GetDominanceFrontier(ir::BasicBlock* block) const;

private:
    void setEntryBlock(ir::BasicBlock* basicBlock);
    void setEdgeDirections(bool isReverse);

    void computeDominanceFrontiers();

    void updateChildren();

    ir::BasicBlock* intersectTwoDoms(ir::BasicBlock* firstBlock, ir::BasicBlock* secondBlock);

    ir::BasicBlock* intersectPredsDoms(const std::vector<ir::BasicBlock*>& preds);

    void solveDominance();

    void computeReversePostorder();

    void traversePostorder(ir::BasicBlock* block, std::stack<ir::BasicBlock*>& postorder,
                           std::unordered_map<ir::BasicBlock*, bool>& visited);

private:
    ir::BasicBlock* m_EntryBlock = nullptr;
    std::vector<ir::BasicBlock*> (ir::BasicBlock::*p_GetPredecessors)() const = nullptr;
    std::vector<ir::BasicBlock*> (ir::BasicBlock::*p_GetSuccessors)() const = nullptr;

    std::unordered_map<ir::BasicBlock*, ir::BasicBlock*> m_Dominators;
    std::unordered_map<ir::BasicBlock*, std::vector<ir::BasicBlock*>> m_Children;

    std::unordered_map<ir::BasicBlock*, std::unordered_set<ir::BasicBlock*>> m_DominanceFrontierSet;

    // Reverse postorder info
    std::vector<ir::BasicBlock*> m_RPOrder;
    std::unordered_map<ir::BasicBlock*, uint64_t> m_RPONumbering;
};
