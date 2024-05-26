#pragma once

#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/StorageClass.hpp>


namespace ast {

class ParameterDeclaration: public ValueDeclaration {
public:
    ParameterDeclaration() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    StorageClass GetStorageClass() const {
        return m_StorageClass;
    }

    void SetStorageClass(StorageClass storageClass) {
        m_StorageClass = storageClass;
    }

private:
    StorageClass m_StorageClass = StorageClass::kNone;
};

}  // namespace ast
