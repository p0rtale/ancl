#pragma once

#include <array>
#include <Ancl/CodeGen/Target/Base/RegisterSet.hpp>
#include <Ancl/CodeGen/Target/AMD64/AMD64InstructionSet.hpp>


namespace target::amd64 {

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

    uint GetRegisterClass(uint bytes, bool isFloat = false) {
        if (isFloat) {
            return FR;
        }

        switch (bytes) {
        case 1:
            return GR8;
        case 2:
            return GR16;
        case 4:
            return GR32;
        case 8:
            return GR64;
        default:
            return INVALID_CLASS;
        }
    }

    uint GetRegisterClass(Register reg) {
        uint regNumber = reg.GetNumber();

        if (regNumber >= AL && regNumber <= R15B) {
            return GR8;
        }

        if (regNumber >= AX && regNumber <= R15W) {
            return GR16;
        }

        if (regNumber >= EAX && regNumber <= R15D) {
            return GR32;
        }
 
        if (regNumber >= RAX && regNumber <= RIP) {
            return GR64;
        }

        if (regNumber >= XMM0 && regNumber <= XMM15) {
            return FR;
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
