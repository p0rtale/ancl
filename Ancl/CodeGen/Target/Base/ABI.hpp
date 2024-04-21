#pragma once

#include <cstdlib>

namespace target {

class TargetABI {
public:
    TargetABI() = default;

    uint GetStackAlignment() const {
        return m_StackAlignment;
    }

    uint GetMaxStructParamSize() const {
        return m_MaxStructParamSize;
    }

protected:
    uint m_StackAlignment;
    uint m_MaxStructParamSize;
};

}  // namespace target
