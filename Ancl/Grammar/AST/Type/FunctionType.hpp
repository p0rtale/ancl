#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Type/Type.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>
#include <Ancl/Grammar/AST/Type/NodeType.hpp>


namespace ast {

class FunctionType: public Type, public INodeType {
public:
    FunctionType(QualType* returnType): m_ReturnType(returnType) {}

    void SetVariadic() {
        m_IsVariadic = true;
    }

    bool IsVariadic() const {
        return m_IsVariadic;
    }

    void SetSubType(QualType* subType) override {
        m_ReturnType = subType;
    }

    QualType* GetSubType() const override {
        return m_ReturnType;
    }

    void SetParamTypes(std::vector<QualType*> paramTypes) {
        m_ParamTypes = std::move(paramTypes);
    }

    std::vector<QualType*> GetParamTypes() const {
        return m_ParamTypes;
    }

private:
    QualType* m_ReturnType;
    std::vector<QualType*> m_ParamTypes;

    bool m_IsVariadic = false;
};

}  // namespace ast
