#include <Ancl/Grammar/AST/Statement/Expression/DeclRefExpression.hpp>


namespace ast {

DeclRefExpression::DeclRefExpression(ValueDeclaration* declaration)
    : m_Declaration(declaration) {}

void DeclRefExpression::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

void DeclRefExpression::SetDeclaration(ValueDeclaration* declaration) {
    m_Declaration = declaration;
}

ValueDeclaration* DeclRefExpression::GetDeclaration() const {
    return m_Declaration;
}

}  // namespace ast
