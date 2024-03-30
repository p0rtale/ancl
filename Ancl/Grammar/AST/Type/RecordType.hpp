#pragma once

#include <Ancl/Grammar/AST/Type/TagType.hpp>
#include <Ancl/Grammar/AST/Declaration/RecordDeclaration.hpp>


namespace ast {

class RecordType: public TagType {
public:
    RecordType(RecordDeclaration* declaration)
        : m_Declaration(declaration) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    RecordDeclaration* GetDeclaration() const {
        return m_Declaration;
    }

    bool IsRecord() const override {
        return true;
    }

    bool IsEnum() const override {
        return false;
    }

private:
    RecordDeclaration* m_Declaration;
};

}  // namespace ast
