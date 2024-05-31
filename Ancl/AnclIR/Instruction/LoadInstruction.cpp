#include <Ancl/AnclIR/Instruction/LoadInstruction.hpp>


namespace ir {

LoadInstruction::LoadInstruction(Value* pointer, Type* type, const std::string& name,
                                 BasicBlock* basicBlock)
        : Instruction(type, basicBlock) {
    SetName(name);
    AddOperand(pointer);
}

Value* LoadInstruction::GetPtrOperand() const {
    return GetOperand(0);
}

void LoadInstruction::SetVolatile() {
    m_IsVolatile = true;
}

bool LoadInstruction::IsVolatile() const {
    return m_IsVolatile;
}

}  // namespace ir
