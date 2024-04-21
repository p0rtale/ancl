#pragma once

#include <Ancl/CodeGen/Target/Base/Register.hpp>

namespace gen {

class RegisterSet {
public:
    RegisterSet() = default;
    virtual ~RegisterSet() = default;

    virtual Register& GetRegister(uint number) = 0;
    virtual Register& GetARP() = 0;

    // TODO: callersARP, return address, return value, ...
};

}  // namespace gen
