#pragma once

#include <string>

#include <Ancl/Grammar/AST/Base/ASTNode.hpp>


namespace ast {

class Declaration: public ASTNode {
public:
    Declaration() = default;

    Declaration(std::string name)
        : m_Name(std::move(name)) {}

    virtual ~Declaration() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetName(const std::string& name) {
        m_Name = name;
    }

int typedef *kek;

    std::string GetName() const {
        return m_Name;
    }

    virtual bool IsLabelDecl() const = 0;

    virtual bool IsTypeDecl() const = 0;

    virtual bool IsValueDecl() const = 0;

private:
    std::string m_Name;
};

}  // namespace ast
