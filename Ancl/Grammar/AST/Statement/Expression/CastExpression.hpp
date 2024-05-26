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
    CastExpression(Expression* subExpression, QualType toType)
            : m_SubExpression(subExpression), m_ToType(toType) {
        SetType(m_ToType);
        m_ToType.RemoveQualifiers();
    }

    CastExpression(Expression* subExpression, Kind castKind)
            : m_SubExpression(subExpression), m_ToType(subExpression->GetType()),
              m_CastKind(castKind) {
        SetType(m_ToType);
        m_ToType.RemoveQualifiers();
    }

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    bool IsLValueToRValue() const {
        return m_CastKind == Kind::kLValueToRValue;
    }

    void SetSubExpression(Expression* subExpression) {
        m_SubExpression = subExpression;
    }

    Expression* GetSubExpression() const {
        return m_SubExpression;
    }

    QualType GetToType() const {
        return m_ToType;
    }

private:
    Expression* m_SubExpression;
    QualType m_ToType;

    Kind m_CastKind = Kind::kNone;
};

}  // namespace ast
