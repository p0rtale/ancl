#include <Ancl/AnclIR/Type/IntType.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

using namespace ir;


IntType::IntType(IRProgram& program, uint64_t bytesNumber)
    : Type(program), m_BytesNumber(bytesNumber) {}

IntType* IntType::Create(IRProgram& program, uint64_t bytesNumber) {
    return program.CreateType<IntType>(program, bytesNumber);
}

uint64_t IntType::GetBytesNumber() const {
    return m_BytesNumber;
}
