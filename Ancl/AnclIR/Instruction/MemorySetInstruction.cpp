#include <Ancl/AnclIR/Instruction/MemorySetInstruction.hpp>

#include <Ancl/AnclIR/Type/VoidType.hpp>


namespace ir {

MemorySetInstruction::MemorySetInstruction(Value* destination, IntConstant* fillByte,
                                           IntConstant* bytesNumber, BasicBlock* basicBlock)
        : Instruction(VoidType::Create(destination->GetProgram()), basicBlock),
          m_FillByte(fillByte),
          m_BytesNumber(bytesNumber) {
    AddOperand(destination);
}

Value* MemorySetInstruction::GetDestinationOperand() const {
    return GetOperand(0);
}

IntConstant* MemorySetInstruction::GetFillByte() const {
    return m_FillByte;
}

IntConstant* MemorySetInstruction::GetBytesNumber() const {
    return m_BytesNumber;
}

}  // namespace ir
