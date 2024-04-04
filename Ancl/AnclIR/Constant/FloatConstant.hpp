#pragma once

#include <Ancl/AnclIR/Constant/Constant.hpp>
#include <Ancl/Grammar/AST/Value/FloatValue.hpp>

namespace ir {

class FloatConstant: public Constant {
public:
    FloatConstant(FloatValue value): m_Value(value) {}

    FloatValue GetValue() const {
        return m_Value;
    }

private:
    FloatValue m_Value;
};

}  // namespace ir
