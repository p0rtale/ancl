#pragma once

#include <cstdint>

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class ArrayType: public Type {
public:
    ArrayType(Type* subType, uint64_t size);

    static ArrayType* Create(Type* subType, uint64_t size);

    Type* GetSubType() const;

    uint64_t GetSize() const;

private:
    Type* m_SubType;
    uint64_t m_Size = 0;
};

}  // namespace ir
