#pragma once

#include <vector>

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class FunctionType: public Type {
public:
    FunctionType(Type* retType, const std::vector<Type*>& paramTypes,
                 bool isVariadic = false)
        : m_ReturnType(retType), m_ParamTypes(paramTypes),
          m_IsVariadic(isVariadic) {}

    Type* GetReturnType() const {
        return m_ReturnType;
    }

    std::vector<Type*> GetParamTypes() const {
        return m_ParamTypes;
    }

    size_t GetParamNumber() const {
        return m_ParamTypes.size();
    }

    bool IsVariadic() const {
        return m_IsVariadic;
    }

private:
    Type* m_ReturnType;
    std::vector<Type*> m_ParamTypes;

    bool m_IsVariadic = false;
};

}  // namespace ir
