#pragma once

class FloatValue {
public:
    FloatValue(float value)
        : m_Value(value), m_IsDoublePrecision(false) {}

    FloatValue(double value)
        : m_Value(value), m_IsDoublePrecision(true) {}

    double GetValue() const {
        return m_Value;
    }

    bool IsDoublePrecision() const {
        return m_IsDoublePrecision;
    }

private:
    double m_Value;
    bool m_IsDoublePrecision = false;
};
