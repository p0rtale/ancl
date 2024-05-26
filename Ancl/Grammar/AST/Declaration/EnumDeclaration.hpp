#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Declaration/TagDeclaration.hpp>


namespace ast {

class EnumDeclaration: public TagDeclaration {
public:
    EnumDeclaration(const std::vector<EnumConstDeclaration*>& enumerators,
                    bool isDefinition = false)
        : m_Enumerators(enumerators), m_IsDefinition(isDefinition) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    bool IsDefinition() const override {
        return m_IsDefinition;
    }

    std::vector<EnumConstDeclaration*> GetEnumerators() const {
        return m_Enumerators;
    }

    bool IsStruct() const override {
        return false;
    }

    bool IsUnion() const override {
        return false;
    }

    bool IsEnum() const override {
        return true;
    }

private:
    std::vector<EnumConstDeclaration*> m_Enumerators;

    bool m_IsDefinition = false;
};

}  // namespace ast
