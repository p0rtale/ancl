#pragma once

#include <Ancl/Grammar/AST/Statement/SwitchCase.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/ConstExpression.hpp>


namespace ast {

class CaseStatement: public SwitchCase {
public:
    CaseStatement(ConstExpression* expression, Statement* body)
        : m_Expression(expression),
          m_Body(body) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    ConstExpression* GetExpression() const {
        return m_Expression;
    }

    Statement* GetBody() const {
        return m_Body;
    }

private:
    ConstExpression* m_Expression;
    Statement* m_Body;
};

}  // namespace ast
