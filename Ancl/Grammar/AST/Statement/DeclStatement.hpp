#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>


namespace ast {

class DeclStatement: public Statement {
public:
    DeclStatement(Declaration* declaration)
        : m_Declaration(declaration) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Declaration* GetDeclaration() const {
        return m_Declaration;
    }

private:
    Declaration* m_Declaration;
};

}  // namespace ast
