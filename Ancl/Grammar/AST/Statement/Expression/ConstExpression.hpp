#pragma once

#include <optional>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>
#include <Ancl/Grammar/AST/Value/Value.hpp>


namespace ast {

class ConstExpression: public Expression {
public:
    ConstExpression(Expression* expression)
        : m_Expression(expression) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetValue(Value value) {
        m_Value = value;
        m_IsEvaluated = true;
    }

    std::optional<Value> GetValue() const {
        if (!m_IsEvaluated) {
            return std::nullopt;
        }
        return m_Value;
    }

    bool IsEvaluated() const {
        return m_IsEvaluated;
    }

private:
    Expression* m_Expression;
    Value m_Value;
    bool m_IsEvaluated = false;
};

}  // namespace ast
