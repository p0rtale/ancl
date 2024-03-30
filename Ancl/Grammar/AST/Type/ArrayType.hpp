#pragma once

#include <Ancl/Grammar/AST/Type/Type.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>
#include <Ancl/Grammar/AST/Type/NodeType.hpp>
#include <Ancl/Grammar/AST/Value/IntValue.hpp>


namespace ast {

class ArrayType: public Type, public INodeType {
public:
    ArrayType(QualType* subType, IntValue size)
        : m_SubType(subType), m_Size(size) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetSubType(QualType* subType) override {
        m_SubType = subType;
    }

    QualType* GetSubType() const override {
        return m_SubType;
    }

    IntValue GetSize() const {
        return m_Size;
    }

    bool IsArrayType() const override {
        return true;
    }

    bool IsFunctionType() const override {
        return false;
    }
    
    bool IsPointerType() const override {
        return false;
    }

private:
    QualType* m_SubType;
    IntValue m_Size;
};

}  // namespace ast
