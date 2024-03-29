#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class ConditionalExpression: public Expression {
public:
    ConditionalExpression(Expression* condition, Expression* trueExpression,
                          Expression* falseExpression)
        : m_Condition(condition), m_TrueExpression(trueExpression),
          m_FalseExpression(falseExpression) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Expression* GetCondition() const {
        return m_Condition;
    }

    Expression* GetTrueExpression() const {
        return m_TrueExpression;
    }

    Expression* GetFalseExpression() const {
        return m_FalseExpression;
    }

private:
    Expression* m_Condition;
    Expression* m_TrueExpression;
    Expression* m_FalseExpression;
};

}  // namespace ast
