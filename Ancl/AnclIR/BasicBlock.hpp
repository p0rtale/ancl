#pragma once

#include <vector>

#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/LabelType.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Instruction/SwitchInstruction.hpp>
#include <Ancl/AnclIR/Instruction/BranchInstruction.hpp>


namespace ir {

class Function;

// Represents block label
class BasicBlock: public Value {
public:
    BasicBlock(const std::string& name, LabelType* type, Function* function):
            Value(type), m_Function(function) {
        SetName(name);
    }

    Function* GetFunction() const {
        return m_Function;
    }

    void AddInstruction(Instruction* instruction) {
        m_Instructions.push_back(instruction);
    }

    TerminatorInstruction* GetTerminator() const {
        if (m_Instructions.empty()) {
            return nullptr;
        }
        auto instruction = m_Instructions[m_Instructions.size() - 1];
        return dynamic_cast<TerminatorInstruction*>(instruction);
    }

    std::vector<BasicBlock*> GetNextBlocks() const {
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

    std::vector<Instruction*> GetInstructions() const {
        return m_Instructions;
    }    

private:
    Function* m_Function;

    std::vector<Instruction*> m_Instructions;
};

}  // namespace ir
