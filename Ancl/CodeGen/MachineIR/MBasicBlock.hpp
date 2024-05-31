#pragma once

#include <list>
#include <string>

#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>


namespace gen {

class MFunction;

class MBasicBlock {
public:
    MBasicBlock(const std::string& name, MFunction* function);

    std::string GetName() const;

    void SetFunction(MFunction* function);

    MFunction* GetFunction() const;

    void AddSuccessor(MBasicBlock* block);
    std::vector<MBasicBlock*> GetSuccessors() const;

    void AddPredecessor(MBasicBlock* block);
    std::vector<MBasicBlock*> GetPredecessors() const;

    size_t GetPredecessorsNumber() const;
    MBasicBlock* GetPredecessor(size_t index) const;


    using TInstructionIt = std::list<MInstruction>::iterator;

    TInstructionIt GetInstrBegin();
    TInstructionIt GetInstrEnd();

    TInstructionIt GetInstrRBegin();
    TInstructionIt GetInstrREnd();

    TInstructionIt GetLastInstruction();

    std::list<MInstruction>& GetInstructions();

    void ClearInstructions();

    TInstructionIt AddInstruction(MInstruction instruction);
    TInstructionIt AddInstructionToBegin(MInstruction instruction);
    TInstructionIt InsertInstr(MInstruction instruction, size_t index);
    TInstructionIt InsertBeforeLastInstruction(MInstruction instruction);
    TInstructionIt InsertBefore(MInstruction instruction, TInstructionIt beforeIt);

    TInstructionIt InsertBefore(std::list<MInstruction>& instructions,
                                TInstructionIt beforeIt);

    TInstructionIt InsertAfter(MInstruction instruction, TInstructionIt afterIt);

    TInstructionIt InsertAfter(std::list<MInstruction>& instructions,
                               TInstructionIt afterIt);

    TInstructionIt GetPrevInstruction(TInstructionIt instructionIt);
    TInstructionIt GetNextInstruction(TInstructionIt instructionIt);

private:
    void handleNewInstruction(MInstruction& instruction);

private:
    std::string m_Name;

    MFunction* m_Function = nullptr;

    std::vector<MBasicBlock*> m_Successors;
    std::vector<MBasicBlock*> m_Predecessors;

    std::list<MInstruction> m_Instructions;
};

}  // namespace gen
