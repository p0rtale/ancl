#include <Ancl/Grammar/AST/Statement/Expression/ConstExpression.hpp>


namespace ast {

ConstExpression::ConstExpression(Expression* expression)
    : m_Expression(expression) {}

ConstExpression::ConstExpression(Value value)
    : m_Value(value) {}

void ConstExpression::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

Expression* ConstExpression::GetExpression() const {
    return m_Expression;
}

void ConstExpression::SetValue(Value value) {
    m_Value = value;
}

Value ConstExpression::GetValue() const {
    return *m_Value;
}

bool ConstExpression::IsEvaluated() const {
    return m_Value.has_value();
}

}  // namespace ast
