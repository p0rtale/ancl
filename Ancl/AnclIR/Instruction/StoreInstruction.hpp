#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/VoidType.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class StoreInstruction: public Instruction {
public:
    StoreInstruction(Value* value, Value* address, const std::string& name, BasicBlock* basicBlock)
            : Instruction(VoidType::Create(value->GetProgram()), basicBlock) {
        SetName(name);
        AddOperand(value);
        AddOperand(address);
    }

    Value* GetValueOperand() const {
        return GetOperand(0);
    }

    Value* GetAddressOperand() const {
        return GetOperand(1);
    }

    void SetVolatile() {
        m_IsVolatile = true;
    }

    bool IsVolatile() const {
        return m_IsVolatile;
    }

private:
    bool m_IsVolatile = false;
};

}  // namespace ir
