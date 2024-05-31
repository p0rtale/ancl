#pragma once

#include <string>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class StoreInstruction: public Instruction {
public:
    StoreInstruction(Value* value, Value* address,
                     const std::string& name, BasicBlock* basicBlock);

    Value* GetValueOperand() const;
    Value* GetAddressOperand() const;

    void SetVolatile();
    bool IsVolatile() const;

private:
    bool m_IsVolatile = false;
};

}  // namespace ir
