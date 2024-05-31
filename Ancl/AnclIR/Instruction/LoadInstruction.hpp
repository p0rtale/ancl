#pragma once

#include <string>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class LoadInstruction: public Instruction {
public:
    LoadInstruction(Value* pointer, Type* type, const std::string& name,
                    BasicBlock* basicBlock);

    Value* GetPtrOperand() const;

    void SetVolatile();
    bool IsVolatile() const;

private:
    bool m_IsVolatile = false;
};

}  // namespace ir
