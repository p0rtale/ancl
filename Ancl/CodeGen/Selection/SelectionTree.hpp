#pragma once

#include <vector>

#include <Ancl/Base.hpp>
#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>


namespace gen {

class SelectionNode {
public:
    SelectionNode(MInstruction instruction)
        : m_Instruction(instruction), m_Children(instruction.GetUsesNumber()) {}

    void SetChild(TScopePtr<SelectionNode> child, size_t index) {
        m_Children[index] = std::move(child);
    }

    MInstruction GetInstruction() const {
        return m_Instruction;
    }

    MInstruction& GetInstructionRef() {
        return m_Instruction;
    }

    void AddTargetInstruction(MInstruction targetInstruction) {
        m_TargetInstructions.push_back(targetInstruction);
    }

    bool HasTargetInstructions() const {
        return !m_TargetInstructions.empty() || m_Instruction.HasTargetInstruction();
    }

    std::vector<MInstruction> GetTargetInstructions() const {
        if (m_TargetInstructions.empty()) {
            return {m_Instruction};
        }
        return m_TargetInstructions;
    }

    size_t GetChildNumber() const {
        return m_Children.size();
    }

    SelectionNode* GetChild(size_t index) const {
        return m_Children.at(index).get();
    }

    std::vector<SelectionNode*> GetChildren() const {
        std::vector<SelectionNode*> childPointers;
        childPointers.reserve(m_Children.size());
        for (auto& child : m_Children) {
            childPointers.push_back(child.get());
        }
        return childPointers;
    }

    void MarkAsSelected() {
        m_IsSelected = true;
    }

    bool IsSelected() const {
        return m_IsSelected;
    }

private:
    MInstruction m_Instruction;

    bool m_IsSelected = false;
    std::vector<MInstruction> m_TargetInstructions;

    std::vector<TScopePtr<SelectionNode>> m_Children;
};


class SelectionTree {
public:
    SelectionTree(MInstruction instruction)
        : m_Root(CreateScope<SelectionNode>(instruction)) {}

    TScopePtr<SelectionNode>& GetRoot() {
        return m_Root;
    }

private:
    TScopePtr<SelectionNode> m_Root;
};

}  // namespace gen
