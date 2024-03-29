#pragma once

#include <Ancl/Grammar/AST/Declaration/TypeDeclaration.hpp>


namespace ast {

class TagDeclaration: public TypeDeclaration {
public:
    TagDeclaration() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    virtual bool IsStruct() const = 0;
    virtual bool IsUnion() const = 0;
    virtual bool IsEnum() const = 0;
};

}  // namespace ast
