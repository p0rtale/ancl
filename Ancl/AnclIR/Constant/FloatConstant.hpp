#pragma once

#include <Ancl/AnclIR/Constant/Constant.hpp>
#include <Ancl/AnclIR/Type/FloatType.hpp>
#include <Ancl/Grammar/AST/Value/FloatValue.hpp>


namespace ir {

class FloatConstant: public Constant {
public:
    FloatConstant(FloatType* type, FloatValue value)
        : Constant(type), m_Value(value) {}

    FloatValue GetValue() const {
        return m_Value;
    }

private:
    FloatValue m_Value;
};

}  // namespace ir
