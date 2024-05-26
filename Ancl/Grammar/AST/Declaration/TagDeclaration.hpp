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

    virtual bool IsDefinition() const = 0;

    // TODO: Interface?
    void SetPreviousDeclaration(TagDeclaration* declaration) {
        m_PreviousDeclaration = declaration;
    }

    bool IsFirstDeclaration() const {
        return m_PreviousDeclaration;
    }

    TagDeclaration* GetPreviousDeclaration() {
        return m_PreviousDeclaration;
    }

private:
    TagDeclaration* m_PreviousDeclaration = nullptr;
};

}  // namespace ast
