#pragma once

#include <vector>

#include <Ancl/Base.hpp>
#include <Ancl/CodeGen/MachineIR/MBasicBlock.hpp>
#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>


namespace gen {

class SelectionNode {
public:
    SelectionNode(MInstruction instruction)
        : m_Instruction(instruction),
          m_BasicBlock(instruction.GetBasicBlock()),
          m_Children(instruction.GetUsesNumber()) {}

    void SetChild(TScopePtr<SelectionNode> child, size_t index) {
        m_Children[index] = std::move(child);
    }

    void SetInstruction(const MInstruction& instruction) {
        m_Instruction = instruction;
    }

    MInstruction GetInstruction() const {
        return m_Instruction;
    }

    MInstruction& GetInstructionRef() {
        return m_Instruction;
    }

    MBasicBlock* GetBasicBlock() {
        return m_BasicBlock;
    }

    void AddTargetInstruction(MInstruction targetInstruction) {
        m_TargetInstructions.push_back(targetInstruction);
    }

    bool HasTargetInstructions() const {
        return !m_TargetInstructions.empty() || m_Instruction.HasTargetInstruction();
    }

    void AddPrologueInstruction(const MInstruction& targetInstruction) {
        m_PrologueInstructions.push_back(targetInstruction);
    }

    std::vector<MInstruction> GetPrologueInstructions() const {
        return m_PrologueInstructions;
    }

    void AddEpilogueInstruction(const MInstruction& targetInstruction) {
        m_EpilogueInstructions.push_back(targetInstruction);
    }

    std::vector<MInstruction> GetEpilogueInstructions() const {
        return m_EpilogueInstructions;
    }

    std::vector<MInstruction> GetTargetInstructions() const {
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
    MBasicBlock* m_BasicBlock = nullptr;

    bool m_IsSelected = false;
    std::vector<MInstruction> m_TargetInstructions;

    std::vector<MInstruction> m_PrologueInstructions;
    std::vector<MInstruction> m_EpilogueInstructions;

    std::vector<TScopePtr<SelectionNode>> m_Children;
};

}  // namespace gen
