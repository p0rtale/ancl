#pragma once

#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class ReturnInstruction: public TerminatorInstruction {
public:
    // TODO: create VoidType
    ReturnInstruction(Value* returnValue, Type* type, BasicBlock* basicBlock)
        : TerminatorInstruction(type, basicBlock),
          m_ReturnValue(returnValue) {}

    bool HasReturnValue() const {
        return m_ReturnValue;
    }

    Value* GetReturnValue() const {
        return m_ReturnValue;
    }

private:
    Value* m_ReturnValue = nullptr;
};

}  // namespace ir
