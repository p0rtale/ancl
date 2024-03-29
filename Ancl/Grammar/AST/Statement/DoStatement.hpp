#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class DoStatement: public Statement {
public:
    DoStatement(Expression* condition, Statement* body)
        : m_Condition(condition),
          m_Body(body) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Expression* GetCondition() const {
        return m_Condition;
    }

    Statement* GetBody() const {
        return m_Body;
    }

private:
    Expression* m_Condition;
    Statement* m_Body;
};

}  // namespace ast
