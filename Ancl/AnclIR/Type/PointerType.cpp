#include <Ancl/AnclIR/Type/PointerType.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

using namespace ir;

PointerType::PointerType(IRProgram& program, Type* subType)
    : Type(program), m_SubType(subType) {}

PointerType* PointerType::Create(Type* subType) {
    auto program = subType->GetProgram();
    return program.CreateType<PointerType>(program, subType);
}

Type* PointerType::GetSubType() {
    return m_SubType;
}
