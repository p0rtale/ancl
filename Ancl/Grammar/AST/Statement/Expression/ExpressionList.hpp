#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class ExpressionList: public Expression {
public:
    ExpressionList() = default;

    ExpressionList(std::vector<Expression*> expressions)
        : m_Expressions(std::move(expressions)) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void AddExpression(Expression* expression) {
        m_Expressions.push_back(expression);
    }

    std::vector<Expression*> GetExpressions() const {
        return m_Expressions;
    }

private:
    std::vector<Expression*> m_Expressions;
};

}  // namespace ast
