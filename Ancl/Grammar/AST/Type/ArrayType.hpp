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

    void SetSubType(QualType* subType) override {
        m_SubType = subType;
    }

    QualType* GetSubType() const override {
        return m_SubType;
    }

    IntValue GetSize() const {
        return m_Size;
    }

private:
    QualType* m_SubType;
    IntValue m_Size;
};

}  // namespace ast
