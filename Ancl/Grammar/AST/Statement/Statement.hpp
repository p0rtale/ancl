#pragma once

#include <Ancl/Grammar/AST/Base/ASTNode.hpp>


namespace ast {

class Statement: public ASTNode {
public:
    virtual ~Statement() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }
};

}  // namespace ast
