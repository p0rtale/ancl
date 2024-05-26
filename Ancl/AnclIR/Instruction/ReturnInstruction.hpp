#pragma once

#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/VoidType.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class ReturnInstruction: public TerminatorInstruction {
public:
    ReturnInstruction(BasicBlock* basicBlock)
            : TerminatorInstruction(VoidType::Create(basicBlock->GetProgram()), basicBlock) {
        AddOperand(nullptr);
    }

    ReturnInstruction(Value* returnValue, BasicBlock* basicBlock)
            : TerminatorInstruction(returnValue->GetType(), basicBlock) {
        AddOperand(returnValue);
    }

    bool HasReturnValue() const {
        return HasOperand(0);
    }

    Value* GetReturnValue() const {
        return GetOperand(0);
    }
};

}  // namespace ir
