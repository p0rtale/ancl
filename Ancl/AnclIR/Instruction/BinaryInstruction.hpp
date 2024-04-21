#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class BinaryInstruction: public Instruction {
public:
    enum class OpType {
        kNone = 0,
        kMul, kFMul,
        kSDiv, kUDiv, kFDiv,
        kSRem, kURem, kFRem,
        kAdd, kFAdd,
        kSub, kFSub,
        kShiftL, kLShiftR, kAShiftR,
        kAnd, kXor, kOr,
    };

public:
    BinaryInstruction(OpType opType, const std::string& name,
                      Value* left, Value* right,
                      BasicBlock* basicBlock)
            : Instruction(left->GetType(), basicBlock), m_OpType(opType),
              m_LeftOperand(left), m_RightOperand(right) {
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

    std::string GetOpTypeStr() const {
        switch (m_OpType) {
            case OpType::kNone:  return "None";

            case OpType::kMul:   return "mul";
            case OpType::kFMul:  return "fmul";
            
            case OpType::kSDiv:  return "sdiv";
            case OpType::kUDiv:  return "udiv";
            case OpType::kFDiv:  return "fdiv";

            case OpType::kSRem:  return "srem";
            case OpType::kURem:  return "urem";
            case OpType::kFRem:  return "frem";

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

private:
    OpType m_OpType = OpType::kNone;

    Value* m_LeftOperand;
    Value* m_RightOperand;
};

}  // namespace ir
