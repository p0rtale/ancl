#pragma once

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class ReturnInstruction: public TerminatorInstruction {
public:
    ReturnInstruction(BasicBlock* basicBlock);
    ReturnInstruction(Value* returnValue, BasicBlock* basicBlock);

    bool HasReturnValue() const;

    Value* GetReturnValue() const;
};

}  // namespace ir
