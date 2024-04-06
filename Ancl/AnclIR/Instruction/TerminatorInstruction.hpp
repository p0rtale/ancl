#pragma once

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class TerminatorInstruction: public Instruction {
public:
    TerminatorInstruction(Type* type, BasicBlock* basicBlock)
        : Instruction(type, basicBlock) {}
};

}  // namespace ir
