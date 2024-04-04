#pragma once

#include <vector>

#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/LabelType.hpp>
#include <Ancl/AnclIR/Constant/Function.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>


namespace ir {

// Represents block label
class BasicBlock: public Value {
public:
    BasicBlock(LabelType* type, Function* function):
        Value(type), m_Function(function) {}

    Function* GetFunction() const {
        return m_Function;
    }

    void AddInstruction(Instruction* instruction) {
        m_Instructions.push_back(instruction);
    }

    std::vector<Instruction*> GetInstructions() const {
        return m_Instructions;
    }

private:
    Function* m_Function;

    std::vector<Instruction*> m_Instructions;
};

}  // namespace ir
