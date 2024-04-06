#pragma once

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class IntType: public Type {
public:
    IntType(size_t bitsNumber): m_BitsNumber(bitsNumber) {}

    size_t GetBitsNumber() const {
        return m_BitsNumber;
    }

private:
    size_t m_BitsNumber = 0;
};

}  // namespace ir
