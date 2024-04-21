#pragma once

#include <cstddef>

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class ArrayType: public Type {
public:
    ArrayType(Type* subType, size_t size);

    static ArrayType* Create(Type* subType, size_t size);

    Type* GetSubType() const;

    size_t GetSize() const;

private:
    Type* m_SubType;
    size_t m_Size = 0;
};

}  // namespace ir
