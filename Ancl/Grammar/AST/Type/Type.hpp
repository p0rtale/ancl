#pragma once

#include <Ancl/Grammar/AST/Type/TypeNode.hpp>


namespace ast {

class Type: public TypeNode {
public:
    virtual ~Type() = default;
};

}  // namespace ast
