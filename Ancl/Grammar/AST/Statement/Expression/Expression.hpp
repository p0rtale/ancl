#pragma once

#include <Ancl/Grammar/AST/Statement/ValueStatement.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>


namespace ast {

class Expression: public ValueStatement {
public:
    virtual ~Expression() = default;

    void Accept(AstVisitor& visitor) override;

    QualType GetType() const;
    void SetType(QualType type);

    void SetLValue();
    void SetRValue();

    bool IsLValue() const;
    bool IsRValue() const;

    bool IsModifiableLValue() const;

private:
    QualType m_Type;

    bool m_IsLValue = false;
};

}  // namespace ast
