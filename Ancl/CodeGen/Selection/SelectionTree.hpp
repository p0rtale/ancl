#pragma once

#include <vector>

#include <Ancl/Base.hpp>
#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>
#include <Ancl/CodeGen/Selection/SelectionNode.hpp>


namespace gen {

class SelectionTree {
public:
    SelectionTree(const MInstruction& instruction)
        : m_Root(CreateScope<SelectionNode>(instruction)) {}

    TScopePtr<SelectionNode>& GetRoot() {
        return m_Root;
    }

    std::vector<MInstruction> GenerateTargetInstructions() {
        std::vector<MInstruction> instructions;
        generatePostorder(m_Root.get(), instructions);
        return instructions;
    }

private:
    std::vector<MInstruction> generatePostorder(SelectionNode* node, std::vector<MInstruction>& instructions) {
        // TODO: Support DAG postorder

        for (SelectionNode* child : node->GetChildren()) {
            if (child) {
                generatePostorder(child, instructions);
            }
        }

        std::vector<MInstruction> prologueInstructions = node->GetPrologueInstructions();
        instructions.insert(instructions.end(), prologueInstructions.begin(), prologueInstructions.end());

        std::vector<MInstruction> targetInstructions = node->GetTargetInstructions();
        instructions.insert(instructions.end(), targetInstructions.begin(), targetInstructions.end());

        std::vector<MInstruction> epilogueInstructions = node->GetEpilogueInstructions();
        instructions.insert(instructions.end(), epilogueInstructions.begin(), epilogueInstructions.end());

        return instructions;
    }

private:
    TScopePtr<SelectionNode> m_Root;
};

}  // namespace gen
