#pragma once

#include <cstdint>

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class IRProgram;

class IntType: public Type {
public:
    IntType(IRProgram& program, uint64_t bytesNumber);

    static IntType* Create(IRProgram& program, uint64_t bytesNumber);

    uint64_t GetBytesNumber() const;

private:
    uint64_t m_BytesNumber = 0;
};

}  // namespace ir
