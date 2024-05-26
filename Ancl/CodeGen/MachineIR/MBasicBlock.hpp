#pragma once

#include <string>
#include <list>

#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>


namespace gen {

class MFunction;

class MBasicBlock {
public:
    MBasicBlock(const std::string& name, MFunction* function)
        : m_Name(name), m_Function(function) {}

    std::string GetName() const {
        return m_Name;
    }

    void SetFunction(MFunction* function) {
        m_Function = function;
    }

    MFunction* GetFunction() const {
        return m_Function;
    }  

    void AddSuccessor(MBasicBlock* block) {
        m_Successors.push_back(block);
    }

    std::vector<MBasicBlock*> GetSuccessors() const {
        return m_Successors;
    }

    void AddPredecessor(MBasicBlock* block) {
        m_Predecessors.push_back(block);
    }

    std::vector<MBasicBlock*> GetPredecessors() const {
        return m_Predecessors;
    }

    size_t GetPredecessorsNumber() const {
        return m_Predecessors.size();
    }

    MBasicBlock* GetPredecessor(size_t index) const {
        return m_Predecessors.at(index);
    }

    using TInstructionIt = std::list<MInstruction>::iterator;

    TInstructionIt GetInstrBegin() {
        return m_Instructions.begin();
    }

    TInstructionIt GetInstrEnd() {
        return m_Instructions.end();
    }

    std::list<MInstruction>& GetInstructions() {
        return m_Instructions;
    }

    void ClearInstructions() {
        m_Instructions.clear();
    }

    TInstructionIt AddInstruction(MInstruction instruction) {
        handleNewInstruction(instruction);
        m_Instructions.push_back(instruction);
        return --m_Instructions.end();
    }

    TInstructionIt AddInstructionToBegin(MInstruction instruction) {
        handleNewInstruction(instruction);
        m_Instructions.push_front(instruction);
        return m_Instructions.begin();
    }

    TInstructionIt InsertInstr(MInstruction instruction, size_t index) {
        handleNewInstruction(instruction);
        auto iterator = m_Instructions.begin();
        std::advance(iterator, index);
        return m_Instructions.insert(iterator, instruction);
    }

    TInstructionIt InsertBefore(MInstruction instruction, TInstructionIt beforeIt) {
        handleNewInstruction(instruction);
        return m_Instructions.insert(beforeIt, instruction);
    }

    TInstructionIt InsertBefore(std::list<MInstruction>& instructions,
                                TInstructionIt beforeIt) {
        for (auto& instruction : instructions) {
            handleNewInstruction(instruction);
        }
        return m_Instructions.insert(beforeIt, instructions.begin(), instructions.end());
    }

    TInstructionIt InsertAfter(MInstruction instruction, TInstructionIt afterIt) {
        return InsertBefore(instruction, ++afterIt);
    }

    TInstructionIt InsertAfter(std::list<MInstruction>& instructions,
                               TInstructionIt afterIt) {
        return InsertBefore(instructions, ++afterIt);
    }

    TInstructionIt GetPrevInstruction(TInstructionIt instructionIt) {
        assert(instructionIt != m_Instructions.begin());
        return --instructionIt;
    }

    TInstructionIt GetNextInstruction(TInstructionIt instructionIt) {
        assert(instructionIt != m_Instructions.end());
        return ++instructionIt;
    }

private:
    void handleNewInstruction(MInstruction& instruction) {
        if (!instruction.HasBasicBlock()) {
            instruction.SetBasicBlock(this);
        }
    }

private:
    std::string m_Name;

    MFunction* m_Function = nullptr;

    std::vector<MBasicBlock*> m_Successors;
    std::vector<MBasicBlock*> m_Predecessors;

    std::list<MInstruction> m_Instructions;
};

}  // namespace gen
