#include <Ancl/AnclIR/Instruction/BranchInstruction.hpp>

#include <Ancl/AnclIR/Type/VoidType.hpp>


namespace ir {

BranchInstruction::BranchInstruction(Value* condition, BasicBlock* trueBB,
                                     BasicBlock* falseBB, BasicBlock* basicBlock)
        : TerminatorInstruction(VoidType::Create(trueBB->GetProgram()), basicBlock),
          m_TrueBB(trueBB), m_FalseBB(falseBB) {
    // TODO: Blocks uses?
    AddOperand(condition);

    trueBB->AddPredecessor(basicBlock);
    falseBB->AddPredecessor(basicBlock);
}

BranchInstruction::BranchInstruction(BasicBlock* trueBB, BasicBlock* basicBlock)
        : TerminatorInstruction(VoidType::Create(trueBB->GetProgram()), basicBlock),
          m_TrueBB(trueBB) {
    trueBB->AddPredecessor(basicBlock);
}

bool BranchInstruction::IsConditional() const {
    return HasOperand(0);
}

bool BranchInstruction::IsUnconditional() const {
    return !HasOperand(0);
}

Value* BranchInstruction::GetCondition() const {
    return GetOperand(0);
}

void BranchInstruction::SetTrueBasicBlock(BasicBlock* basicBlock) {
    m_TrueBB = basicBlock;
}

void BranchInstruction::SetFalseBasicBlock(BasicBlock* basicBlock) {
    m_FalseBB = basicBlock;
}

BasicBlock* BranchInstruction::GetTrueBasicBlock() const {
    return m_TrueBB;
}

BasicBlock* BranchInstruction::GetFalseBasicBlock() const {
    return m_FalseBB;
}

void BranchInstruction::ToUnconditional(BasicBlock* basicBlock) {
    BasicBlock* branchBlock = GetBasicBlock();
    if (m_TrueBB) {
        m_TrueBB->RemovePredecessor(branchBlock);
    }
    if (m_FalseBB) {
        m_FalseBB->RemovePredecessor(branchBlock);
    }
    ClearOperands();

    m_TrueBB = basicBlock;
}

void BranchInstruction::ToUnconditionalTrue() {
    if (IsUnconditional()) {
        return;
    }

    BasicBlock* branchBlock = GetBasicBlock();
    m_FalseBB->RemovePredecessor(branchBlock);
    ClearOperands();

    m_FalseBB = nullptr;
}

void BranchInstruction::ToUnconditionalFalse() {
    if (IsUnconditional()) {
        return;
    }

    BasicBlock* branchBlock = GetBasicBlock();
    m_TrueBB->RemovePredecessor(branchBlock);
    ClearOperands();

    m_TrueBB = m_FalseBB;
    m_FalseBB = nullptr;
}

}  // namespace ir
