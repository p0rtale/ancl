#pragma once

#include <Ancl/Grammar/AST/Type/TagType.hpp>
#include <Ancl/Grammar/AST/Declaration/EnumDeclaration.hpp>


namespace ast {

class EnumType: public TagType {
public:
    EnumType(EnumDeclaration* declaration)
        : m_Declaration(declaration) {}

    EnumDeclaration* GetDeclaration() const {
        return m_Declaration;
    }

    bool IsRecord() const override {
        return false;
    }

    bool IsEnum() const override {
        return true;
    }

private:
    EnumDeclaration* m_Declaration;
};

}  // namespace ast
