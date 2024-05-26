#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

// TODO: give access to the variable declaration

class ForStatement: public Statement {
public:
    ForStatement(Statement* init, Expression* condition,
                 Expression* step, Statement* body)
        : m_Init(init), m_Condition(condition),
          m_Step(step), m_Body(body) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    bool HasInit() const {
        return m_Init;
    }

    Statement* GetInit() const {
        return m_Init;
    }

    bool HasCondition() const {
        return m_Condition;
    }

    Expression* GetCondition() const {
        return m_Condition;
    }

    bool HasStep() const {
        return m_Step;
    }

    Expression* GetStep() const {
        return m_Step;
    }

    Statement* GetBody() const {
        return m_Body;
    }

private:
    Statement* m_Init;
    Expression* m_Condition;
    Expression* m_Step;
    Statement* m_Body;
};

}  // namespace ast
