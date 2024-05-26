#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/UnaryExpression.hpp>


namespace ast {

class SizeofTypeExpression: public Expression {
public:
    SizeofTypeExpression(QualType subType)
        : m_SubType(subType) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    QualType GetSubType() const {
        return m_SubType;
    }

private:
    QualType m_SubType;
};

}  // namespace ast
