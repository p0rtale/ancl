#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class LoadInstruction: public Instruction {
public:
    LoadInstruction(Value* pointer, Type* type, const std::string& name, BasicBlock* basicBlock)
            : Instruction(type, basicBlock),
              m_PtrOperand(pointer) {
        SetName(name);
        AddOperand(pointer);
    }

    Value* GetPtrOperand() const {
        return m_PtrOperand;
    }

    void SetVolatile() {
        m_IsVolatile = true;
    }

    bool IsVolatile() const {
        return m_IsVolatile;
    }

private:
    bool m_IsVolatile = false;

    Value* m_PtrOperand;
};

}  // namespace ir
