#pragma once

#include <string>
#include <vector>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Constant/Function.hpp>
#include <Ancl/AnclIR/Type/FunctionType.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class CallInstruction: public Instruction {
public:
    // TODO: simplify Instruction init
    CallInstruction(Function* function, std::vector<Value*> arguments, const std::string& name,
                    BasicBlock* basicBlock)
            : Instruction(dynamic_cast<FunctionType*>(function->GetType())->GetReturnType(), basicBlock),
              m_Callee(function), m_Arguments(arguments) {
        SetName(name);

        AddOperand(function);
        for (Value* argument : arguments) {
            AddOperand(argument);
        }
    }

    Function* GetCallee() const {
        return m_Callee;
    }

    std::vector<Value*> GetArguments() const {
        return m_Arguments;
    }

    bool HasArguments() const {
        return !m_Arguments.empty();
    }

    size_t GetArgumentsNumber() const {
        return m_Arguments.size();
    }

private:
    Function* m_Callee;
    std::vector<Value*> m_Arguments;
};

}  // namespace ir
