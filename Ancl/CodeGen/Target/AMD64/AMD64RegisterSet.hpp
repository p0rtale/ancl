#pragma once

#include <array>
#include <Ancl/CodeGen/Target/Base/RegisterSet.hpp>

namespace target::amd64 {

class AMD64RegisterSet: public RegisterSet {
public:
    enum RegNumber {
        INVALID = 0,

#define AMD64_REGISTER(NAME, NUMBER, BYTES, SUB_REGISTERS) NUMBER,
#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.inc>

        REG_NUMBER_END,
    };

    enum class RegClass {
        GR, GR8, GR16, GR32, GR64,
        FR,
    };

    using TRegArray = std::array<Register, REG_NUMBER_END>;

public:
    AMD64RegisterSet() {
        initRegisters();
    }    

    Register GetRegister(uint number) override {
        return m_Registers.at(number);
    }

    TRegArray GetRegisters() const {
        return m_Registers;
    }

    bool HasZeroRegister() override {
        return false;
    }

    Register GetZeroRegister() override {
        return m_Registers[INVALID];
    }

    bool HasLinkRegister() override {
        return false;
    }

    Register GetLinkRegister() override {
        return m_Registers[INVALID];
    }

    Register GetARP() override {
        return m_Registers[RBP];
    }

    Register GetSP() override {
        return m_Registers[RSP];
    }

    RegClass GetRegisterClass(Register reg) {
        uint regNumber = reg.GetNumber();

        if (regNumber >= AL && regNumber <= R15B) {
            return RegClass::GR8;
        }

        if (regNumber >= AX && regNumber <= R15W) {
            return RegClass::GR16;
        }

        if (regNumber >= EAX && regNumber <= R15D) {
            return RegClass::GR32;
        }
 
        if (regNumber >= RAX && regNumber <= RIP) {
            return RegClass::GR64;
        }

        if (regNumber >= XMM0 && regNumber <= XMM15) {
            return RegClass::FR;
        }
    }

private:
    void initRegisters() {
        m_Registers[INVALID] = Register(INVALID);

#define AMD64_REGISTER(NAME, NUMBER, BYTES, SUB_REGISTERS) \
        m_Registers[NUMBER] = Register(NAME, NUMBER, BYTES, SUB_REGISTERS);
#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.inc>
    }

private:
    TRegArray m_Registers;
};

}  // namespace amd64
