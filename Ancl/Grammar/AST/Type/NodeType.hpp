#pragma once

#include <Ancl/Grammar/AST/Type/QualType.hpp>


namespace ast {

class INodeType {
public:
    virtual void SetSubType(QualType) = 0;

    virtual QualType GetSubType() const = 0;

    virtual bool IsArrayType() const = 0;
    virtual bool IsFunctionType() const = 0;
    virtual bool IsPointerType() const = 0;
};

}  // namespace ast
