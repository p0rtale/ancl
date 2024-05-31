#include <Ancl/AnclIR/Type/VoidType.hpp>

#include <Ancl/AnclIR/IRProgram.hpp>


namespace ir {

VoidType::VoidType(IRProgram& program): Type(program) {}

VoidType* VoidType::Create(IRProgram& program) {
    return program.CreateType<VoidType>(program);
}

}  // namespace ir
