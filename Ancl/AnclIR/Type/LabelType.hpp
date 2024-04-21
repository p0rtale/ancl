#pragma once

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class IRProgram;

class LabelType: public Type {
public:
    LabelType(IRProgram& program);

    static LabelType* Create(IRProgram& program);
};

}  // namespace ir
