#pragma once

#include <Ancl/AnclIR/Constant/Constant.hpp>
#include <Ancl/AnclIR/Type/IntType.hpp>
#include <Ancl/Grammar/AST/Value/IntValue.hpp>


namespace ir {

class IntConstant: public Constant {
public:
    IntConstant(IntType* type, IntValue value)
        : Constant(type), m_Value(value) {}

    IntValue GetValue() const {
        return m_Value;
    }

private:
    IntValue m_Value;
};

}  // namespace ir
