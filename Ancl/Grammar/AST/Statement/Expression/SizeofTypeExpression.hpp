#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/UnaryExpression.hpp>


namespace ast {

class SizeofTypeExpression: public Expression {
public:
    SizeofTypeExpression(QualType* type)
        : m_Type(type) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    QualType* GetType() const {
        return m_Type;
    }

private:
    QualType* m_Type;
};

}  // namespace ast
