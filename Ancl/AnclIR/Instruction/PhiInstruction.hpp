#pragma once

#include <string>
#include <vector>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class PhiInstruction: public Instruction {
public:
    PhiInstruction(Type* type, const std::string& name, BasicBlock* basicBlock);

    size_t GetArgumentsNumber() const;

    void SetIncomingValue(size_t index, Value* value);
    Value* GetIncomingValue(size_t index) const;

    void SetIncomingBlock(size_t index, BasicBlock* block);
    BasicBlock* GetIncomingBlock(size_t index) const;

    void AddArgument(BasicBlock* block, Value* value);
    void DeleteArgument(size_t index);

private:
    std::vector<BasicBlock*> m_IncomingBlocks;
};

}  // namespace ir
