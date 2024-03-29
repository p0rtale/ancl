#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>


namespace ast {

class ValueStatement: public Statement {
public:
    virtual ~ValueStatement() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }
};

}  // namespace ast
