#include <Ancl/AnclIR/Instruction/MemoryCopyInstruction.hpp>

#include <Ancl/AnclIR/Type/VoidType.hpp>


namespace ir {

MemoryCopyInstruction::MemoryCopyInstruction(Value* destination, Value* source,
                                             IntConstant* sizeConstant, BasicBlock* basicBlock)
        : Instruction(VoidType::Create(destination->GetProgram()), basicBlock),
          m_SizeConstant(sizeConstant) {
    AddOperand(destination);
    AddOperand(source);
}

Value* MemoryCopyInstruction::GetDestinationOperand() const {
    return GetOperand(0);
}

Value* MemoryCopyInstruction::GetSourceOperand() const {
    return GetOperand(1);
}

IntConstant* MemoryCopyInstruction::GetSizeConstant() const {
    return m_SizeConstant;
}

}  // namespace ir
