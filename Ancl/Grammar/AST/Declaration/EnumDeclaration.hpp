#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Declaration/TagDeclaration.hpp>


namespace ast {

class EnumDeclaration: public TagDeclaration {
public:
    EnumDeclaration(std::vector<EnumConstDeclaration*> enumerators)
        : m_Enumerators(std::move(enumerators)) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
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
};

}  // namespace ast
