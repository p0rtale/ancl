#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>


namespace ast {

class DeclStatement: public Statement {
public:
    DeclStatement(const std::vector<Declaration*>& decls)
        : m_DeclGroup(decls) {}

    DeclStatement(Declaration* declaration)
        : m_DeclGroup{declaration} {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    Declaration* GetFirstDeclaration() const {
        return m_DeclGroup.at(0);
    }

    std::vector<Declaration*> GetDeclarations() const {
        return m_DeclGroup;
    }

    size_t GetDeclGroupSize() const {
        return m_DeclGroup.size();
    }

private:
    std::vector<Declaration*> m_DeclGroup;
};

}  // namespace ast
