#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>
#include <Ancl/Grammar/AST/Declaration/LabelDeclaration.hpp>


namespace ast {

class GotoStatement: public Statement {
public:
    GotoStatement(LabelDeclaration* label): m_Label(label) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetLabel(LabelDeclaration* label) {
        m_Label = label;
    }

    LabelDeclaration* GetLabel() const {
        return m_Label;
    }

private:
    LabelDeclaration* m_Label;
};

}  // namespace ast
