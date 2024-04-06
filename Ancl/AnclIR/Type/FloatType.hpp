#pragma once

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class FloatType: public Type {
public:
    FloatType(bool isDouble = false): m_IsDouble(isDouble) {}

private:
    bool m_IsDouble = false;
};

}  // namespace ir
