#pragma once

#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class BranchInstruction: public TerminatorInstruction {
public:
    // TODO: create VoidType
    BranchInstruction(Value* condition, BasicBlock* trueBB, BasicBlock* falseBB,
                      Type* type, BasicBlock* basicBlock)
        : TerminatorInstruction(type, basicBlock),
          m_Condition(condition), m_TrueBB(trueBB), m_FalseBB(falseBB) {}

    BranchInstruction(BasicBlock* trueBB, Type* type, BasicBlock* basicBlock)
        : TerminatorInstruction(type, basicBlock), m_TrueBB(trueBB) {}

    bool IsUnconditional() const {
        return !m_Condition;
    }

    Value* GetCondition() const {
        return m_Condition;
    }

    BasicBlock* GetTrueBasicBlock() const {
        return m_TrueBB;
    }

    BasicBlock* GetFalseBasicBlock() const {
        return m_FalseBB;
    }

private:
    Value* m_Condition = nullptr;
    BasicBlock* m_TrueBB = nullptr;
    BasicBlock* m_FalseBB = nullptr;
};

}  // namespace ir
