#include <Ancl/AnclIR/Instruction/CompareInstruction.hpp>

#include <Ancl/AnclIR/Type/IntType.hpp>


namespace ir {

CompareInstruction::CompareInstruction(OpType opType, const std::string& name,
                                       Value* left, Value* right, BasicBlock* basicBlock)
        : Instruction(IntType::Create(left->GetProgram(), 1), basicBlock),
          m_OpType(opType) {
    SetName(name);
    AddOperand(left);
    AddOperand(right);
}

Value* CompareInstruction::GetLeftOperand() const {
    return GetOperand(0);
}

Value* CompareInstruction::GetRightOperand() const {
    return GetOperand(1);
}

bool CompareInstruction::IsEqual() const {
    return m_OpType == OpType::kIEqual || m_OpType == OpType::kFEqual;
}

bool CompareInstruction::IsNEqual() const {
    return m_OpType == OpType::kINEqual || m_OpType == OpType::kFNEqual;
}

bool CompareInstruction::IsLess() const {
    return m_OpType == OpType::kISLess || m_OpType == OpType::kIULess ||
            m_OpType == OpType::kFLess;
}

bool CompareInstruction::IsLessEq() const {
    return m_OpType == OpType::kISLessEq || m_OpType == OpType::kIULessEq ||
            m_OpType == OpType::kFLessEq; 
}

bool CompareInstruction::IsGreater() const {
    return m_OpType == OpType::kISGreater || m_OpType == OpType::kIUGreater ||
            m_OpType == OpType::kFGreater;
}

bool CompareInstruction::IsGreaterEq() const {
    return m_OpType == OpType::kISGreaterEq || m_OpType == OpType::kIUGreaterEq ||
            m_OpType == OpType::kFGreaterEq;
}

bool CompareInstruction::IsUnsigned() const {
    return m_OpType == OpType::kIULess || m_OpType == OpType::kIUGreater ||
            m_OpType == OpType::kIULessEq || m_OpType == OpType::kIUGreaterEq;
}

bool CompareInstruction::IsFloat() const {
    return m_OpType == OpType::kFLess || m_OpType == OpType::kFGreater ||
            m_OpType == OpType::kFLessEq || m_OpType == OpType::kFGreaterEq ||
            m_OpType == OpType::kFEqual || m_OpType == OpType::kFNEqual;
}

CompareInstruction::OpType CompareInstruction::GetOpType() const {
    return m_OpType;
}

std::string CompareInstruction::GetOpTypeStr() const {
    switch (m_OpType) {
        case OpType::kNone:  return "None";

        case OpType::kIULess:       return "icmp ult";
        case OpType::kIUGreater:    return "icmp ugt";
        case OpType::kIULessEq:     return "icmp ule";
        case OpType::kIUGreaterEq:  return "icmp uge";
        case OpType::kISLess:       return "icmp slt";
        case OpType::kISGreater:    return "icmp sgt";
        case OpType::kISLessEq:     return "icmp sle";
        case OpType::kISGreaterEq:  return "icmp sge";
        case OpType::kIEqual:       return "icmp eq";
        case OpType::kINEqual:      return "icmp ne";

        case OpType::kFLess:       return "fcmp lt";
        case OpType::kFGreater:    return "fcmp gt";
        case OpType::kFLessEq:     return "fcmp le";
        case OpType::kFGreaterEq:  return "fcmp ge";
        case OpType::kFEqual:      return "fcmp eq";
        case OpType::kFNEqual:     return "fcmp ne";

        default: {
            return "";
        }
    }
}

}  // namespace ir
