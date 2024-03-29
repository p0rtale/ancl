#pragma once

#include <Ancl/Visitor/AstVisitor.hpp>


namespace ast {

class ASTNode {
public:
    virtual ~ASTNode() = default;

    virtual void Accept(AstVisitor& visitor) = 0;
};

}  // namespace ast
