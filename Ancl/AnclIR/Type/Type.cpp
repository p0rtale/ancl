#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

Type::Type(IRProgram& program): m_Program(program) {}

IRProgram& Type::GetProgram() const {
    return m_Program;
}

}  // namespace ir
