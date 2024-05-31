#pragma once

#include <string>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class MemberInstruction: public Instruction {
public:
    MemberInstruction(Value* ptrValue, Value* index,
                      const std::string& name, Type* memberType, BasicBlock* basicBlock);

    void SetDeref(bool deref);
    bool IsDeref() const;

    Value* GetPtrOperand() const;
    Value* GetIndex() const;

private:
    bool m_IsDeref = false;
};

}  // namespace ir
