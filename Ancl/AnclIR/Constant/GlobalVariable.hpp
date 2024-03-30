#pragma once

#include <Ancl/AnclIR/Constant/Constant.hpp>

namespace ir {

class GlobalVariable: public Constant {
public:
    GlobalVariable() = default;
};

}  // namespace ir
