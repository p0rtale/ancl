#pragma once

#include <Ancl/Grammar/AST/Declaration/TypeDeclaration.hpp>


namespace ast {

class TypedefDeclaration: public TypeDeclaration {
public:
    TypedefDeclaration() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }
};

}  // namespace ast
