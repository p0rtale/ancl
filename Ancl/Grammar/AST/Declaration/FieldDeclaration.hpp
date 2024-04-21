#pragma once

#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>


namespace ast {

class FieldDeclaration: public ValueDeclaration {
public:
    FieldDeclaration() = default;

    FieldDeclaration(std::string name, QualType* type = nullptr)
        : ValueDeclaration(std::move(name), type) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetPosition(size_t position) {
        m_Position = position;
    }

    size_t GetPosition() const {
        return m_Position;
    }

private:
    size_t m_Position = 0;
};

}  // namespace ast
