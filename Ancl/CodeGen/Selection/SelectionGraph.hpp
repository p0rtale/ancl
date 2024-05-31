#pragma once

#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include <Ancl/CodeGen/MachineIR/MFunction.hpp>
#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>
#include <Ancl/CodeGen/Selection/SelectionTree.hpp>


namespace gen {

using TContext = std::vector<MInstruction>;

// Simple list of selection trees.
// Does not change the order.
class SelectionGraph {
public:
    SelectionGraph() = default;

    void Build(MFunction* function) {
        m_SelectionTrees.clear();
        TUseCount useCount = calcUseCount(function);

        std::vector<TContext> contexts = buildContexts(function);
        for (const TContext& context : contexts) {
            std::list<SelectionTree> contextTrees;
            for (const MInstruction& instruction : context) {
                contextTrees.emplace_back(instruction);
            }

            mergeTrees(contextTrees, useCount);

            for (SelectionTree& tree : contextTrees) {
                m_SelectionTrees.push_back(std::move(tree));
            }
        }
    }

    std::vector<SelectionTree>& GetTrees() {
        return m_SelectionTrees;
    }

private:
    using TUseCount = std::unordered_map<uint64_t, uint64_t>;

    TUseCount calcUseCount(MFunction* function) {
        TUseCount useCount;
        for (auto& basicBlock : function->GetBasicBlocks()) {
            for (MInstruction& instruction : basicBlock->GetInstructions()) {
                for (size_t i = 0; i < instruction.GetUsesNumber(); ++i) {
                    MInstruction::TOperandIt use = instruction.GetUse(i);
                    if (use->IsVRegister()) {
                        ++useCount[use->GetRegister()];
                    }
                }
            }
        }
        return useCount;
    }

    void mergeTrees(std::list<SelectionTree>& trees, const TUseCount& useCount) {
        std::unordered_map<uint64_t, std::list<SelectionTree>::iterator> definitionsMap;
        std::unordered_set<MInstruction*> validLoads;

        for (auto it = trees.begin(); it != trees.end(); ++it) {
            SelectionTree& tree = *it;

            auto& root = tree.GetRoot();
            MInstruction* rootInstruction = &root->GetInstructionRef();
            if (rootInstruction->IsDefinition()) {
                MInstruction::TOperandIt defOperand = rootInstruction->GetDefinition();
                if (defOperand->IsVRegister()) {
                    definitionsMap[defOperand->GetRegister()] = it;
                    if (rootInstruction->IsLoad()) {
                        validLoads.insert(rootInstruction);
                    }
                }
            }

            for (size_t i = 0; i < root->GetChildNumber(); ++i) {
                SelectionNode* child = root->GetChild(i);
                if (!child) {
                    MInstruction::TOperandIt useOperand = rootInstruction->GetUse(i);
                    if (useOperand->IsVRegister()) {
                        uint64_t vreg = useOperand->GetRegister();
                        if (definitionsMap.contains(vreg) && useCount.at(vreg) == 1) {
                            auto useTreeIt = definitionsMap[vreg];
                            auto& useRoot = useTreeIt->GetRoot();

                            MInstruction* useInstruction = &useRoot->GetInstructionRef();
                            if (useInstruction->IsLoad() && !validLoads.contains(useInstruction)) {
                                continue;
                            }

                            root->SetChild(std::move(useRoot), i);
                            trees.erase(useTreeIt);
                        }
                    }
                }
            }

            // Forget only after processing first Store
            // (store (add (load x) 10) x) --> add [x] 10
            if (rootInstruction->IsStore()) {
                validLoads.clear();
            }
        }
    }

    std::vector<TContext> buildContexts(MFunction* function) {
        std::vector<TContext> contexts;

        for (auto& basicBlock : function->GetBasicBlocks()) {
            TContext currentContext;
            for (MInstruction& instruction : basicBlock->GetInstructions()) {
                // Separate call and PR copies
                if (instruction.IsCall() || instruction.IsMov()) {
                    contexts.push_back(currentContext);
                    contexts.push_back({instruction});
                    currentContext.clear();
                    continue;
                }

                currentContext.push_back(instruction);

                if (instruction.IsTerminator()) {
                    contexts.push_back(currentContext);
                    currentContext.clear();
                }
            }

            if (!currentContext.empty()) {
                contexts.push_back(currentContext);
            }
        }

        return contexts;
    }

private:
    std::vector<SelectionTree> m_SelectionTrees;
};

}  // namespace gen
