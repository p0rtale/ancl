#pragma once

#include <string>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class MemberInstruction: public Instruction {
public:
    // TODO: create member Type
    MemberInstruction(Value* ptrValue, Value* index,
                      const std::string& name, Type* memberType, BasicBlock* basicBlock)
            : Instruction(memberType, basicBlock) {
        SetName(name);
        AddOperand(ptrValue);
        AddOperand(index);
    }

    void SetDeref(bool deref) {
        m_IsDeref = deref;
    }

    bool IsDeref() const {
        return m_IsDeref;
    }

    Value* GetPtrOperand() const {
        return GetOperand(0);
    }

    Value* GetIndex() const {
        return GetOperand(1);
    }

private:
    bool m_IsDeref = false;
};

}  // namespace ir
