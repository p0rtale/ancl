#pragma once

#include <Ancl/Base.hpp>

#include <Ancl/CodeGen/Target/Base/Machine.hpp>

#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.hpp>
#include <Ancl/CodeGen/Target/AMD64/AMD64ABI.hpp>


namespace target::amd64 {

class AMD64TargetMachine: public TargetMachine {
public:
    AMD64TargetMachine() {
        createRegisterSet();
        createABI();
    }

    uint GetPointerByteSize() override {
        return 8;
    }

private:
    void createRegisterSet() {
        m_RegisterSet = CreateScope<AMD64RegisterSet>();
    }

    void createABI() {
        m_ABI = CreateScope<AMD64TargetABI>(GetPointerByteSize());
    }
};

}  // namespace amd64
