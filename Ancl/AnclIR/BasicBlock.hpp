#pragma once

#include <Ancl/AnclIR/Value.hpp>

namespace ir {

// Represents block label
class BasicBlock: public Value {
public:
    BasicBlock() = default;
};

}  // namespace ir
