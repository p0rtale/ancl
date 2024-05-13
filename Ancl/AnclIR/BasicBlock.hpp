#pragma once

#include <vector>
#include <string>
#include <list>

#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/LabelType.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Instruction/PhiInstruction.hpp>


namespace ir {

class Function;
class IRProgram;


// Represents block label
class BasicBlock: public Value {
public:
    BasicBlock(const std::string& name, LabelType* type, Function* function);

    IRProgram& GetProgram() const;

    Function* GetFunction() const;

    void AddInstruction(Instruction* instruction);

    void AddPhiFunction(PhiInstruction* phiInstruction) {
        m_Instructions.push_front(phiInstruction);
    }

    std::vector<PhiInstruction*> GetPhiFunctions() const {
        std::vector<PhiInstruction*> phis;
        for (Instruction* instruction : m_Instructions) {
            if (auto* phiInstr = dynamic_cast<PhiInstruction*>(instruction)) {
                phis.push_back(phiInstr);
            }
        }
        return phis;
    }

    TerminatorInstruction* GetTerminator() const;

    std::vector<BasicBlock*> GetSuccessors() const;

    std::list<Instruction*> GetInstructions() const;

    void AddPredecessor(BasicBlock* block);

    std::vector<BasicBlock*> GetPredecessors() const {
        return m_Predecessors;
    }

    size_t GetPredecessorsNumber() const {
        return m_Predecessors.size();
    }

private:
    Function* m_Function;

    // TODO: Generalize to the value "Use"
    std::vector<BasicBlock*> m_Predecessors;

    std::list<Instruction*> m_Instructions;
};

}  // namespace ir
