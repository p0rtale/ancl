#include <Ancl/AnclIR/Instruction/CallInstruction.hpp>


namespace ir {

// TODO: simplify Instruction init
CallInstruction::CallInstruction(Function* function, std::vector<Value*> arguments,
                                 const std::string& name, BasicBlock* basicBlock)
        : Instruction(dynamic_cast<FunctionType*>(function->GetType())->GetReturnType(), basicBlock),
          m_Callee(function) {
    SetName(name);

    for (Value* argument : arguments) {
        AddOperand(argument);
    }
}

Function* CallInstruction::GetCallee() const {
    return m_Callee;
}

std::vector<Value*> CallInstruction::GetArguments() const {
    return GetOperands();
}

bool CallInstruction::HasArguments() const {
    return HasOperands();
}

size_t CallInstruction::GetArgumentsNumber() const {
    return GetOperandsNumber();
}

}  // namespace ir
