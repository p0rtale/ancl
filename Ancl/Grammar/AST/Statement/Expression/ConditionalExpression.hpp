#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class ConditionalExpression: public Expression {
public:
    ConditionalExpression(Expression* condition, Expression* trueExpression,
                          Expression* falseExpression);

    void Accept(AstVisitor& visitor) override;

    void SetCondition(Expression* condition);
    Expression* GetCondition() const;

    void SetTrueExpression(Expression* trueExpr);
    Expression* GetTrueExpression() const;

    void SetFalseExpression(Expression* falseExpr);
    Expression* GetFalseExpression() const;

private:
    Expression* m_Condition;
    Expression* m_TrueExpression;
    Expression* m_FalseExpression;
};

}  // namespace ast
