#pragma once

#include <Ancl/Grammar/AST/Type/QualType.hpp>


namespace ast {

class INodeType {
    virtual void SetSubType(QualType*) = 0;

    virtual QualType* GetSubType() const = 0; 
};

}  // namespace ast
