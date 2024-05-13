#pragma once

#include <string>
#include <vector>

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class PhiInstruction: public Instruction {
public:
    PhiInstruction(Type* type, const std::string& name, BasicBlock* basicBlock);

    size_t GetArgumentsNumber() const {
        return m_Arguments.size();
    }

    void SetIncomingValue(size_t index, Value* value) {
        m_Arguments.at(index).ArgValue = value;
    }

    Value* GetIncomingValue(size_t index) const {
        return m_Arguments.at(index).ArgValue;
    }    

    void SetIncomingBlock(size_t index, BasicBlock* block) {
        m_Arguments.at(index).Block = block;
    }

    BasicBlock* GetIncomingBlock(size_t index) const {
        return m_Arguments.at(index).Block;
    }   

private:
    struct PhiArg {
        BasicBlock* Block;
        Value* ArgValue;
    };

    std::vector<PhiArg> m_Arguments;
};

}  // namespace ir
