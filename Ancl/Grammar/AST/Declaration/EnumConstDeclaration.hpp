#pragma once

#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>


namespace ast {

class EnumConstDeclaration: public ValueDeclaration {
public:
    EnumConstDeclaration() = default;
    EnumConstDeclaration(Expression* init): m_Init(init) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    bool HasInit() const {
        return m_Init;
    }

    void SetInit(Expression* init) {
        m_Init = init;
    }

    Expression* GetInit() const {
        return m_Init;
    }

private:
    Expression* m_Init = nullptr;
};

}  // namespace ast
