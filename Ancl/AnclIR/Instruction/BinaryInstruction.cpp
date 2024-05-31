#include <Ancl/AnclIR/Instruction/BinaryInstruction.hpp>


namespace ir {

BinaryInstruction::BinaryInstruction(OpType opType, const std::string& name,
                                     Value* left, Value* right,
                                     BasicBlock* basicBlock)
        : Instruction(left->GetType(), basicBlock), m_OpType(opType) {
    SetName(name);
    AddOperand(left);
    AddOperand(right);
}

Value* BinaryInstruction::GetLeftOperand() const {
    return GetOperand(0);
}

Value* BinaryInstruction::GetRightOperand() const {
    return GetOperand(1);
}

bool BinaryInstruction::IsCommutative() const {
    switch (m_OpType) {
        case OpType::kMul:
        case OpType::kFMul:
        case OpType::kAdd:
        case OpType::kFAdd:
        case OpType::kAnd:
        case OpType::kXor:
        case OpType::kOr:
            return true;

        default:
            return false;
    }
}

BinaryInstruction::OpType BinaryInstruction::GetOpType() const {
    return m_OpType;
}

std::string BinaryInstruction::GetOpTypeStr() const {
    switch (m_OpType) {
        case OpType::kNone:  return "None";

        case OpType::kMul:   return "mul";
        case OpType::kFMul:  return "fmul";
        
        case OpType::kSDiv:  return "sdiv";
        case OpType::kUDiv:  return "udiv";
        case OpType::kFDiv:  return "fdiv";

        case OpType::kSRem:  return "srem";
        case OpType::kURem:  return "urem";

        case OpType::kAdd:   return "add";
        case OpType::kFAdd:  return "fadd";

        case OpType::kSub:   return "sub";
        case OpType::kFSub:  return "fsub";

        case OpType::kShiftL:   return "shl";
        case OpType::kLShiftR:  return "lshr";
        case OpType::kAShiftR:  return "ashr";

        case OpType::kAnd:  return "and";
        case OpType::kXor:  return "xor";
        case OpType::kOr:   return "or";

        default: {
            return "";
        }
    }
}

}  // namespace ir
