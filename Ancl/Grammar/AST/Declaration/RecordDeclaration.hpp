#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Declaration/TagDeclaration.hpp>


namespace ast {

class RecordDeclaration: public TagDeclaration {
public:
    RecordDeclaration(bool isUnion = false): m_IsUnion(isUnion) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    bool IsDefinition() const {
        return !m_InternalDecls.empty();
    }

    void AddInternalTagDecl(TagDeclaration* tagDecl) {
        m_InternalDecls.push_back(tagDecl);
    }

    void AddField(FieldDeclaration* field) {
        m_InternalDecls.push_back(field);
    }

    std::vector<FieldDeclaration*> GetFields() const {
        auto fields = std::vector<FieldDeclaration*>{};
        for (const auto& decl : m_InternalDecls) {
            auto field = dynamic_cast<FieldDeclaration*>(decl);
            if (field) {
                fields.push_back(field);
            }
        }
        return fields;
    }

    std::vector<Declaration*> GetInternalDecls() const {
        return m_InternalDecls;
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
    // TODO: simplify
    // FieldDeclaration | TagDeclaration
    std::vector<Declaration*> m_InternalDecls;

    // std::vector<FieldDeclaration*> m_Fields;
    // std::vector<TagDeclaration*> m_InternalDecls;

    bool m_IsUnion = false;
};

}  // namespace ast
