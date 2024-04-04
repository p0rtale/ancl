#pragma once

#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>

namespace ir {

class Constant: public Value {
public:
    Constant(Type* type): Value(type) {}
};

}  // namespace ir
