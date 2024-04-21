#include <Ancl/AnclIR/Type/IntType.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

using namespace ir;


IntType::IntType(IRProgram& program, size_t bytesNumber)
    : Type(program), m_BytesNumber(bytesNumber) {}

IntType* IntType::Create(IRProgram& program, size_t bytesNumber) {
    return program.CreateType<IntType>(program, bytesNumber);
}

size_t IntType::GetBytesNumber() const {
    return m_BytesNumber;
}
