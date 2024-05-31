#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class CallExpression: public Expression {
public:
    CallExpression(Expression* callee, std::vector<Expression*> arguments);

    void Accept(AstVisitor& visitor) override;

    Expression* GetCallee() const;

    void SetArgument(size_t index, Expression* argExpr);
    Expression* GetArgument(size_t index);

    std::vector<Expression*> GetArguments() const;

private:
    Expression* m_Callee;
    std::vector<Expression*> m_Arguments;
};

}  // namespace ast
