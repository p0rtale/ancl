#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Type/Type.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>
#include <Ancl/Grammar/AST/Type/NodeType.hpp>


namespace ast {

class FunctionType: public Type, public INodeType {
public:
    FunctionType(QualType returnType): m_ReturnType(returnType) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetVariadic() {
        m_IsVariadic = true;
    }

    bool IsVariadic() const {
        return m_IsVariadic;
    }

    void SetSubType(QualType subType) override {
        m_ReturnType = subType;
    }

    QualType GetSubType() const override {
        return m_ReturnType;
    }

    void SetParamTypes(const std::vector<QualType>& paramTypes) {
        m_ParamTypes = paramTypes;
    }

    std::vector<QualType> GetParamTypes() const {
        return m_ParamTypes;
    }

    bool IsArrayType() const override {
        return false;
    }

    bool IsFunctionType() const override {
        return true;
    }
    
    bool IsPointerType() const override {
        return false;
    }

private:
    QualType m_ReturnType;
    std::vector<QualType> m_ParamTypes;

    bool m_IsVariadic = false;
};

}  // namespace ast
