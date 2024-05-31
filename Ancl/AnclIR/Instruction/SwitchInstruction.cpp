#include <Ancl/AnclIR/Instruction/SwitchInstruction.hpp>

#include <Ancl/AnclIR/Type/VoidType.hpp>


namespace ir {

SwitchInstruction::SwitchInstruction(Value* value, BasicBlock* defaultBlock,
                                     BasicBlock* basicBlock)
        : TerminatorInstruction(VoidType::Create(value->GetProgram()), basicBlock),
          m_Value(value), m_DefaultBB(defaultBlock) {
    // TODO: Cases uses?
    AddOperand(value);
    AddOperand(defaultBlock);

    // TODO: Link Basic Blocks
}

Value* SwitchInstruction::GetValue() const {
    return m_Value;
}

bool SwitchInstruction::HasDefaultBasicBlock() const {
    return m_DefaultBB;
}

BasicBlock* SwitchInstruction::GetDefaultBasicBlock() const {
    return m_DefaultBB;
}

void SwitchInstruction::AddCase(SwitchCase switchCase) {
    m_SwitchCases.push_back(switchCase);
}

std::vector<SwitchInstruction::SwitchCase> SwitchInstruction::GetCases() const {
    return m_SwitchCases;
}

size_t SwitchInstruction::GetCasesNumber() const {
    return m_SwitchCases.size();
}

}  // namespace ir
