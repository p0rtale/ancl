#pragma once

#include <cstdint>
#include <array>
#include <vector>

#include <Ancl/CodeGen/Target/Base/RegisterSet.hpp>


namespace gen::target::amd64 {

class AMD64RegisterSet: public RegisterSet {
public:
    enum RegNumber {
        INVALID_REGNUM = 0,

#define AMD64_REGISTER(NAME, NUMBER, BYTES, SUB_REGISTERS) NUMBER,
#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.inc>

        REG_NUMBER_END,
    };

    using TRegArray = std::array<Register, REG_NUMBER_END>;

public:
    AMD64RegisterSet();

    bool IsValidRegister(uint64_t number) override;

    Register GetRegister(uint64_t number) override;
    TRegArray GetAllRegisters() const;

    std::vector<Register> GetRegisters(uint64_t regClass) override;

    std::vector<uint64_t> GetGPClasses() const override;
    std::vector<uint64_t> GetFPClasses() const override;

    bool HasZeroRegister() override;
    Register GetZeroRegister() override;

    bool HasLinkRegister() override;
    Register GetLinkRegister() override;

    Register GetARP() override;
    Register GetSP() override;
    Register GetIP() override;

    unsigned int GetRegisterClass(uint64_t bytes, bool isFloat = false) override;
    unsigned int GetRegisterClass(Register reg) override;

private:
    void initRegisters();
    void updateRegistersInfo();

private:
    TRegArray m_Registers;
};

}  // namespace amd64
