#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/VoidType.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class StoreInstruction: public Instruction {
public:
    StoreInstruction(Value* fromValue, Value* toValue, const std::string& name, BasicBlock* basicBlock)
            : Instruction(VoidType::Create(fromValue->GetProgram()), basicBlock),
              m_FromOperand(fromValue), m_ToOperand(toValue) {
        SetName(name);
        AddOperand(fromValue);
        AddOperand(toValue);
    }

    Value* GetFromOperand() const {
        return m_FromOperand;
    }

    Value* GetToOperand() const {
        return m_ToOperand;
    }

    void SetVolatile() {
        m_IsVolatile = true;
    }

    bool IsVolatile() const {
        return m_IsVolatile;
    }

private:
    bool m_IsVolatile = false;

    Value* m_FromOperand;
    Value* m_ToOperand;
};

}  // namespace ir
