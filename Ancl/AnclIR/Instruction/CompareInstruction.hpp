#pragma once

#include <string>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>


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
                       Value* left, Value* right, BasicBlock* basicBlock);

    Value* GetLeftOperand() const;
    Value* GetRightOperand() const;

    bool IsEqual() const;
    bool IsNEqual() const;
    bool IsLess() const;
    bool IsLessEq() const;
    bool IsGreater() const;
    bool IsGreaterEq() const;
    bool IsUnsigned() const;
    bool IsFloat() const;

    OpType GetOpType() const;
    std::string GetOpTypeStr() const;

private:
    OpType m_OpType = OpType::kNone;
};

}  // namespace ir
