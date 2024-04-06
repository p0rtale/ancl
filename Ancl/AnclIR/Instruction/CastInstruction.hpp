#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class CastInstruction: public Instruction {
public:
    enum class OpType {
        kNone = 0,
        kITrunc, kFTrunc,
        kZExt, kSExt, kFExt,
        kFToUI, kFToSI,
        kUIToF, kSIToF,
        kPtrToI, kIToPtr,
    };

public:
    CastInstruction(OpType opType, const std::string& name,
                    Value* fromValue, Type* toType,
                    BasicBlock* basicBlock)
            : Instruction(toType, basicBlock), m_OpType(opType),
              m_FromOperand(fromValue) {
        SetName(name);
    }

    Value* GetFromOperand() const {
        return m_FromOperand;
    }

    Type* GetFromType() const {
        return m_FromOperand->GetType();
    }

    Type* GetToType() const {
        return GetType();
    }

    std::string GetOpTypeStr() const {
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

            default: {
                return "";
            }
        }
    }

private:
    OpType m_OpType = OpType::kNone;

    Value* m_FromOperand;
};

}  // namespace ir
