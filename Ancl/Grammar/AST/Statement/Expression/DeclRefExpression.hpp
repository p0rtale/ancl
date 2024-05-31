#pragma once

#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class DeclRefExpression: public Expression {
public:
    DeclRefExpression() = default;

    DeclRefExpression(ValueDeclaration* declaration);

    void Accept(AstVisitor& visitor) override;

    void SetDeclaration(ValueDeclaration* declaration);
    ValueDeclaration* GetDeclaration() const;

private:
    ValueDeclaration* m_Declaration;
};

}  // namespace ast
