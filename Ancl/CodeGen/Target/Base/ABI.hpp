#pragma once

#include <cstdlib>
#include <vector>

#include <Ancl/CodeGen/Target/Base/Register.hpp>


namespace gen::target {

class TargetABI {
public:
    TargetABI() = default;

    uint64_t GetStackAlignment() const {
        return m_StackAlignment;
    }

    uint64_t GetRedZoneSize() const {
        return m_RedZoneSize;
    }

    uint64_t GetMaxStructParamSize() const {
        return m_MaxStructParamSize;
    }

    std::vector<Register> GetIntArgumentRegisters() const {
        return m_IntArgumentRegisters;
    }

    std::vector<Register> GetFloatArgumentRegisters() const {
        return m_FloatArgumentRegisters;
    }

    std::vector<Register> GetIntReturnRegisters() const {
        return m_IntReturnRegisters;
    }

    std::vector<Register> GetFloatReturnRegisters() const {
        return m_FloatReturnRegisters;
    }

    Register GetVectorNumberInfoRegister() const {
        return m_VectorNumberInfoRegister;
    }

    std::vector<Register> GetCalleeSavedRegisters() const {
        return m_CalleeSavedRegisters;
    } 

    std::vector<Register> GetCallerSavedRegisters() const {
        return m_CallerSavedRegisters;
    }

protected:
    uint64_t m_StackAlignment;
    uint64_t m_RedZoneSize;
    uint64_t m_MaxStructParamSize;

    // TODO: std arrays
    std::vector<Register> m_IntArgumentRegisters;
    std::vector<Register> m_FloatArgumentRegisters;

    std::vector<Register> m_IntReturnRegisters;
    std::vector<Register> m_FloatReturnRegisters;

    std::vector<Register> m_CalleeSavedRegisters;
    std::vector<Register> m_CallerSavedRegisters;

    Register m_VectorNumberInfoRegister;
};

}  // namespace gen::target
