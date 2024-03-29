#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class SwitchStatement: public Statement {
public:
    SwitchStatement(Expression* expression, Statement* body)
        : m_Expression(expression),
          m_Body(body) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Expression* GetExpression() const {
        return m_Expression;
    }

    Statement* GetBody() const {
        return m_Body;
    }

private:
    Expression* m_Expression;
    Statement* m_Body;
};

}  // namespace ast
