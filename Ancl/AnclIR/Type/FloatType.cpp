#include <Ancl/AnclIR/Type/FloatType.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

using namespace ir;


FloatType::FloatType(IRProgram& program, Kind kind)
    : Type(program), m_Kind(kind) {}

FloatType* FloatType::Create(IRProgram& program, Kind kind) {
    return program.CreateType<FloatType>(program, kind);
}

FloatType::Kind FloatType::GetKind() const {
    return m_Kind;
}
