#include <Ancl/Grammar/AST/Declaration/RecordDeclaration.hpp>


namespace ast {

RecordDeclaration::RecordDeclaration(bool isUnion, bool isDefinition)
    : m_IsUnion(isUnion), m_IsDefinition(isDefinition) {}

void RecordDeclaration::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

bool RecordDeclaration::IsDefinition() const {
    return m_IsDefinition;
}

void RecordDeclaration::AddInternalTagDecl(TagDeclaration* tagDecl) {
    m_InternalDecls.push_back(tagDecl);
}

void RecordDeclaration::AddField(FieldDeclaration* field) {
    m_InternalDecls.push_back(field);
    field->SetPosition(m_FieldsNum++);
}

FieldDeclaration* RecordDeclaration::GetField(const std::string& name) {
    for (Declaration* decl : m_InternalDecls) {
        auto* field = dynamic_cast<FieldDeclaration*>(decl);
        if (field && field->GetName() == name) {
            return field;
        }
    }
    return nullptr;
}

std::vector<FieldDeclaration*> RecordDeclaration::GetFields() const {
    std::vector<FieldDeclaration*> fields;
    for (Declaration* decl : m_InternalDecls) {
        if (auto* field = dynamic_cast<FieldDeclaration*>(decl)) {
            fields.push_back(field);
        }
    }
    return fields;
}

std::vector<Declaration*> RecordDeclaration::GetInternalDecls() const {
    return m_InternalDecls;
}

bool RecordDeclaration::IsStruct() const {
    return !m_IsUnion;
}

bool RecordDeclaration::IsUnion() const {
    return m_IsUnion;
}

bool RecordDeclaration::IsEnum() const {
    return false;
}

}  // namespace ast
