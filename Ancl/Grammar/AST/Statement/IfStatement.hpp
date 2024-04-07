#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class IfStatement: public Statement {
public:
    IfStatement(Expression* condition, Statement* thenStmt,
                Statement* elseStmt = nullptr)
        : m_Condition(condition), m_Then(thenStmt), m_Else(elseStmt) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Expression* GetCondition() const {
        return m_Condition;
    }
     
    Statement* GetThen() const {
        return m_Then;
    }

    Statement* GetElse() const {
        return m_Else;
    }

private:
    Expression* m_Condition;
    Statement* m_Then;
    Statement* m_Else;
};

}  // namespace ast
