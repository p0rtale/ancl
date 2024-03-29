#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>


namespace ast {

class SwitchCase: public Statement {
public:
    virtual ~SwitchCase() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }
};

}  // namespace ast
