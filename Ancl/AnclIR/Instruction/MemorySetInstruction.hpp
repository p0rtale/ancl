#pragma once

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Constant/IntConstant.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Value.hpp>


namespace ir {

class MemorySetInstruction: public Instruction {
public:
    MemorySetInstruction(Value* destination, IntConstant* fillByte, IntConstant* bytesNumber,
                         BasicBlock* basicBlock);

    Value* GetDestinationOperand() const;

    IntConstant* GetFillByte() const;

    IntConstant* GetBytesNumber() const;

private:
    IntConstant* m_FillByte;
    IntConstant* m_BytesNumber;
};

}  // namespace ir
