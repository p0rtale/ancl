#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/ParameterDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/StorageClass.hpp>
#include <Ancl/Grammar/AST/Type/FunctionType.hpp>


namespace ast {

class FunctionDeclaration: public ValueDeclaration {
public:
    FunctionDeclaration() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    bool IsDefinition() const {
        return m_Body;
    }

    void SetBody(Statement* body) {
        m_Body = body;
    }

    bool HasBody() const {
        return m_Body;
    }

    Statement* GetBody() const {
        return m_Body;
    }

    void SetParams(const std::vector<ParameterDeclaration*>& params) {
        m_Params = params;
    }

    std::vector<ParameterDeclaration*> GetParams() const {
        return m_Params;
    }

    StorageClass GetStorageClass() const {
        return m_StorageClass;
    }

    void SetStorageClass(StorageClass storageClass) {
        m_StorageClass = storageClass;
    }

    bool IsInline() const {
        return m_IsInline;
    }

    void SetInline() {
        m_IsInline = true;
    }

    void SetVariadic() {
        m_IsVariadic = true;
    }

    bool IsVariadic() const {
        return m_IsVariadic;
    }

private:
    Statement* m_Body;
    std::vector<ParameterDeclaration*> m_Params;

    StorageClass m_StorageClass = StorageClass::kNone;
    bool m_IsInline = false;
    bool m_IsVariadic = false;
};

}  // namespace ast
