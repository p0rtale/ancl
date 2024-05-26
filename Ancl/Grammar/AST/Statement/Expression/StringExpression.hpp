#pragma once

#include <string>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class StringExpression: public Expression {
public:
    StringExpression(const std::string& stringValue)
        : m_StringValue(stringValue) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    std::string GetStringValue() const {
        return m_StringValue;
    }

private:
    std::string m_StringValue;
};

}  // namespace ast
