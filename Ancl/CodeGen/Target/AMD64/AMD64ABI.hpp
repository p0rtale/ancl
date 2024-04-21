#pragma once

#include <cstdlib>

#include <Ancl/CodeGen/Target/Base/ABI.hpp>


namespace amd64 {

class AMD64TargetABI: public target::TargetABI {
public:
    AMD64TargetABI(uint pointerSize) {
        setStackAlignment(2);
        setMaxStructParamSize(pointerSize * 2);
    }

private:
    void setStackAlignment(uint align) {
        m_StackAlignment = align;
    }

    void setMaxStructParamSize(uint size) {
        m_MaxStructParamSize = size;
    }
};

}  // namespace amd64
