#pragma once

#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/StorageClass.hpp>


namespace ast {

class VariableDeclaration: public ValueDeclaration {
public:
    VariableDeclaration() = default;
    VariableDeclaration(Expression* init): m_Init(init) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetGlobal() {
        m_IsGlobal = true;
    }

    bool IsGlobal() const {
        return m_IsGlobal;
    }

    bool HasInit() const {
        return m_Init;
    }

    void SetInit(Expression* init) {
        m_Init = init;
    }

    Expression* GetInit() const {
        return m_Init;
    }

    StorageClass GetStorageClass() const {
        return m_StorageClass;
    }

    void SetStorageClass(StorageClass storageClass) {
        m_StorageClass = storageClass;
    }

private:
    Expression* m_Init = nullptr;

    bool m_IsGlobal = false;

    StorageClass m_StorageClass = StorageClass::kNone;
};

}  // namespace ast
