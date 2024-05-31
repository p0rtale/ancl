#include <Ancl/Grammar/AST/Statement/Expression/ExpressionList.hpp>

#include <cassert>


namespace ast {

ExpressionList::ExpressionList(const std::vector<Expression*>& expressions)
    : m_Expressions(expressions) {}

void ExpressionList::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

void ExpressionList::SetExpression(Expression* expression, size_t index) {
    m_Expressions.at(index) = expression;
}

void ExpressionList::AddExpression(Expression* expression) {
    m_Expressions.push_back(expression);
}

Expression* ExpressionList::GetLastExpression() {
    assert(!m_Expressions.empty());
    return m_Expressions.back();
}

std::vector<Expression*> ExpressionList::GetExpressions() const {
    return m_Expressions;
}

}  // namespace ast
