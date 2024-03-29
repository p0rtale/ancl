#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>
#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>


namespace ast {

class DeclRefExpression: public Expression {
public:
    DeclRefExpression() = default;

    DeclRefExpression(ValueDeclaration* declaration)
        : m_Declaration(declaration) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetDeclaration(ValueDeclaration* declaration) {
        m_Declaration = declaration;
    }

    ValueDeclaration* GetDeclaration() const {
        return m_Declaration;
    }

private:
    ValueDeclaration* m_Declaration;
};

}  // namespace ast
