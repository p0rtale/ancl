#pragma once

#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/VoidType.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class ReturnInstruction: public TerminatorInstruction {
public:
    ReturnInstruction(Value* returnValue, BasicBlock* basicBlock)
        : TerminatorInstruction(VoidType::Create(returnValue->GetProgram()), basicBlock),
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
