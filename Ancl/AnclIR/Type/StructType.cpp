#include <Ancl/AnclIR/Type/StructType.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

using namespace ir;

StructType::StructType(IRProgram& program, const std::vector<Type*>& elementTypes)
    : Type(program), m_ElementTypes(elementTypes) {}

StructType* StructType::Create(const std::vector<Type*>& elementTypes) {
    auto& program = elementTypes.at(0)->GetProgram();
    return program.CreateType<StructType>(program, elementTypes);
}

void StructType::SetName(const std::string& name) {
    m_Name = name;
}

std::string StructType::GetName() const {
    return m_Name;
}

std::vector<Type*> StructType::GetElementTypes() const {
    return m_ElementTypes;
}

size_t StructType::GetElementsNumber() const {
    return m_ElementTypes.size();
}
