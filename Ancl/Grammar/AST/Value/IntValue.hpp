#pragma once

#include <cstdint>

class IntValue {
public:
    IntValue(uint64_t value, bool isSigned = true)
        : m_Value(value), m_IsSigned(isSigned) {}

    uint64_t GetUnsignedValue() const {
        return m_Value;
    }

    int64_t GetSignedValue() const {
        return static_cast<int64_t>(m_Value);
    }

    bool IsSigned() const {
        return m_IsSigned;
    }

private:
    uint64_t m_Value;
    bool m_IsSigned = true;
};


inline bool operator==(const IntValue& lhs, const IntValue& rhs) {
    return lhs.IsSigned() == rhs.IsSigned() &&
            lhs.GetSignedValue() == rhs.GetSignedValue();
}
