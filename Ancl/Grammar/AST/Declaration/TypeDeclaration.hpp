#pragma once

#include <Ancl/Grammar/AST/Declaration/Declaration.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>


namespace ast {

class TypeDeclaration: public Declaration {
public:
    TypeDeclaration() = default;

    TypeDeclaration(QualType type)
        : m_Type(std::move(type)) {}

    virtual ~TypeDeclaration() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetType(QualType type) {
        m_Type = type;
    }

    QualType GetType() const {
        return m_Type;
    }

    bool IsLabelDecl() const override {
        return false;
    }

    bool IsTypeDecl() const override {
        return true;
    }

    bool IsValueDecl() const override {
        return false;
    }

private:
    QualType m_Type;
};

}  // namespace ast
