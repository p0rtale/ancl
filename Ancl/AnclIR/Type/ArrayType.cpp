#include <Ancl/AnclIR/Type/ArrayType.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

using namespace ir;

ArrayType::ArrayType(Type* subType, size_t size)
    : Type(subType->GetProgram()),
      m_SubType(subType), m_Size(size) {}

ArrayType* ArrayType::Create(Type* subType, size_t size) {
    auto program = subType->GetProgram();
    return program.CreateType<ArrayType>(subType, size);
}

Type* ArrayType::GetSubType() const {
    return m_SubType;
}

size_t ArrayType::GetSize() const {
    return m_Size;
}
