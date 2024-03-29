#pragma once

#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>


namespace ast {

class FieldDeclaration: public ValueDeclaration {
public:
    FieldDeclaration() = default;

    FieldDeclaration(std::string name, QualType* type = nullptr)
        : ValueDeclaration(std::move(name), type) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }
};

}  // namespace ast
