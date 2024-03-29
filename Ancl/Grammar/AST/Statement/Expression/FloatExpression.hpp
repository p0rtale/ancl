#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>
#include <Ancl/Grammar/AST/Value/FloatValue.hpp>


namespace ast {

class FloatExpression: public Expression {
public:
    FloatExpression(FloatValue floatValue): m_FloatValue(floatValue) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    FloatValue GetFloatValue() const {
        return m_FloatValue;
    }

private:
    FloatValue m_FloatValue;
};

}  // namespace ast
