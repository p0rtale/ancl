#pragma once

#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>


namespace ast {

class EnumConstDeclaration: public ValueDeclaration {
public:
    EnumConstDeclaration() = default;
    EnumConstDeclaration(ConstExpression* init): m_Init(init) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    bool HasInit() const {
        return m_Init;
    }

    void SetInit(ConstExpression* init) {
        m_Init = init;
    }

    ConstExpression* GetInit() const {
        return m_Init;
    }

private:
    ConstExpression* m_Init = nullptr;
};

}  // namespace ast
