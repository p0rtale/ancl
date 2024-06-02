#include <Ancl/CodeGen/MachineIR/MBasicBlock.hpp>

#include <cassert>


namespace gen {

MBasicBlock::MBasicBlock(const std::string& name, MFunction* function)
    : m_Name(name), m_Function(function) {}

void MBasicBlock::SetName(const std::string& name) {
    m_Name = name;
}

std::string MBasicBlock::GetName() const {
    return m_Name;
}

void MBasicBlock::SetFunction(MFunction* function) {
    m_Function = function;
}

MFunction* MBasicBlock::GetFunction() const {
    return m_Function;
}  

void MBasicBlock::AddSuccessor(MBasicBlock* block) {
    m_Successors.push_back(block);
}

std::vector<MBasicBlock*> MBasicBlock::GetSuccessors() const {
    return m_Successors;
}

void MBasicBlock::AddPredecessor(MBasicBlock* block) {
    m_Predecessors.push_back(block);
}

std::vector<MBasicBlock*> MBasicBlock::GetPredecessors() const {
    return m_Predecessors;
}

size_t MBasicBlock::GetPredecessorsNumber() const {
    return m_Predecessors.size();
}

MBasicBlock* MBasicBlock::GetPredecessor(size_t index) const {
    return m_Predecessors.at(index);
}

MBasicBlock::TInstructionIt MBasicBlock::GetInstrBegin() {
    return m_Instructions.begin();
}

MBasicBlock::TInstructionIt MBasicBlock::GetInstrEnd() {
    return m_Instructions.end();
}

MBasicBlock::TInstructionIt MBasicBlock::GetLastInstruction() {
    return --m_Instructions.end();
}

std::list<MInstruction>& MBasicBlock::GetInstructions() {
    return m_Instructions;
}

void MBasicBlock::ClearInstructions() {
    m_Instructions.clear();
}

MBasicBlock::TInstructionIt MBasicBlock::AddInstruction(MInstruction instruction) {
    handleNewInstruction(instruction);
    m_Instructions.push_back(instruction);
    return --m_Instructions.end();
}

MBasicBlock::TInstructionIt MBasicBlock::AddInstructionToBegin(MInstruction instruction) {
    handleNewInstruction(instruction);
    m_Instructions.push_front(instruction);
    return m_Instructions.begin();
}

MBasicBlock::TInstructionIt MBasicBlock::InsertInstr(MInstruction instruction, size_t index) {
    handleNewInstruction(instruction);
    auto iterator = m_Instructions.begin();
    std::advance(iterator, index);
    return m_Instructions.insert(iterator, instruction);
}

MBasicBlock::TInstructionIt MBasicBlock::InsertBeforeLastInstruction(MInstruction instruction) {
    handleNewInstruction(instruction);
    MBasicBlock::TInstructionIt lastInstrIr = --GetInstrEnd();
    return m_Instructions.insert(lastInstrIr, instruction);
}

MBasicBlock::TInstructionIt MBasicBlock::InsertBefore(MInstruction instruction, TInstructionIt beforeIt) {
    handleNewInstruction(instruction);
    return m_Instructions.insert(beforeIt, instruction);
}

MBasicBlock::TInstructionIt MBasicBlock::InsertBefore(std::list<MInstruction>& instructions,
                                                      TInstructionIt beforeIt) {
    for (auto& instruction : instructions) {
        handleNewInstruction(instruction);
    }
    return m_Instructions.insert(beforeIt, instructions.begin(), instructions.end());
}

MBasicBlock::TInstructionIt MBasicBlock::InsertAfter(MInstruction instruction, TInstructionIt afterIt) {
    return InsertBefore(instruction, ++afterIt);
}

MBasicBlock::TInstructionIt MBasicBlock::InsertAfter(std::list<MInstruction>& instructions,
                                                     TInstructionIt afterIt) {
    return InsertBefore(instructions, ++afterIt);
}

MBasicBlock::TInstructionIt MBasicBlock::GetPrevInstruction(TInstructionIt instructionIt) {
    assert(instructionIt != m_Instructions.begin());
    return --instructionIt;
}

MBasicBlock::TInstructionIt MBasicBlock::GetNextInstruction(TInstructionIt instructionIt) {
    assert(instructionIt != m_Instructions.end());
    return ++instructionIt;
}

void MBasicBlock::handleNewInstruction(MInstruction& instruction) {
    if (!instruction.HasBasicBlock()) {
        instruction.SetBasicBlock(this);
    }
}

}  // namespace gen
