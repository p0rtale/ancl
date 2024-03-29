#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class CastExpression: public Expression {
public:
    CastExpression(Expression* subExpression, QualType* toType)
        : m_SubExpression(subExpression), m_ToType(toType) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Expression* GetSubExpression() const {
        return m_SubExpression;
    }

    QualType* GetToType() const {
        return m_ToType;
    }

private:
    Expression* m_SubExpression;
    QualType* m_ToType;
};

}  // namespace ast
