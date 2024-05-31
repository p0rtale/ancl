#include <Ancl/AnclIR/Instruction/ReturnInstruction.hpp>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Constant/Function.hpp>
#include <Ancl/AnclIR/Type/VoidType.hpp>


namespace ir {

ReturnInstruction::ReturnInstruction(BasicBlock* basicBlock)
        : TerminatorInstruction(VoidType::Create(basicBlock->GetProgram()), basicBlock) {
    AddOperand(nullptr);
}

ReturnInstruction::ReturnInstruction(Value* returnValue, BasicBlock* basicBlock)
        : TerminatorInstruction(VoidType::Create(basicBlock->GetProgram()), basicBlock) {
    AddOperand(returnValue);
    basicBlock->GetFunction()->SetReturnValue(returnValue);
}

bool ReturnInstruction::HasReturnValue() const {
    return GetOperand(0);
}

Value* ReturnInstruction::GetReturnValue() const {
    return GetOperand(0);
}

}  // namespace ir
