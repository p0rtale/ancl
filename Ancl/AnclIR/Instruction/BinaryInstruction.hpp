#pragma once

#include <string>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class BinaryInstruction: public Instruction {
public:
    enum class OpType {
        kNone = 0,
        kMul, kFMul,
        kSDiv, kUDiv, kFDiv,
        kSRem, kURem,
        kAdd, kFAdd,
        kSub, kFSub,
        kShiftL, kLShiftR, kAShiftR,
        kAnd, kXor, kOr,
    };

public:
    BinaryInstruction(OpType opType, const std::string& name,
                      Value* left, Value* right,
                      BasicBlock* basicBlock);

    Value* GetLeftOperand() const;
    Value* GetRightOperand() const;

    bool IsCommutative() const;

    OpType GetOpType() const;
    std::string GetOpTypeStr() const;

private:
    OpType m_OpType = OpType::kNone;
};

}  // namespace ir
