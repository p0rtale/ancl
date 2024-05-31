#include <Ancl/AnclIR/Type/ArrayType.hpp>

#include <Ancl/AnclIR/IRProgram.hpp>


namespace ir {

ArrayType::ArrayType(Type* subType, uint64_t size)
    : Type(subType->GetProgram()),
      m_SubType(subType), m_Size(size) {}

ArrayType* ArrayType::Create(Type* subType, uint64_t size) {
    IRProgram& program = subType->GetProgram();
    return program.CreateType<ArrayType>(subType, size);
}

Type* ArrayType::GetSubType() const {
    return m_SubType;
}

uint64_t ArrayType::GetSize() const {
    return m_Size;
}

}  // namespace ir
