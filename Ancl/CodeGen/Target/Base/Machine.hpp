#pragma once

#include <Ancl/Base.hpp>
#include <Ancl/CodeGen/Target/Base/ABI.hpp>


namespace target {

class TargetMachine {
public:
    TargetMachine() = default;
    virtual ~TargetMachine() = default;

    TargetABI* GetABI() {
        return m_ABI.get();
    }

    virtual uint GetPointerByteSize() = 0;

protected:
    TScopePtr<TargetABI> m_ABI = nullptr;
};

}  // namespace target
