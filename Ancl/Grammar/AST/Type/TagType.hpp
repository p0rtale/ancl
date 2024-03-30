#pragma once

#include <Ancl/Grammar/AST/Type/Type.hpp>


namespace ast {

class TagType: public Type {
public:
    virtual ~TagType() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    virtual bool IsRecord() const = 0;

    virtual bool IsEnum() const = 0;
};

}  // namespace ast
