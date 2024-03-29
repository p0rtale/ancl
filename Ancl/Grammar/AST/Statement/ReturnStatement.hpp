#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class ReturnStatement: public Statement {
public:
    ReturnStatement(Expression* expression = nullptr)
        : m_ReturnExpression(expression) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Expression* GetReturnExpression() const {
        return m_ReturnExpression;
    }

private:
    Expression* m_ReturnExpression;
};

}  // namespace ast
