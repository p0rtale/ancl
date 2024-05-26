#include <Ancl/AnclIR/Type/PointerType.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

using namespace ir;


PointerType::PointerType(IRProgram& program, Type* subType)
    : Type(program), m_SubType(subType) {}

PointerType* PointerType::Create(Type* subType) {
    IRProgram& program = subType->GetProgram();
    return program.CreateType<PointerType>(program, subType);
}

Type* PointerType::GetSubType() const {
    return m_SubType;
}
