#pragma once

#include <string>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Value.hpp>


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
        kBitcast,
    };

public:
    CastInstruction(OpType opType, const std::string& name,
                    Value* fromValue, Type* toType, BasicBlock* basicBlock);

    Value* GetFromOperand() const;

    Type* GetFromType() const;
    Type* GetToType() const;

    OpType GetOpType() const;
    std::string GetOpTypeStr() const;

private:
    OpType m_OpType = OpType::kNone;
};

}  // namespace ir
