#pragma once

#include <cstdint>

#include <Ancl/CodeGen/Target/Base/Register.hpp>


namespace gen::target {

class RegisterSet {
public:
    RegisterSet() = default;
    virtual ~RegisterSet() = default;

    virtual bool IsValidRegister(uint64_t number) = 0;
    virtual Register GetRegister(uint64_t number) = 0;

    virtual std::vector<Register> GetRegisters(uint64_t regClass) = 0;

    virtual std::vector<uint64_t> GetGPClasses() const = 0;
    virtual std::vector<uint64_t> GetFPClasses() const = 0;

    virtual bool HasZeroRegister() = 0;
    virtual Register GetZeroRegister() = 0;

    virtual bool HasLinkRegister() = 0;
    virtual Register GetLinkRegister() = 0;

    virtual Register GetARP() = 0;

    virtual Register GetSP() = 0;

    virtual Register GetIP() = 0;

    virtual unsigned int GetRegisterClass(Register reg) = 0;
    virtual unsigned int GetRegisterClass(uint64_t bytes, bool isFloat = false) = 0;
};

}  // namespace gen::target
