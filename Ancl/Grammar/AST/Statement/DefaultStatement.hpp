#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>
#include <Ancl/Grammar/AST/Statement/SwitchCase.hpp>


namespace ast {

class DefaultStatement: public SwitchCase {
public:
    DefaultStatement(Statement* body)
        : m_Body(body) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Statement* GetBody() const {
        return m_Body;
    }

private:
    Statement* m_Body;
};

}  // namespace ast
