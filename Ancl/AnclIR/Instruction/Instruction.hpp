#pragma once

#include <vector>

#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class BasicBlock;

/*
    %add = add %0 %1

    %add - instruction
    %0 and %1 - instruction values
*/
class Instruction: public Value {
public:
    Instruction(Type* type, BasicBlock* basicBlock);

    void SetName(const std::string& name);

    void SetBasicBlock(BasicBlock* basicBlock);
    BasicBlock* GetBasicBlock() const;

    void AddOperand(Value* operand);
    void DeleteOperand(size_t index);

    bool HasOperand(size_t index) const;
    Value* GetOperand(size_t index) const;
    void SetOperand(Value* operand, size_t index);

    void ClearOperands();

    bool HasOperands() const;
    size_t GetOperandsNumber() const;
    std::vector<Value*> GetOperands() const;

private:
    BasicBlock* m_BasicBlock;

    std::vector<Value*> m_Operands;
};

}  // namespace ir
