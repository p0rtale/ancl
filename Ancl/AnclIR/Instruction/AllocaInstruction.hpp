#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class AllocaInstruction: public Instruction {
public:
    // TODO: create PointerType from Type
    AllocaInstruction(Type* type, const std::string& name, BasicBlock* basicBlock)
            : Instruction(type, basicBlock),
              m_AllocaType(type) {
        SetName(name);
    }

    Type* GetAllocaType() const {
        return m_AllocaType;
    }

private:
    Type* m_AllocaType;
};

}  // namespace ir
