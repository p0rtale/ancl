#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Statement/Statement.hpp>


namespace ast {

class CompoundStatement: public Statement {
public:
    CompoundStatement() = default;

    CompoundStatement(std::vector<Statement*> body)
        : m_Body(std::move(body)) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void AddStatement(Statement* statement) {
        m_Body.push_back(statement);
    }

    std::vector<Statement*> GetBody() const {
        return m_Body;
    }

    size_t GetSize() const {
        return m_Body.size();
    }

private:
    std::vector<Statement*> m_Body;
};

}  // namespace ast
