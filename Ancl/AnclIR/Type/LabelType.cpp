#include <Ancl/AnclIR/Type/LabelType.hpp>

#include <Ancl/AnclIR/IRProgram.hpp>


namespace ir {

LabelType::LabelType(IRProgram& program): Type(program) {}

LabelType* LabelType::Create(IRProgram& program) {
    return program.CreateType<LabelType>(program);
}

}  // namespace ir
