#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
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
    // TODO: create IntType 1
    CompareInstruction(OpType opType, const std::string& name,
                       Value* left, Value* right,
                       Type* type, BasicBlock* basicBlock)
            : Instruction(type, basicBlock), m_OpType(opType),
              m_LeftOperand(left), m_RightOperand(right) {
        SetName(name);
    }

    Value* GetLeftOperand() const {
        return m_LeftOperand;
    }

    Value* GetRightOperand() const {
        return m_RightOperand;
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
