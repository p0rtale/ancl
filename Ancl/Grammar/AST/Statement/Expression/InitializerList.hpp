#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class InitializerList: public Expression {
public:
    InitializerList(const std::vector<Expression*>& inits)
        : m_Inits(inits) {}

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
    // Assignment expressions + Initializer lists
    std::vector<Expression*> m_Inits;
};

}  // namespace ast
