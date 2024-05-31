#include <Ancl/Grammar/AST/Statement/Expression/ConditionalExpression.hpp>


namespace ast {

ConditionalExpression::ConditionalExpression(Expression* condition, Expression* trueExpression,
                        Expression* falseExpression)
    : m_Condition(condition), m_TrueExpression(trueExpression),
      m_FalseExpression(falseExpression) {}

void ConditionalExpression::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

void ConditionalExpression::SetCondition(Expression* condition) {
    m_Condition = condition;
}

Expression* ConditionalExpression::GetCondition() const {
    return m_Condition;
}

void ConditionalExpression::SetTrueExpression(Expression* trueExpr) {
    m_TrueExpression = trueExpr;
}

Expression* ConditionalExpression::GetTrueExpression() const {
    return m_TrueExpression;
}

void ConditionalExpression::SetFalseExpression(Expression* falseExpr) {
    m_FalseExpression = falseExpr;
}

Expression* ConditionalExpression::GetFalseExpression() const {
    return m_FalseExpression;
}

}  // namespace ast
