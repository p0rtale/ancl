#include <Ancl/AnclIR/Instruction/CastInstruction.hpp>


namespace ir {

CastInstruction::CastInstruction(OpType opType, const std::string& name,
                Value* fromValue, Type* toType,
                BasicBlock* basicBlock)
        : Instruction(toType, basicBlock), m_OpType(opType) {
    SetName(name);
    AddOperand(fromValue);
}

Value* CastInstruction::GetFromOperand() const {
    return GetOperand(0);
}

Type* CastInstruction::GetFromType() const {
    return GetOperand(0)->GetType();
}

Type* CastInstruction::GetToType() const {
    return GetType();
}

CastInstruction::OpType CastInstruction::GetOpType() const {
    return m_OpType;
}

std::string CastInstruction::GetOpTypeStr() const {
    switch (m_OpType) {
        case OpType::kNone:  return "None";

        case OpType::kITrunc:  return "itrunc";
        case OpType::kFTrunc:  return "ftrunc";
        
        case OpType::kZExt:  return "zext";
        case OpType::kSExt:  return "sext";
        case OpType::kFExt:  return "fext";

        case OpType::kFToUI:  return "ftoui";
        case OpType::kFToSI:  return "ftosi";

        case OpType::kUIToF:  return "uitof";
        case OpType::kSIToF:  return "sitof";

        case OpType::kPtrToI:  return "ptrtoi";
        case OpType::kIToPtr:  return "itoptr";

        case OpType::kBitcast:  return "bitcast";

        default: {
            return "";
        }
    }
}

}  // namespace ir
