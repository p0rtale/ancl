#pragma once

#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>

namespace ir {

class BasicBlock;

class TerminatorInstruction: public Instruction {
public:
    TerminatorInstruction(Type* type, BasicBlock* basicBlock)
        : Instruction(type, basicBlock) {}
};

}  // namespace ir
