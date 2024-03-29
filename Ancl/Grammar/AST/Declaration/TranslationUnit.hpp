#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Declaration/Declaration.hpp>


namespace ast {

class TranslationUnit: public ASTNode {
public:
    TranslationUnit() = default;

    TranslationUnit(std::vector<Declaration*> declarations)
        : m_Declarations(std::move(declarations)) {} 

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void AddDeclaration(Declaration* declaration) {
        m_Declarations.push_back(declaration);
    }

    std::vector<Declaration*> GetDeclarations() const {
        return m_Declarations;
    }

private:
    std::vector<Declaration*> m_Declarations;
};

}  // namespace ast
