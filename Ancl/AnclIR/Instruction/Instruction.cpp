#include <Ancl/AnclIR/Instruction/Instruction.hpp>

#include <cassert>

#include <Ancl/AnclIR/Constant/Function.hpp>


namespace ir {

Instruction::Instruction(Type* type, BasicBlock* basicBlock)
    : Value(type), m_BasicBlock(basicBlock) {}

void Instruction::SetName(const std::string& name) {
    Function* function = m_BasicBlock->GetFunction();
    Value::SetName(function->GetNewInstructionName(name));
}

void Instruction::SetBasicBlock(BasicBlock* basicBlock) {
    m_BasicBlock = basicBlock;
}

BasicBlock* Instruction::GetBasicBlock() const {
    return m_BasicBlock;
}

void Instruction::AddOperand(Value* operand) {
    m_Operands.push_back(operand);
}

void Instruction::DeleteOperand(size_t index) {
    assert(HasOperand(index));
    m_Operands.erase(m_Operands.begin() + index);
}

bool Instruction::HasOperand(size_t index) const {
    return index < m_Operands.size();
}

Value* Instruction::GetOperand(size_t index) const {
    return m_Operands.at(index);
}

void Instruction::SetOperand(Value* operand, size_t index) {
    m_Operands[index] = operand;
}

void Instruction::ClearOperands() {
    m_Operands.clear();
}

bool Instruction::HasOperands() const {
    return !m_Operands.empty();
}

size_t Instruction::GetOperandsNumber() const {
    return m_Operands.size();
}

std::vector<Value*> Instruction::GetOperands() const {
    return m_Operands;
}

}  // namespace ir
