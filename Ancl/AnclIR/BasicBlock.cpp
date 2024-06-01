#include <Ancl/AnclIR/BasicBlock.hpp>

#include <cassert>

#include <Ancl/AnclIR/IRProgram.hpp>
#include <Ancl/AnclIR/Instruction/BranchInstruction.hpp>
#include <Ancl/AnclIR/Instruction/SwitchInstruction.hpp>


namespace ir {

BasicBlock::BasicBlock(const std::string& name, LabelType* type, Function* function)
        : Value(type), m_Function(function) {
    SetName(m_Function->GetNewBasicBlockName(name));
}

IRProgram& BasicBlock::GetProgram() const {
    return m_Function->GetProgram();
}

Function* BasicBlock::GetFunction() const {
    return m_Function;
}

void BasicBlock::AddInstructionToBegin(Instruction* instruction) {
    instruction->SetBasicBlock(this);
    m_Instructions.push_front(instruction);
}

void BasicBlock::AddInstruction(Instruction* instruction) {
    if (!m_Instructions.empty() &&
                dynamic_cast<ir::TerminatorInstruction*>(m_Instructions.back())) {
        return;
    }

    if (auto* terminator = dynamic_cast<TerminatorInstruction*>(instruction)) {
        handleNewTerminator(terminator);
    }

    instruction->SetBasicBlock(this);
    m_Instructions.push_back(instruction);
}

void BasicBlock::InsertInstructionBeforeTerminator(Instruction* instruction) {
    instruction->SetBasicBlock(this);
    auto lastInstrIt = --m_Instructions.end();
    m_Instructions.insert(lastInstrIt, instruction);
}

bool BasicBlock::IsEmpty() const {
    // Do not take into account the terminator
    return m_Instructions.size() == 1;
}

void BasicBlock::AddPhiFunction(PhiInstruction* phiInstruction) {
    phiInstruction->SetBasicBlock(this);
    m_Instructions.push_front(phiInstruction);
}

std::vector<PhiInstruction*> BasicBlock::GetPhiFunctions() const {
    std::vector<PhiInstruction*> phis;
    for (Instruction* instruction : m_Instructions) {
        if (auto* phiInstr = dynamic_cast<PhiInstruction*>(instruction)) {
            phis.push_back(phiInstr);
        }
    }
    return phis;
}

bool BasicBlock::HasPhiFunctions() const {
    for (Instruction* instruction : m_Instructions) {
        if (auto* phiInstr = dynamic_cast<PhiInstruction*>(instruction)) {
            return true;
        }
    }
    return false;
}

void BasicBlock::ReplaceTerminator(TerminatorInstruction* terminator) {
    assert(!m_Instructions.empty());
    handleNewTerminator(terminator);
    terminator->SetBasicBlock(this);
    m_Instructions.back() = terminator;
}

bool BasicBlock::IsTerminated() const {
    return GetTerminator();
}

TerminatorInstruction* BasicBlock::GetTerminator() const {
    if (m_Instructions.empty()) {
        return nullptr;
    }
    return dynamic_cast<TerminatorInstruction*>(m_Instructions.back());
}

std::vector<BasicBlock*> BasicBlock::GetSuccessors() const {
    TerminatorInstruction* terminator = GetTerminator();
    std::vector<BasicBlock*> nextBlocks;
    if (auto* branchInstr = dynamic_cast<BranchInstruction*>(terminator)) {
        nextBlocks.push_back(branchInstr->GetTrueBasicBlock());
        if (branchInstr->IsConditional()) {
            nextBlocks.push_back(branchInstr->GetFalseBasicBlock());
        }
    } else if (auto* switchInstr = dynamic_cast<SwitchInstruction*>(terminator)) {
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

std::list<Instruction*>& BasicBlock::GetInstructionsRef() {
    return m_Instructions;
}  

void BasicBlock::AddPredecessor(BasicBlock* block) {
    for (PhiInstruction* phi : GetPhiFunctions()) {
        phi->AddArgument(block, nullptr);
    }
    m_Predecessors.push_back(block);
}

void BasicBlock::AddPredecessorWithPhiValues(BasicBlock* block, const std::vector<Value*>& values) {
    size_t i = 0;
    for (PhiInstruction* phi : GetPhiFunctions()) {
        phi->AddArgument(block, values.at(i++));
    }
    m_Predecessors.push_back(block);
}

void BasicBlock::ReplacePredecessor(BasicBlock* fromBlock, BasicBlock* toBlock) {
    for (size_t i = 0; i < m_Predecessors.size(); ++i) {
        if (m_Predecessors[i] == fromBlock) {
            for (PhiInstruction* phi : GetPhiFunctions()) {
                phi->SetIncomingBlock(i, toBlock);
                // phi->SetIncomingValue(i, nullptr);
            }
            m_Predecessors[i] = toBlock;
            return;
        }
    }
}

void BasicBlock::RemovePredecessor(BasicBlock* block) {
    // TODO: Use list of blocks?
    for (size_t i = 0; i < m_Predecessors.size(); ++i) {
        if (m_Predecessors[i] == block) {
            for (PhiInstruction* phi : GetPhiFunctions()) {
                phi->DeleteArgument(i);
            }
            m_Predecessors.erase(m_Predecessors.begin() + i);
            return;
        }
    }
}

std::vector<BasicBlock*> BasicBlock::GetPredecessors() const {
    return m_Predecessors;
}

size_t BasicBlock::GetPredecessorsNumber() const {
    return m_Predecessors.size();
}

void BasicBlock::handleNewTerminator(TerminatorInstruction* terminator) {
    // TODO: switch
    if (auto* branch = dynamic_cast<ir::BranchInstruction*>(terminator)) {
        branch->GetTrueBasicBlock()->AddPredecessor(this);
        if (branch->IsConditional()) {
            branch->GetFalseBasicBlock()->AddPredecessor(this);
        }
    }
}

}  // namespace ir
