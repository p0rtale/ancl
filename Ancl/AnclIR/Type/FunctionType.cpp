#include <Ancl/AnclIR/Type/FunctionType.hpp>

#include <Ancl/AnclIR/IRProgram.hpp>


namespace ir {

FunctionType::FunctionType(Type* retType, const std::vector<Type*>& paramTypes,
                           bool isVariadic)
    : Type(retType->GetProgram()), m_ReturnType(retType),
      m_ParamTypes(paramTypes), m_IsVariadic(isVariadic) {}

FunctionType* FunctionType::Create(Type* retType, const std::vector<Type*>& paramTypes,
                                   bool isVariadic) {
    IRProgram& program = retType->GetProgram();
    return program.CreateType<FunctionType>(retType, paramTypes, isVariadic);
}

Type* FunctionType::GetReturnType() const {
    return m_ReturnType;
}

std::vector<Type*> FunctionType::GetParamTypes() const {
    return m_ParamTypes;
}

bool FunctionType::HasParameters() const {
    return !m_ParamTypes.empty();
}

size_t FunctionType::GetParamNumber() const {
    return m_ParamTypes.size();
}

void FunctionType::SetVariadic() {
    m_IsVariadic = true;
}

bool FunctionType::IsVariadic() const {
    return m_IsVariadic;
}

}  // namespace ir
