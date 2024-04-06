#pragma once

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class ArrayType: public Type {
public:
    ArrayType(Type* subType, size_t size)
        : m_SubType(subType), m_Size(size) {}

    Type* GetSubType() const {
        return m_SubType;
    }

    size_t GetSize() const {
        return m_Size;
    }

private:
    Type* m_SubType;
    size_t m_Size = 0;
};

}  // namespace ir
