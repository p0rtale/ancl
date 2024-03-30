#pragma once

#include <Ancl/AnclIR/Value.hpp>

namespace ir {

/*
    %add = add %0 %1

    %add - instruction
    %0 and %1 - instruction values
*/
class Instruction: public Value {
public:
    Instruction() = default;
};

}  // namespace ir
