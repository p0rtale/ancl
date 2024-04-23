#pragma once

#include <Ancl/Base.hpp>
#include <Ancl/CodeGen/Target/Base/ABI.hpp>
#include <Ancl/CodeGen/Target/Base/RegisterSet.hpp>


namespace target {

class TargetMachine {
public:
    TargetMachine() = default;
    virtual ~TargetMachine() = default;

    RegisterSet* GetRegisterSet() {
        return m_RegisterSet.get();
    }

    TargetABI* GetABI() {
        return m_ABI.get();
    }

    virtual uint GetPointerByteSize() = 0;

protected:
    TScopePtr<RegisterSet> m_RegisterSet = nullptr;
    TScopePtr<TargetABI> m_ABI = nullptr;
};

}  // namespace target
