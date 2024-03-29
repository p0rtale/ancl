#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class InitializerList: public Expression {
public:
    InitializerList(std::vector<Expression*> inits)
        : m_Inits(std::move(inits)) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void AddInit(Expression* init) {
        m_Inits.push_back(init);
    }

    std::vector<Expression*> GetInits() const {
        return m_Inits;
    }

private:
    std::vector<Expression*> m_Inits;
};

}  // namespace ast
