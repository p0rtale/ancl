#pragma once

#include <Ancl/Grammar/AST/Statement/ValueStatement.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>


namespace ast {

class Expression: public ValueStatement {
public:
    virtual ~Expression() = default;

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    QualType GetType() const {
        return m_Type;
    }

    void SetType(QualType type) {
        m_Type = type;
    }

    bool IsLValue() const {
        return m_IsLValue;
    }

    bool IsRValue() const {
        return !m_IsLValue;
    }

    bool IsModifiableLValue() const {
        // TODO: check type
        return m_IsLValue;
    }

    void SetLValue() {
        m_IsLValue = true;
    }

    void SetRValue() {
        m_IsLValue = false;
    }

private:
    QualType m_Type;

    bool m_IsLValue = false;
};

}  // namespace ast
