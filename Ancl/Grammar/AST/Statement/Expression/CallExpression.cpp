#include <Ancl/Grammar/AST/Statement/Expression/CallExpression.hpp>


namespace ast {

CallExpression::CallExpression(Expression* callee, std::vector<Expression*> arguments)
    : m_Callee(callee), m_Arguments(arguments) {}

void CallExpression::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

Expression* CallExpression::GetCallee() const {
    return m_Callee;
}

void CallExpression::SetArgument(size_t index, Expression* argExpr) {
    m_Arguments.at(index) = argExpr;
}

Expression* CallExpression::GetArgument(size_t index) {
    return m_Arguments.at(index);
}

std::vector<Expression*> CallExpression::GetArguments() const {
    return m_Arguments;
}

}  // namespace ast
