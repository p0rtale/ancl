#pragma once

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class BranchInstruction: public TerminatorInstruction {
public:
    BranchInstruction(Value* condition, BasicBlock* trueBB, BasicBlock* falseBB,
                      BasicBlock* basicBlock);

    BranchInstruction(BasicBlock* trueBB, BasicBlock* basicBlock);

    bool IsConditional() const;
    bool IsUnconditional() const;

    Value* GetCondition() const;

    void SetTrueBasicBlock(BasicBlock* basicBlock);
    void SetFalseBasicBlock(BasicBlock* basicBlock);

    BasicBlock* GetTrueBasicBlock() const;
    BasicBlock* GetFalseBasicBlock() const;

    void ToUnconditional(BasicBlock* basicBlock);
    void ToUnconditionalTrue();
    void ToUnconditionalFalse();

private:
    BasicBlock* m_TrueBB = nullptr;
    BasicBlock* m_FalseBB = nullptr;
};

}  // namespace ir
