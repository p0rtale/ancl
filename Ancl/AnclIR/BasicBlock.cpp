#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

#include <Ancl/AnclIR/Instruction/BranchInstruction.hpp>
#include <Ancl/AnclIR/Instruction/SwitchInstruction.hpp>

using namespace ir;


BasicBlock::BasicBlock(const std::string& name, LabelType* type, Function* function)
        : Value(type), m_Function(function) {
    SetName(name);
}

IRProgram& BasicBlock::GetProgram() const {
    return m_Function->GetProgram();
}

Function* BasicBlock::GetFunction() const {
    return m_Function;
}

void BasicBlock::AddInstruction(Instruction* instruction) {
    m_Instructions.push_back(instruction);
}

TerminatorInstruction* BasicBlock::GetTerminator() const {
    if (m_Instructions.empty()) {
        return nullptr;
    }
    return dynamic_cast<TerminatorInstruction*>(m_Instructions.back());
}

std::vector<BasicBlock*> BasicBlock::GetSuccessors() const {
    auto terminator = GetTerminator();
    std::vector<BasicBlock*> nextBlocks;
    if (auto branchInstr = dynamic_cast<BranchInstruction*>(terminator)) {
        nextBlocks.push_back(branchInstr->GetTrueBasicBlock());
        if (branchInstr->IsConditional()) {
            nextBlocks.push_back(branchInstr->GetFalseBasicBlock());
        }
    } else if (auto switchInstr = dynamic_cast<SwitchInstruction*>(terminator)) {
        nextBlocks.push_back(switchInstr->GetDefaultBasicBlock());
        for (const auto switchCase : switchInstr->GetCases()) {
            nextBlocks.push_back(switchCase.CaseBasicBlock);
        }
    }

    return nextBlocks;
}

std::list<Instruction*> BasicBlock::GetInstructions() const {
    return m_Instructions;
}  

void BasicBlock::AddPredecessor(BasicBlock* block) {
    m_Predecessors.push_back(block);
}
