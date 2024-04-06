#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class StoreInstruction: public Instruction {
public:
    // TODO: create VoidType
    StoreInstruction(Value* fromPtr, Value* toValue, const std::string& name, Type* type, BasicBlock* basicBlock)
            : Instruction(type, basicBlock),
              m_FromPtrOperand(fromPtr), m_ToValueOperand(toValue) {
        SetName(name);
    }

    Value* GetFromPtrOperand() const {
        return m_FromPtrOperand;
    }

    Value* GetToValueOperand() const {
        return m_ToValueOperand;
    }

private:
    Value* m_FromPtrOperand;
    Value* m_ToValueOperand;
};

}  // namespace ir
