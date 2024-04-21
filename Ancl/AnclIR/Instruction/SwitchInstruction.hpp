#pragma once

#include <vector>

#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/VoidType.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class SwitchInstruction: public TerminatorInstruction {
public:
    struct SwitchCase {
        Value* CaseValue;
        BasicBlock* CaseBasicBlock;
    };

public:
    SwitchInstruction(Value* value, BasicBlock* basicBlock)
        : TerminatorInstruction(VoidType::Create(value->GetProgram()), basicBlock),
          m_Value(value) {}

    Value* GetValue() const {
        return m_Value;
    }

    bool HasDefaultBasicBlock() const {
        return m_DefaultBB;
    }

    BasicBlock* GetDefaultBasicBlock() const {
        return m_DefaultBB;
    }

    void AddCase(SwitchCase switchCase) {
        m_SwitchCases.push_back(switchCase);
    }

    std::vector<SwitchCase> GetCases() const {
        return m_SwitchCases;
    }

    size_t GetCasesNumber() const {
        return m_SwitchCases.size();
    }

private:
    Value* m_Value;

    BasicBlock* m_DefaultBB = nullptr;
    std::vector<SwitchCase> m_SwitchCases;
};

}  // namespace ir
