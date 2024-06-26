#pragma once

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class VoidType: public Type {
public:
    VoidType(IRProgram& program);

    static VoidType* Create(IRProgram& program);
};

}  // namespace ir
