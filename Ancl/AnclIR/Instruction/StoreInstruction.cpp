#include <Ancl/AnclIR/Instruction/StoreInstruction.hpp>

#include <Ancl/AnclIR/Type/VoidType.hpp>


namespace ir {

StoreInstruction::StoreInstruction(Value* value, Value* address,
                                   const std::string& name, BasicBlock* basicBlock)
        : Instruction(VoidType::Create(value->GetProgram()), basicBlock) {
    SetName(name);
    AddOperand(value);
    AddOperand(address);
}

Value* StoreInstruction::GetValueOperand() const {
    return GetOperand(0);
}

Value* StoreInstruction::GetAddressOperand() const {
    return GetOperand(1);
}

void StoreInstruction::SetVolatile() {
    m_IsVolatile = true;
}

bool StoreInstruction::IsVolatile() const {
    return m_IsVolatile;
}

}  // namespace ir
