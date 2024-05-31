#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class ExpressionList: public Expression {
public:
    ExpressionList() = default;

    ExpressionList(const std::vector<Expression*>& expressions);

    void Accept(AstVisitor& visitor) override;

    void SetExpression(Expression* expression, size_t index);
    void AddExpression(Expression* expression);

    Expression* GetLastExpression();

    std::vector<Expression*> GetExpressions() const;

private:
    std::vector<Expression*> m_Expressions;
};

}  // namespace ast
