#include <Ancl/AnclIR/Instruction/PhiInstruction.hpp>

#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

PhiInstruction::PhiInstruction(Type* type, const std::string& name, BasicBlock* basicBlock)
        : Instruction(type, basicBlock),
          m_IncomingBlocks(basicBlock->GetPredecessorsNumber()) {
    SetName(name);
    // TODO: Block uses?

    for (size_t i = 0; i < basicBlock->GetPredecessorsNumber(); ++i) {
        AddOperand(nullptr);
    }
}

size_t PhiInstruction::GetArgumentsNumber() const {
    return GetOperandsNumber();
}

void PhiInstruction::SetIncomingValue(size_t index, Value* value) {
    SetOperand(value, index);
}

Value* PhiInstruction::GetIncomingValue(size_t index) const {
    return GetOperand(index);
}    

void PhiInstruction::SetIncomingBlock(size_t index, BasicBlock* block) {
    m_IncomingBlocks.at(index) = block;
}

BasicBlock* PhiInstruction::GetIncomingBlock(size_t index) const {
    return m_IncomingBlocks.at(index);
}   

void PhiInstruction::AddArgument(BasicBlock* block, Value* value) {
    AddOperand(value);
    m_IncomingBlocks.push_back(block);
}

void PhiInstruction::DeleteArgument(size_t index) {
    DeleteOperand(index);
    m_IncomingBlocks.erase(m_IncomingBlocks.begin() + index);
}

}  // namespace ir
