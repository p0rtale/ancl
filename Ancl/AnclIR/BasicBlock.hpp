#pragma once

#include <list>
#include <string>
#include <vector>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Instruction/PhiInstruction.hpp>
#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Type/LabelType.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class Function;
class IRProgram;


// Represents block label
class BasicBlock: public Value {
public:
    BasicBlock(const std::string& name, LabelType* type, Function* function);

    IRProgram& GetProgram() const;

    Function* GetFunction() const;

    void AddInstructionToBegin(Instruction* instruction);

    void AddInstruction(Instruction* instruction);

    void InsertInstructionBeforeTerminator(Instruction* instruction);

    bool IsEmpty() const;

    void AddPhiFunction(PhiInstruction* phiInstruction);
    std::vector<PhiInstruction*> GetPhiFunctions() const;
    bool HasPhiFunctions() const;

    void ReplaceTerminator(TerminatorInstruction* terminator);
    bool IsTerminated() const;
    TerminatorInstruction* GetTerminator() const;

    std::vector<BasicBlock*> GetSuccessors() const;

    std::list<Instruction*> GetInstructions() const;
    std::list<Instruction*>& GetInstructionsRef();

    void AddPredecessor(BasicBlock* block);
    void AddPredecessorWithPhiValues(BasicBlock* block, const std::vector<Value*>& values);

    void ReplacePredecessor(BasicBlock* fromBlock, BasicBlock* toBlock);
    void RemovePredecessor(BasicBlock* block);

    std::vector<BasicBlock*> GetPredecessors() const;
    size_t GetPredecessorsNumber() const;

private:
    void handleNewTerminator(TerminatorInstruction* terminator);

private:
    Function* m_Function;

    // TODO: Generalize to the value "Use"
    std::vector<BasicBlock*> m_Predecessors;

    std::list<Instruction*> m_Instructions;
};

}  // namespace ir
