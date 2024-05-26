#pragma once

#include <cassert>
#include <vector>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class ExpressionList: public Expression {
public:
    ExpressionList() = default;

    ExpressionList(const std::vector<Expression*>& expressions)
        : m_Expressions(expressions) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetExpression(Expression* expression, size_t index) {
        m_Expressions.at(index) = expression;
    }

    void AddExpression(Expression* expression) {
        m_Expressions.push_back(expression);
    }

    Expression* GetLastExpression() {
        assert(!m_Expressions.empty());
        return m_Expressions.back();
    }

    std::vector<Expression*> GetExpressions() const {
        return m_Expressions;
    }

private:
    std::vector<Expression*> m_Expressions;
};

}  // namespace ast
