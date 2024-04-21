#pragma once

#include <Ancl/Base.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>
#include <Ancl/CodeGen/Target/AMD64/AMD64ABI.hpp>


namespace amd64 {

class AMD64TargetMachine: public target::TargetMachine {
public:
    AMD64TargetMachine() {
        createABI();
    }

    uint GetPointerByteSize() override {
        return 8;
    }

private:
    void createABI() {
        m_ABI = CreateScope<AMD64TargetABI>(GetPointerByteSize());
    }
};

}  // namespace amd64
