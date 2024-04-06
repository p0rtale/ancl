#pragma once

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

private:
    BasicBlock* m_BasicBlock;
};

}  // namespace ir
