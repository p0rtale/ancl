#include <Ancl/Grammar/AST/Statement/Expression/CastExpression.hpp>


namespace ast {

CastExpression::CastExpression(Expression* subExpression, QualType toType)
        : m_SubExpression(subExpression), m_ToType(toType) {
    SetType(m_ToType);
    m_ToType.RemoveQualifiers();
}

CastExpression::CastExpression(Expression* subExpression, Kind castKind)
        : m_SubExpression(subExpression), m_ToType(subExpression->GetType()),
          m_CastKind(castKind) {
    SetType(m_ToType);
    m_ToType.RemoveQualifiers();
}

void CastExpression::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

bool CastExpression::IsLValueToRValue() const {
    return m_CastKind == Kind::kLValueToRValue;
}

void CastExpression::SetSubExpression(Expression* subExpression) {
    m_SubExpression = subExpression;
}

Expression* CastExpression::GetSubExpression() const {
    return m_SubExpression;
}

QualType CastExpression::GetToType() const {
    return m_ToType;
}

}  // namespace ast
