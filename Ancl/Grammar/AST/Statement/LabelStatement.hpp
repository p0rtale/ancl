#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>
#include <Ancl/Grammar/AST/Statement/ValueStatement.hpp>


namespace ast {

class LabelStatement: public ValueStatement {
public:
    LabelStatement(LabelDeclaration* label, Statement* body)
        : m_Label(label), m_Body(body) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    LabelDeclaration* GetLabel() const {
        return m_Label;
    }

    Statement* GetBody() const {
        return m_Body;
    }

private:
    LabelDeclaration* m_Label;
    Statement* m_Body;
};

}  // namespace ast
