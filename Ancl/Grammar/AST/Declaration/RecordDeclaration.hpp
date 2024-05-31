#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Declaration/FieldDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/TagDeclaration.hpp>


namespace ast {

class RecordDeclaration: public TagDeclaration {
public:
    RecordDeclaration(bool isUnion = false, bool isDefinition = false);

    void Accept(AstVisitor& visitor) override;

    bool IsDefinition() const override;

    void AddInternalTagDecl(TagDeclaration* tagDecl);
    void AddField(FieldDeclaration* field);

    FieldDeclaration* GetField(const std::string& name);

    std::vector<FieldDeclaration*> GetFields() const;

    std::vector<Declaration*> GetInternalDecls() const;

    bool IsStruct() const override;
    bool IsUnion() const override;
    bool IsEnum() const override;

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
