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

    // TODO: Simplify...
    bool HasSize() const {
        return m_HasSetValue || m_SizeExpr && m_SizeExpr->IsEvaluated();
    }

    void SetSize(uint64_t size) {
        m_SetSizeValue = IntValue(size, /*isSigned=*/false);
        m_HasSetValue = true;
    }

    IntValue GetSize() const {
        if (m_HasSetValue) {
            return m_SetSizeValue;
        }
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

    IntValue m_SetSizeValue{0};
    bool m_HasSetValue = false;
};

}  // namespace ast
