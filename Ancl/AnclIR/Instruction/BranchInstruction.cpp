#include <Ancl/AnclIR/Instruction/BranchInstruction.hpp>
#include <Ancl/AnclIR/Type/VoidType.hpp>

using namespace ir;


BranchInstruction::BranchInstruction(Value* condition, BasicBlock* trueBB,
                                     BasicBlock* falseBB, BasicBlock* basicBlock)
    : TerminatorInstruction(VoidType::Create(trueBB->GetProgram()), basicBlock),
      m_Condition(condition), m_TrueBB(trueBB), m_FalseBB(falseBB) {}

BranchInstruction::BranchInstruction(BasicBlock* trueBB, BasicBlock* basicBlock)
    : TerminatorInstruction(VoidType::Create(trueBB->GetProgram()), basicBlock),
      m_TrueBB(trueBB) {}

bool BranchInstruction::IsConditional() const {
    return m_Condition;
}

bool BranchInstruction::IsUnconditional() const {
    return !m_Condition;
}

Value* BranchInstruction::GetCondition() const {
    return m_Condition;
}

BasicBlock* BranchInstruction::GetTrueBasicBlock() const {
    return m_TrueBB;
}

BasicBlock* BranchInstruction::GetFalseBasicBlock() const {
    return m_FalseBB;
}