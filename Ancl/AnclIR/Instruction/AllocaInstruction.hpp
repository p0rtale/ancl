#pragma once

#include <string>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/PointerType.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class AllocaInstruction: public Instruction {
public:
    AllocaInstruction(Type* type, const std::string& name, BasicBlock* basicBlock);

    Type* GetAllocaType() const;

private:
    Type* m_AllocaType;
};

}  // namespace ir
