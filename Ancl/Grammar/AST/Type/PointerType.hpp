#pragma once

#include <Ancl/Grammar/AST/Type/Type.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>
#include <Ancl/Grammar/AST/Type/NodeType.hpp>


namespace ast {

class PointerType: public Type, public INodeType {
public:
    PointerType(QualType* subType): m_SubType(subType) {}

    void SetSubType(QualType* subType) override {
        m_SubType = subType;
    }

    QualType* GetSubType() const override {
        return m_SubType;
    }

private:
    QualType* m_SubType;
};

}  // namespace ast
