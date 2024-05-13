#pragma once

#include <vector>

#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class BasicBlock;

/*
    %add = add %0 %1

    %add - instruction
    %0 and %1 - instruction values
*/
class Instruction: public Value {
public:
    Instruction(Type* type, BasicBlock* basicBlock)
        : Value(type), m_BasicBlock(basicBlock) {}

    BasicBlock* GetBasicBlock() const {
        return m_BasicBlock;
    }

    void AddOperand(Value* operand) {
        m_Operands.push_back(operand);
    }

    void SetOperand(Value* operand, size_t index) {
        m_Operands[index] = operand;
    }

    std::vector<Value*> GetOperands() const {
        return m_Operands;
    }

private:
    BasicBlock* m_BasicBlock;

    std::vector<Value*> m_Operands;
};

}  // namespace ir
