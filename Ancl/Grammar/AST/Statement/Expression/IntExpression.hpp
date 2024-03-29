#pragma once

#include <cstdint>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>
#include <Ancl/Grammar/AST/Value/IntValue.hpp>


namespace ast {

class IntExpression: public Expression {
public:
    IntExpression(IntValue intValue): m_IntValue(intValue) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    IntValue GetIntValue() const {
        return m_IntValue;
    }

private:
    IntValue m_IntValue;
};

}  // namespace ast
