#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class CharExpression: public Expression {
public:
    CharExpression(char charValue): m_CharValue(charValue) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    char GetCharValue() const {
        return m_CharValue;
    }

private:
    char m_CharValue;
};

}  // namespace ast
