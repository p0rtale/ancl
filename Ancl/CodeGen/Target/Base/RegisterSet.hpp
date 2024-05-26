#pragma once

#include <Ancl/CodeGen/Target/Base/Register.hpp>

namespace gen::target {

class RegisterSet {
public:
    RegisterSet() = default;
    virtual ~RegisterSet() = default;

    virtual bool IsValidRegister(uint number) = 0;
    virtual Register GetRegister(uint number) = 0;

    virtual std::vector<Register> GetRegisters(uint regClass) = 0;

    virtual std::vector<uint> GetGPClasses() const = 0;
    virtual std::vector<uint> GetFPClasses() const = 0;

    virtual bool HasZeroRegister() = 0;
    virtual Register GetZeroRegister() = 0;

    virtual bool HasLinkRegister() = 0;
    virtual Register GetLinkRegister() = 0;

    virtual Register GetARP() = 0;

    virtual Register GetSP() = 0;

    virtual Register GetIP() = 0;

    virtual uint GetRegisterClass(Register reg) = 0;
    virtual uint GetRegisterClass(uint bytes, bool isFloat = false) = 0;
};

}  // namespace target
