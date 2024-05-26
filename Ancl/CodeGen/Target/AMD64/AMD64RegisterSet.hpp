#pragma once

#include <array>
#include <vector>

#include <Ancl/CodeGen/Target/Base/RegisterSet.hpp>
#include <Ancl/CodeGen/Target/AMD64/AMD64InstructionSet.hpp>


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
    AMD64RegisterSet() {
        initRegisters();
        updateRegistersInfo();
    }    

    bool IsValidRegister(uint number) override {
        return number > INVALID_REGNUM && number < REG_NUMBER_END;
    }

    Register GetRegister(uint number) override {
        return m_Registers.at(number);
    }

    TRegArray GetAllRegisters() const {
        return m_Registers;
    }

    std::vector<Register> GetRegisters(uint regClass) override {
        std::vector<Register> registers;
        switch (regClass) {
        case GR8:
            for (uint regNumber = AL; regNumber <= R15B; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            break;
        case GR16:
            for (uint regNumber = AX; regNumber <= R15W; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            break;
        case GR32:
            for (uint regNumber = EAX; regNumber <= R15D; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            break;
        case GR64:
            for (uint regNumber = RAX; regNumber <= RDI; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            // Skip RSP and RBP
            for (uint regNumber = R8; regNumber <= R15; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            // Skip RIP
            break;
        case FR:
        case FR32:
        case FR64:
            for (uint regNumber = XMM0; regNumber <= XMM15; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            break;
        default:
            // TODO: Handle error
            break;
        }

        return registers;
    }

    std::vector<uint> GetGPClasses() const override {
        return {GR8, GR16, GR32, GR64};
    }

    std::vector<uint> GetFPClasses() const override {
        return {FR};
    }

    bool HasZeroRegister() override {
        return false;
    }

    Register GetZeroRegister() override {
        return m_Registers[INVALID_REGNUM];
    }

    bool HasLinkRegister() override {
        return false;
    }

    Register GetLinkRegister() override {
        return m_Registers[INVALID_REGNUM];
    }

    Register GetARP() override {
        return m_Registers[RBP];
    }

    Register GetSP() override {
        return m_Registers[RSP];
    }

    Register GetIP() override {
        return m_Registers[RIP];
    }

    uint GetRegisterClass(uint bytes, bool isFloat = false) override {
        if (isFloat) {
            // switch (bytes) {
            // case 4:
            //     return FR32;
            // case 8:
            //     return FR64;
            // default:
            //     return INVALID_CLASS;
            // }
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

    uint GetRegisterClass(Register reg) override {
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
            // return FR64;
            return FR;
        }

        return INVALID_CLASS;
    }

private:
    void initRegisters() {
        m_Registers[INVALID_REGNUM] = Register(INVALID_REGNUM);

#define AMD64_REGISTER(NAME, NUMBER, BYTES, SUB_REGISTERS) \
        m_Registers[NUMBER] = Register(NAME, NUMBER, BYTES, SUB_REGISTERS);
#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.inc>
    }

    void updateRegistersInfo() {
        for (auto& reg : m_Registers) {
            if (GetRegisterClass(reg) == FR64) {
                reg.SetFloat();
            }

            std::vector<uint> subregNums = reg.GetSubRegNumbers();
            for (uint subregNum : subregNums) {
                m_Registers[subregNum].SetParentRegNumber(reg.GetNumber());
            }
            if (subregNums.size() == 2) {
                m_Registers[subregNums[0]].SetPairedRegNumber(subregNums[1]);
                m_Registers[subregNums[1]].SetPairedRegNumber(subregNums[0]);
            }
        }
    }

private:
    TRegArray m_Registers;
};

}  // namespace amd64
