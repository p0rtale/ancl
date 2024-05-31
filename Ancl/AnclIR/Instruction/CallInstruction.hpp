#pragma once

#include <string>
#include <vector>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Constant/Function.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class CallInstruction: public Instruction {
public:
    CallInstruction(Function* function, std::vector<Value*> arguments, const std::string& name,
                    BasicBlock* basicBlock);

    Function* GetCallee() const;

    std::vector<Value*> GetArguments() const;
    bool HasArguments() const;

    size_t GetArgumentsNumber() const;

private:
    Function* m_Callee;
};

}  // namespace ir
