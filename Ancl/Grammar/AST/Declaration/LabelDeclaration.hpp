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

    void SetPreviousLabelDeclaration(LabelDeclaration* nextLabelDecl) {
        m_PrevLabelDeclaration = nextLabelDecl;
    }

    bool IsFirstLabelDeclaration() {
        return m_PrevLabelDeclaration;
    }

    LabelDeclaration* GetPrevLabelDeclaration() const {
        return m_PrevLabelDeclaration;
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

    LabelDeclaration* m_PrevLabelDeclaration;
};

}  // namespace ast
