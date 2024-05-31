#pragma once

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Constant/IntConstant.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class MemoryCopyInstruction: public Instruction {
public:
    MemoryCopyInstruction(Value* destination, Value* source, IntConstant* sizeConstant,
                          BasicBlock* basicBlock);

    Value* GetDestinationOperand() const;

    Value* GetSourceOperand() const;

    IntConstant* GetSizeConstant() const;

private:
    IntConstant* m_SizeConstant;
};

}  // namespace ir
