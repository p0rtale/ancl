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
            : Instruction(memberType, basicBlock),
              m_PtrOperand(ptrValue), m_Index(index) {
        SetName(name);
    }

    void SetDeref(bool deref) {
        m_IsDeref = deref;
    }

    bool IsDeref() const {
        return m_IsDeref;
    }

    Value* GetPtrOperand() const {
        return m_PtrOperand;
    }

    Value* GetIndex() const {
        return m_Index;
    }

private:
    Value* m_PtrOperand;
    Value* m_Index;

    bool m_IsDeref = false;
};

}  // namespace ir
