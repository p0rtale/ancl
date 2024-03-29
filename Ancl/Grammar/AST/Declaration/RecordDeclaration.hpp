#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Declaration/TagDeclaration.hpp>


namespace ast {

class RecordDeclaration: public TagDeclaration {
public:
    RecordDeclaration(std::vector<FieldDeclaration*> fields,
                      bool isUnion = false)
        : m_Fields(std::move(fields)), m_IsUnion(isUnion) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    std::vector<FieldDeclaration*> GetFields() const {
        return m_Fields;
    }

    bool IsStruct() const override {
        return !m_IsUnion;
    }

    bool IsUnion() const override {
        return m_IsUnion;
    }

    bool IsEnum() const override {
        return false;
    }

private:
    std::vector<FieldDeclaration*> m_Fields;
    bool m_IsUnion = false;
};

}  // namespace ast
