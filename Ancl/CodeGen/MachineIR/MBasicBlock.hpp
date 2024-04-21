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

    using TInstructionIt = std::list<MInstruction>::iterator;

    TInstructionIt GetInstructionsBegin() {
        return m_Instructions.begin();
    }

    TInstructionIt GetInstructionsEnd() {
        return m_Instructions.end();
    }

    std::list<MInstruction>& GetInstructions() {
        return m_Instructions;
    }

    void AddInstruction(MInstruction instruction) {
        handleNewInstruction(instruction);
        m_Instructions.push_back(instruction);
    }

    void AddInstructionToBegin(MInstruction instruction) {
        handleNewInstruction(instruction);
        m_Instructions.push_front(instruction);
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

    std::list<MInstruction> m_Instructions;
};

}  // namespace gen
