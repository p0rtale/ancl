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
              m_Caller(function), m_Arguments(arguments) {
        SetName(name);
    }

    Function* GetCaller() const {
        return m_Caller;
    }

    std::vector<Value*> GetArguments() const {
        return m_Arguments;
    }

    size_t GetArgumentsNumber() const {
        return m_Arguments.size();
    }

private:
    Function* m_Caller;
    std::vector<Value*> m_Arguments;
};

}  // namespace ir
