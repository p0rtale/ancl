#pragma once

#include <variant>

#include <Ancl/Grammar/AST/Value/IntValue.hpp>
#include <Ancl/Grammar/AST/Value/FloatValue.hpp>


class Value {
public:
    Value(int64_t value, bool isSigned)
        : m_Value(IntValue(value, isSigned)) {}

    explicit Value(float value): m_Value(FloatValue(value)) {}
    explicit Value(double value): m_Value(FloatValue(value)) {}

    Value(const IntValue& intValue): m_Value(intValue) {}
    Value(const FloatValue& floatValue): m_Value(floatValue) {}

    bool IsInteger() const {
        return std::holds_alternative<IntValue>(m_Value);
    }

    bool IsFloat() const {
        return std::holds_alternative<FloatValue>(m_Value);
    }

    IntValue GetIntValue() const {
        return std::get<IntValue>(m_Value);
    }

    FloatValue GetFloatValue() const {
        return std::get<FloatValue>(m_Value);
    }

private:
    std::variant<IntValue, FloatValue> m_Value;
};
