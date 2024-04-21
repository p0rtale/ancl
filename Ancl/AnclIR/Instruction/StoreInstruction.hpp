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
    }

    Value* GetFromOperand() const {
        return m_FromOperand;
    }

    Value* GetToOperand() const {
        return m_ToOperand;
    }

private:
    Value* m_FromOperand;
    Value* m_ToOperand;
};

}  // namespace ir
