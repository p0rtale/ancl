#pragma once

#include <cstddef>

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class IRProgram;

class IntType: public Type {
public:
    IntType(IRProgram& program, size_t bytesNumber);

    static IntType* Create(IRProgram& program, size_t bytesNumber);

    size_t GetBytesNumber() const;

private:
    size_t m_BytesNumber = 0;
};

}  // namespace ir
