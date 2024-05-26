#pragma once

#include <cassert>

#include <Ancl/Grammar/AST/Type/Type.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>
#include <Ancl/Grammar/AST/Type/NodeType.hpp>
#include <Ancl/Grammar/AST/Value/IntValue.hpp>

#include <Ancl/Grammar/AST/Statement/Expression/ConstExpression.hpp>


namespace ast {

class ArrayType: public Type, public INodeType {
public:
    ArrayType(QualType subType, ConstExpression* sizeExpr)
        : m_SubType(subType), m_SizeExpr(sizeExpr) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetSubType(QualType subType) override {
        m_SubType = subType;
    }

    QualType GetSubType() const override {
        return m_SubType;
    }

    bool HasSize() const {
        return m_SizeExpr && m_SizeExpr->IsEvaluated();
    }

    IntValue GetSize() const {
        if (HasSize()) {
            Value value = m_SizeExpr->GetValue();
            assert(value.IsInteger());
            return value.GetIntValue();
        }
        return IntValue(0);
    }

    ConstExpression* GetSizeExpression() const {
        return m_SizeExpr;
    }

    bool IsArrayType() const override {
        return true;
    }

    bool IsFunctionType() const override {
        return false;
    }
    
    bool IsPointerType() const override {
        return false;
    }

private:
    QualType m_SubType;
    ConstExpression* m_SizeExpr;
};

}  // namespace ast
