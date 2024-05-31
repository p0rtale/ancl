#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.hpp>

#include <Ancl/CodeGen/Target/AMD64/AMD64InstructionSet.hpp>


namespace gen::target::amd64 {

AMD64RegisterSet::AMD64RegisterSet() {
    initRegisters();
    updateRegistersInfo();
}    

bool AMD64RegisterSet::IsValidRegister(uint64_t number) {
    return number > INVALID_REGNUM && number < REG_NUMBER_END;
}

Register AMD64RegisterSet::GetRegister(uint64_t number) {
    return m_Registers.at(number);
}

AMD64RegisterSet::TRegArray AMD64RegisterSet::GetAllRegisters() const {
    return m_Registers;
}

std::vector<Register> AMD64RegisterSet::GetRegisters(uint64_t regClass) {
    std::vector<Register> registers;
    switch (regClass) {
        case GR8:
            for (uint64_t regNumber = AL; regNumber <= R15B; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            break;
        case GR16:
            for (uint64_t regNumber = AX; regNumber <= R15W; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            break;
        case GR32:
            for (uint64_t regNumber = EAX; regNumber <= R15D; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            break;
        case GR64:
            for (uint64_t regNumber = RAX; regNumber <= RDI; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            // Skip RSP and RBP
            for (uint64_t regNumber = R8; regNumber <= R15; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            // Skip RIP
            break;
        case FR:
        case FR32:
        case FR64:
            for (uint64_t regNumber = XMM0; regNumber <= XMM15; ++regNumber) {
                registers.emplace_back(regNumber);
            }
            break;
        default:
            // TODO: Handle error
            break;
    }

    return registers;
}

std::vector<uint64_t> AMD64RegisterSet::GetGPClasses() const {
    return {GR8, GR16, GR32, GR64};
}

std::vector<uint64_t> AMD64RegisterSet::GetFPClasses() const {
    return {FR};
}

bool AMD64RegisterSet::HasZeroRegister() {
    return false;
}

Register AMD64RegisterSet::GetZeroRegister() {
    return m_Registers[INVALID_REGNUM];
}

bool AMD64RegisterSet::HasLinkRegister() {
    return false;
}

Register AMD64RegisterSet::GetLinkRegister() {
    return m_Registers[INVALID_REGNUM];
}

Register AMD64RegisterSet::GetARP() {
    return m_Registers[RBP];
}

Register AMD64RegisterSet::GetSP() {
    return m_Registers[RSP];
}

Register AMD64RegisterSet::GetIP() {
    return m_Registers[RIP];
}

unsigned int AMD64RegisterSet::GetRegisterClass(uint64_t bytes, bool isFloat) {
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

unsigned int AMD64RegisterSet::GetRegisterClass(Register reg) {
    uint64_t regNumber = reg.GetNumber();

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

void AMD64RegisterSet::initRegisters() {
    m_Registers[INVALID_REGNUM] = Register(INVALID_REGNUM);

#define AMD64_REGISTER(NAME, NUMBER, BYTES, SUB_REGISTERS) \
    m_Registers[NUMBER] = Register(NAME, NUMBER, BYTES, SUB_REGISTERS);
#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.inc>
}

void AMD64RegisterSet::updateRegistersInfo() {
    for (auto& reg : m_Registers) {
        if (GetRegisterClass(reg) == FR) {
            reg.SetFloat();
        }

        std::vector<uint64_t> subregNums = reg.GetSubRegNumbers();
        for (uint64_t subregNum : subregNums) {
            m_Registers[subregNum].SetParentRegNumber(reg.GetNumber());
        }
        if (subregNums.size() == 2) {
            m_Registers[subregNums[0]].SetPairedRegNumber(subregNums[1]);
            m_Registers[subregNums[1]].SetPairedRegNumber(subregNums[0]);
        }
    }
}

}  // namespace gen::target::amd64
