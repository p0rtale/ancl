#pragma once

#include <Ancl/Grammar/AST/Declaration/Declaration.hpp>
#include <Ancl/Grammar/AST/Statement/Statement.hpp>


namespace ast {

class LabelDeclaration: public Declaration {
public:
    LabelDeclaration() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetStatement(Statement* statement) {
        m_Statement = statement;
    }

    Statement* GetStatement() const {
        return m_Statement;
    }

    bool IsLabelDecl() const override {
        return true;
    }

    bool IsTypeDecl() const override {
        return false;
    }

    bool IsValueDecl() const override {
        return false;
    }

private:
    Statement* m_Statement;
};

}  // namespace ast
