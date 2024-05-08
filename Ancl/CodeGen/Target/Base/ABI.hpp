#pragma once

#include <cstdlib>
#include <vector>

#include <Ancl/CodeGen/Target/Base/Register.hpp>

namespace gen::target {

class TargetABI {
public:
    TargetABI() = default;

    uint GetStackAlignment() const {
        return m_StackAlignment;
    }

    uint GetMaxStructParamSize() const {
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

    std::vector<Register> GetCalleeSavedRegisters() const {
        return m_CalleeSavedRegisters;
    } 

    std::vector<Register> GetCallerSavedRegisters() const {
        return m_CallerSavedRegisters;
    }

protected:
    uint m_StackAlignment;
    uint m_MaxStructParamSize;

    // TODO: std arrays
    std::vector<Register> m_IntArgumentRegisters;
    std::vector<Register> m_FloatArgumentRegisters;

    std::vector<Register> m_IntReturnRegisters;
    std::vector<Register> m_FloatReturnRegisters;

    std::vector<Register> m_CalleeSavedRegisters;
    std::vector<Register> m_CallerSavedRegisters;
};

}  // namespace target
