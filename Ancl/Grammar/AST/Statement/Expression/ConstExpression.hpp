#pragma once

#include <optional>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>
#include <Ancl/Grammar/AST/Value/Value.hpp>


namespace ast {

class ConstExpression: public Expression {
public:
    ConstExpression(Expression* expression)
        : m_Expression(expression) {}

    ConstExpression(Value value)
        : m_Value(value) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Expression* GetExpression() const {
        return m_Expression;
    }

    void SetValue(Value value) {
        m_Value = value;
    }

    Value GetValue() const {
        return *m_Value;
    }

    bool IsEvaluated() const {
        return m_Value.has_value();
    }

private:
    Expression* m_Expression;
    std::optional<Value> m_Value;
};

}  // namespace ast
