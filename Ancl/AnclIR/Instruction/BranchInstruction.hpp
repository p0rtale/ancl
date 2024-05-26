#pragma once

#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class BranchInstruction: public TerminatorInstruction {
public:
    BranchInstruction(Value* condition, BasicBlock* trueBB, BasicBlock* falseBB,
                      BasicBlock* basicBlock);

    BranchInstruction(BasicBlock* trueBB, BasicBlock* basicBlock);

    bool IsConditional() const;

    bool IsUnconditional() const;

    Value* GetCondition() const;

    BasicBlock* GetTrueBasicBlock() const;

    BasicBlock* GetFalseBasicBlock() const;

private:
    BasicBlock* m_TrueBB = nullptr;
    BasicBlock* m_FalseBB = nullptr;
};

}  // namespace ir
