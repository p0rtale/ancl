#include <Ancl/AnclIR/Instruction/MemberInstruction.hpp>


namespace ir {

// TODO: create member Type
MemberInstruction::MemberInstruction(Value* ptrValue, Value* index,
                                     const std::string& name, Type* memberType,
                                     BasicBlock* basicBlock)
        : Instruction(memberType, basicBlock) {
    SetName(name);
    AddOperand(ptrValue);
    AddOperand(index);
}

void MemberInstruction::SetDeref(bool deref) {
    m_IsDeref = deref;
}

bool MemberInstruction::IsDeref() const {
    return m_IsDeref;
}

Value* MemberInstruction::GetPtrOperand() const {
    return GetOperand(0);
}

Value* MemberInstruction::GetIndex() const {
    return GetOperand(1);
}

}  // namespace ir
