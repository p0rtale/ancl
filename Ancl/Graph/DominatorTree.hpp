#pragma once

#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include <Ancl/AnclIR/BasicBlock.hpp>


// http://www.hipersoft.rice.edu/grads/publications/dom14.pdf
class DominatorTree {
public:
    DominatorTree(ir::BasicBlock* entryBlock): m_EntryBlock(entryBlock) {
        computeReversePostorder();
        solveDominance();
        updateChildren();
        computeDominanceFrontiers();
    }

    ir::BasicBlock* GetImmediateDominator(ir::BasicBlock* block) const {
        return m_Dominators.at(block);
    }

    std::vector<ir::BasicBlock*> GetChildren(ir::BasicBlock* block) const {
        return m_Children.at(block);
    }

    std::unordered_set<ir::BasicBlock*> GetDominanceFrontier(ir::BasicBlock* block) const {
        return m_DominanceFrontierSet.at(block);
    }

private:
    void computeDominanceFrontiers() {
        for (ir::BasicBlock* block : m_RPOrder) {
            auto preds = block->GetPredecessors();

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

    void updateChildren() {
        for (ir::BasicBlock* block : m_RPOrder) {
            auto dominator = GetImmediateDominator(block);
            if (dominator) {
                m_Children[dominator].push_back(block);
            }
        }
    }

    ir::BasicBlock* intersectTwoDoms(ir::BasicBlock* firstBlock, ir::BasicBlock* secondBlock) {
        auto firstDom = firstBlock;
        auto secondDom = secondBlock;
        uint firstNumber = m_RPONumbering[firstDom];
        uint secondNumber = m_RPONumbering[secondDom];
        while (firstNumber != secondNumber) {
            while (firstNumber < secondNumber) {
                firstDom = m_Dominators[firstDom];
                firstNumber = m_RPONumbering[firstDom];
            }
            while (secondNumber < firstNumber) {
                secondDom = m_Dominators[secondDom];
                secondNumber = m_RPONumbering[secondDom];
            }
        }
        return firstDom;
    }

    ir::BasicBlock* intersectPredsDoms(const std::vector<ir::BasicBlock*>& preds) {
        if (preds.empty()) {
            return nullptr;
        }

        auto newIDom = preds[0];

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

    void solveDominance() {
        for (size_t i = 0; i < m_RPOrder.size(); ++i) {
            m_RPONumbering[m_RPOrder[i]] = i;
        }

        if (m_RPOrder.size() <= 1) {
            return;
        }

        auto startBlock = m_RPOrder[0];
        m_Dominators[startBlock] = startBlock;

        bool changed = true;
        while (changed) {
            changed = false;

            // Skip start block
            for (size_t i = 1; i < m_RPOrder.size(); ++i) {
                auto block = m_RPOrder[i];
                auto newIDom = intersectPredsDoms(block->GetPredecessors());
                if (m_Dominators[block] != newIDom) {
                    m_Dominators[block] = newIDom;
                    changed = true;
                }
            }
        }

        m_Dominators[startBlock] = nullptr;
    }

    void computeReversePostorder() {
        std::unordered_map<ir::BasicBlock*, bool> visited;
        std::stack<ir::BasicBlock*> postorder;
        traversePostorder(m_EntryBlock, postorder, visited);

        m_RPOrder.clear();
        while (!postorder.empty()) {
            m_RPOrder.push_back(postorder.top());
            postorder.pop();
        }
    }

    void traversePostorder(ir::BasicBlock* block, std::stack<ir::BasicBlock*>& postorder,
                           std::unordered_map<ir::BasicBlock*, bool>& visited) {
        visited[block] = true;
        for (ir::BasicBlock* next : block->GetSuccessors()) {
            if (!visited[next]) {
                traversePostorder(next, postorder, visited);
            }
        }
        postorder.push(block);
    }

private:
    ir::BasicBlock* m_EntryBlock = nullptr;
    std::unordered_map<ir::BasicBlock*, ir::BasicBlock*> m_Dominators;
    std::unordered_map<ir::BasicBlock*, std::vector<ir::BasicBlock*>> m_Children;

    std::unordered_map<ir::BasicBlock*, std::unordered_set<ir::BasicBlock*>> m_DominanceFrontierSet;

    // Reverse postorder info
    std::vector<ir::BasicBlock*> m_RPOrder;
    std::unordered_map<ir::BasicBlock*, uint> m_RPONumbering;
};
