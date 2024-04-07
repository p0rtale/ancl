#pragma once

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class IntType: public Type {
public:
    IntType(size_t bytesNumber): m_BytesNumber(bytesNumber) {}

    size_t GetBytesNumber() const {
        return m_BytesNumber;
    }

private:
    size_t m_BytesNumber = 0;
};

}  // namespace ir
