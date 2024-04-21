#pragma once

#include <string>
#include <vector>

#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/LabelType.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Instruction/SwitchInstruction.hpp>
#include <Ancl/AnclIR/Instruction/BranchInstruction.hpp>


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

    TerminatorInstruction* GetTerminator() const;

    std::vector<BasicBlock*> GetNextBlocks() const;

    std::vector<Instruction*> GetInstructions() const;

private:
    Function* m_Function;

    std::vector<Instruction*> m_Instructions;
};

}  // namespace ir
