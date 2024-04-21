#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/IntType.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class CompareInstruction: public Instruction {
public:
    enum class OpType {
        kNone = 0,

        kIULess, kIUGreater, kIULessEq, kIUGreaterEq,
        kISLess, kISGreater, kISLessEq, kISGreaterEq,
        kIEqual, kINEqual,

        kFLess, kFGreater, kFLessEq, kFGreaterEq,
        kFEqual, kFNEqual,
    };

public:
    CompareInstruction(OpType opType, const std::string& name,
                       Value* left, Value* right, BasicBlock* basicBlock)
            : Instruction(IntType::Create(left->GetProgram(), 1), basicBlock),
              m_OpType(opType), m_LeftOperand(left), m_RightOperand(right) {
        SetName(name);
    }

    Value* GetLeftOperand() const {
        return m_LeftOperand;
    }

    Value* GetRightOperand() const {
        return m_RightOperand;
    }

    OpType GetOpType() const {
        return m_OpType;
    }

    bool IsEqual() const {
        return m_OpType == OpType::kIEqual || m_OpType == OpType::kFEqual;
    }

    bool IsNEqual() const {
        return m_OpType == OpType::kINEqual || m_OpType == OpType::kFNEqual;
    }

    bool IsLess() const {
        return m_OpType == OpType::kISLess || m_OpType == OpType::kIULess ||
               m_OpType == OpType::kFLess;
    }

    bool IsLessEq() const {
        return m_OpType == OpType::kISLessEq || m_OpType == OpType::kIULessEq ||
               m_OpType == OpType::kFLessEq; 
    }

    bool IsGreater() const {
        return m_OpType == OpType::kISGreater || m_OpType == OpType::kIUGreater ||
               m_OpType == OpType::kFGreater;
    }

    bool IsGreaterEq() const {
        return m_OpType == OpType::kISGreaterEq || m_OpType == OpType::kIUGreaterEq ||
               m_OpType == OpType::kFGreaterEq;
    }

    bool IsUnsigned() const {
        return m_OpType == OpType::kIULess || m_OpType == OpType::kIUGreater ||
               m_OpType == OpType::kIULessEq || m_OpType == OpType::kIUGreaterEq;
    }

    bool IsFloat() const {
        return m_OpType == OpType::kFLess || m_OpType == OpType::kFGreater ||
               m_OpType == OpType::kFLessEq || m_OpType == OpType::kFGreaterEq ||
               m_OpType == OpType::kFEqual || m_OpType == OpType::kFNEqual;
    }

    std::string GetOpTypeStr() const {
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

private:
    OpType m_OpType = OpType::kNone;

    Value* m_LeftOperand;
    Value* m_RightOperand;
};

}  // namespace ir
