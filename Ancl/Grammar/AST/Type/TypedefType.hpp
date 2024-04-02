#pragma once

#include <Ancl/Grammar/AST/Type/Type.hpp>
#include <Ancl/Grammar/AST/Declaration/TypedefDeclaration.hpp>


namespace ast {

class TypedefType: public Type {
public:
    TypedefType(TypedefDeclaration* declaration)
        : m_Declaration(declaration) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetDeclaration(TypedefDeclaration* declaration) {
        m_Declaration = declaration;
    }

    TypedefDeclaration* GetDeclaration() const {
        return m_Declaration;
    }

private:
    TypedefDeclaration* m_Declaration;
};

}  // namespace ast
