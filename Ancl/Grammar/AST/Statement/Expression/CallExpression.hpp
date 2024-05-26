#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class CallExpression: public Expression {
public:
    CallExpression(Expression* callee, std::vector<Expression*> arguments)
        : m_Callee(callee), m_Arguments(arguments) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Expression* GetCallee() const {
        return m_Callee;
    }

    void SetArgument(size_t index, Expression* argExpr) {
        m_Arguments.at(index) = argExpr;
    }

    Expression* GetArgument(size_t index) {
        return m_Arguments.at(index);
    }

    std::vector<Expression*> GetArguments() const {
        return m_Arguments;
    }

private:
    Expression* m_Callee;
    std::vector<Expression*> m_Arguments;
};

}  // namespace ast
