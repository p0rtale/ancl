#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class CastExpression: public Expression {
public:
    enum class Kind {
        kNone = 0,
        kLValueToRValue,
    };

public:
    CastExpression(Expression* subExpression, QualType toType);

    CastExpression(Expression* subExpression, Kind castKind);

    void Accept(AstVisitor& visitor) override;

    bool IsLValueToRValue() const;

    void SetSubExpression(Expression* subExpression);
    Expression* GetSubExpression() const;

    QualType GetToType() const;

private:
    Expression* m_SubExpression;
    QualType m_ToType;

    Kind m_CastKind = Kind::kNone;
};

}  // namespace ast
