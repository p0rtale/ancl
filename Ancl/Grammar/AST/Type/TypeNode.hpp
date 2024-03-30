#pragma once

#include <Ancl/Visitor/AstVisitor.hpp>


namespace ast {

class TypeNode {
public:
    virtual ~TypeNode() = default;

    virtual void Accept(AstVisitor& visitor) = 0;
};

}  // namespace ast
