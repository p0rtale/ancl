#include <Ancl/Graph/DominatorTree.hpp>

#include <functional>


DominatorTree::DominatorTree(ir::Function* function, bool isReverse) {
    if (isReverse) {
        setEntryBlock(function->GetLastBlock());
    } else {
        setEntryBlock(function->GetEntryBlock());
    }

    setEdgeDirections(isReverse);

    computeReversePostorder();
    solveDominance();
    updateChildren();
    computeDominanceFrontiers();
}

ir::BasicBlock* DominatorTree::GetImmediateDominator(ir::BasicBlock* block) const {
    return m_Dominators.at(block);
}

std::vector<ir::BasicBlock*> DominatorTree::GetChildren(ir::BasicBlock* block) const {
    if (m_Children.contains(block)) {
        return m_Children.at(block);
    }
    return {};
}

std::unordered_set<ir::BasicBlock*> DominatorTree::GetDominanceFrontier(ir::BasicBlock* block) const {
    if (m_DominanceFrontierSet.contains(block)) {
        return m_DominanceFrontierSet.at(block);
    }
    return {};
}

void DominatorTree::setEntryBlock(ir::BasicBlock* basicBlock) {
    m_EntryBlock = basicBlock;
}

void DominatorTree::setEdgeDirections(bool isReverse) {
    if (isReverse) {
        p_GetPredecessors = &ir::BasicBlock::GetSuccessors;
        p_GetSuccessors = &ir::BasicBlock::GetPredecessors;
    } else {
        p_GetPredecessors = &ir::BasicBlock::GetPredecessors;
        p_GetSuccessors = &ir::BasicBlock::GetSuccessors; 
    }
}

void DominatorTree::computeDominanceFrontiers() {
    for (ir::BasicBlock* block : m_RPOrder) {
        std::vector<ir::BasicBlock*> preds = std::invoke(p_GetPredecessors, block);

        // Entry block or obvious immediate dominator
        if (preds.size() <= 1) {
            continue;
        }

        for (ir::BasicBlock* pred : preds) {
            auto predDominator = pred;
            while (predDominator != GetImmediateDominator(block)) {
                m_DominanceFrontierSet[predDominator].insert(block);
                predDominator = GetImmediateDominator(predDominator);
            }
        }
    }
}

void DominatorTree::updateChildren() {
    for (ir::BasicBlock* block : m_RPOrder) {
        auto dominator = GetImmediateDominator(block);
        if (dominator) {
            m_Children[dominator].push_back(block);
        }
    }
}

ir::BasicBlock* DominatorTree::intersectTwoDoms(ir::BasicBlock* firstBlock, ir::BasicBlock* secondBlock) {
    auto firstDom = firstBlock;
    auto secondDom = secondBlock;
    uint64_t firstNumber = m_RPONumbering[firstDom];
    uint64_t secondNumber = m_RPONumbering[secondDom];
    while (firstNumber != secondNumber) {
        while (firstNumber > secondNumber) {
            firstDom = m_Dominators[firstDom];
            firstNumber = m_RPONumbering[firstDom];
        }
        while (secondNumber > firstNumber) {
            secondDom = m_Dominators[secondDom];
            secondNumber = m_RPONumbering[secondDom];
        }
    }
    return firstDom;
}

ir::BasicBlock* DominatorTree::intersectPredsDoms(const std::vector<ir::BasicBlock*>& preds) {
    if (preds.empty()) {
        return nullptr;
    }

    ir::BasicBlock* newIDom = preds[0];

    if (preds.size() == 1) {
        return newIDom;
    }

    for (size_t i = 1; i < preds.size(); ++i) {
        auto block = preds[i];
        if (m_Dominators[block]) {
            newIDom = intersectTwoDoms(block, newIDom);
        }
    }

    return newIDom;
}

void DominatorTree::solveDominance() {
    for (size_t i = 0; i < m_RPOrder.size(); ++i) {
        m_RPONumbering[m_RPOrder[i]] = i;
    }

    ir::BasicBlock* startBlock = m_RPOrder[0];
    m_Dominators[startBlock] = startBlock;

    bool changed = true;
    while (changed) {
        changed = false;

        // Skip start block
        for (size_t i = 1; i < m_RPOrder.size(); ++i) {
            ir::BasicBlock* block = m_RPOrder[i];
            ir::BasicBlock* newIDom = intersectPredsDoms(std::invoke(p_GetPredecessors, block));
            if (m_Dominators[block] != newIDom) {
                m_Dominators[block] = newIDom;
                changed = true;
            }
        }
    }

    m_Dominators[startBlock] = nullptr;
}

void DominatorTree::computeReversePostorder() {
    std::unordered_map<ir::BasicBlock*, bool> visited;
    std::stack<ir::BasicBlock*> postorder;
    traversePostorder(m_EntryBlock, postorder, visited);

    m_RPOrder.clear();
    while (!postorder.empty()) {
        m_RPOrder.push_back(postorder.top());
        postorder.pop();
    }
}

void DominatorTree::traversePostorder(ir::BasicBlock* block, std::stack<ir::BasicBlock*>& postorder,
                        std::unordered_map<ir::BasicBlock*, bool>& visited) {
    visited[block] = true;
    for (ir::BasicBlock* next : std::invoke(p_GetSuccessors, block)) {
        if (!visited[next]) {
            traversePostorder(next, postorder, visited);
        }
    }
    postorder.push(block);
}
