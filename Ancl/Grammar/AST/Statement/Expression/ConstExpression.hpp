#pragma once

#include <optional>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>
#include <Ancl/Grammar/AST/Value/Value.hpp>


namespace ast {

class ConstExpression: public Expression {
public:
    ConstExpression(Expression* expression);

    ConstExpression(Value value);

    void Accept(AstVisitor& visitor) override;

    Expression* GetExpression() const;

    void SetValue(Value value);
    Value GetValue() const;

    bool IsEvaluated() const;

private:
    Expression* m_Expression;
    std::optional<Value> m_Value;
};

}  // namespace ast
