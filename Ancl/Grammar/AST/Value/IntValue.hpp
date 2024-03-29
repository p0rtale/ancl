#pragma once

#include <cstdint>

#include <Ancl/Grammar/AST/Value/Value.hpp>


namespace ast {

class IntValue: public Value {
public:
    IntValue(uint64_t value, bool isSigned = true)
        : m_Value(value), m_IsSigned(isSigned) {}

    uint64_t GetValue() const {
        return m_Value;
    }

    bool IsSigned() const {
        return m_IsSigned;
    }

private:
    uint64_t m_Value;
    bool m_IsSigned = true;
};

}  // namespace ast
