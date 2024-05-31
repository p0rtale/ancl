#include <Ancl/AnclIR/Instruction/AllocaInstruction.hpp>


namespace ir {

AllocaInstruction::AllocaInstruction(Type* type, const std::string& name,
                                     BasicBlock* basicBlock)
        : Instruction(PointerType::Create(type), basicBlock),
          m_AllocaType(type) {
    SetName(name);
}

Type* AllocaInstruction::GetAllocaType() const {
    return m_AllocaType;
}

}  // namespace ir
