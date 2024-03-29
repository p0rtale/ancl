#pragma once

#include <Ancl/Grammar/AST/Declaration/Declaration.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>


namespace ast {

class ValueDeclaration: public Declaration {
public:
    ValueDeclaration() = default;

    ValueDeclaration(std::string name, QualType* type = nullptr)
        : Declaration(std::move(name)), m_Type(type) {}

    virtual ~ValueDeclaration() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetType(QualType* type) {
        m_Type = type;
    }

    QualType* GetType() const {
        return m_Type;
    }

    bool IsLabelDecl() const override {
        return false;
    }

    bool IsTypeDecl() const override {
        return false;
    }

    bool IsValueDecl() const override {
        return true;
    }

private:
    QualType* m_Type;
};

}  // namespace ast
