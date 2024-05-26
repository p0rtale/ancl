#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Declaration/TagDeclaration.hpp>


namespace ast {

class RecordDeclaration: public TagDeclaration {
public:
    RecordDeclaration(bool isUnion = false, bool isDefinition = false)
        : m_IsUnion(isUnion), m_IsDefinition(isDefinition) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    bool IsDefinition() const override {
        return m_IsDefinition;
    }

    void AddInternalTagDecl(TagDeclaration* tagDecl) {
        m_InternalDecls.push_back(tagDecl);
    }

    void AddField(FieldDeclaration* field) {
        m_InternalDecls.push_back(field);
        field->SetPosition(m_FieldsNum++);
    }

    FieldDeclaration* GetField(const std::string& name) {
        for (Declaration* decl : m_InternalDecls) {
            auto* field = dynamic_cast<FieldDeclaration*>(decl);
            if (field && field->GetName() == name) {
                return field;
            }
        }
        return nullptr;
    }

    std::vector<FieldDeclaration*> GetFields() const {
        std::vector<FieldDeclaration*> fields;
        for (Declaration* decl : m_InternalDecls) {
            if (auto* field = dynamic_cast<FieldDeclaration*>(decl)) {
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

    size_t m_FieldsNum = 0;

    // std::vector<FieldDeclaration*> m_Fields;
    // std::vector<TagDeclaration*> m_InternalDecls;

    bool m_IsUnion = false;
    bool m_IsDefinition = false;
};

}  // namespace ast
