#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/StorageClass.hpp>
#include <Ancl/Grammar/AST/Type/FunctionType.hpp>


namespace ast {

class FunctionDeclaration: public ValueDeclaration {
public:
    FunctionDeclaration() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetBody(Statement* body) {
        m_Body = body;
    }

    Statement* GetBody() const {
        return m_Body;
    }

    void SetParams(std::vector<VariableDeclaration*> params) {
        m_Params = std::move(params);
    }

    std::vector<VariableDeclaration*> GetParams() const {
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
    std::vector<VariableDeclaration*> m_Params;

    StorageClass m_StorageClass = StorageClass::kNone;
    bool m_IsInline = false;
    bool m_IsVariadic = false;
};

}  // namespace ast
