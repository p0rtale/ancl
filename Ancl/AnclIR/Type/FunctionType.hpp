#pragma once

#include <vector>

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class FunctionType: public Type {
public:
    FunctionType(Type* retType, const std::vector<Type*>& paramTypes,
                 bool isVariadic = false);

    static FunctionType* Create(Type* retType, const std::vector<Type*>& paramTypes,
                                bool isVariadic = false);

    Type* GetReturnType() const;

    std::vector<Type*> GetParamTypes() const;

    bool HasParameters() const {
        return !m_ParamTypes.empty();
    }

    std::size_t GetParamNumber() const;

    bool IsVariadic() const;

private:
    Type* m_ReturnType;
    std::vector<Type*> m_ParamTypes;

    bool m_IsVariadic = false;
};

}  // namespace ir
