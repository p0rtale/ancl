#pragma once

#include <vector>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class SwitchInstruction: public TerminatorInstruction {
public:
    struct SwitchCase {
        Value* CaseValue;
        BasicBlock* CaseBasicBlock;
    };

public:
    SwitchInstruction(Value* value, BasicBlock* defaultBlock, BasicBlock* basicBlock);

    Value* GetValue() const;

    bool HasDefaultBasicBlock() const;
    BasicBlock* GetDefaultBasicBlock() const;

    void AddCase(SwitchCase switchCase);

    std::vector<SwitchCase> GetCases() const;

    size_t GetCasesNumber() const;

private:
    Value* m_Value;

    BasicBlock* m_DefaultBB = nullptr;
    std::vector<SwitchCase> m_SwitchCases;
};

}  // namespace ir
