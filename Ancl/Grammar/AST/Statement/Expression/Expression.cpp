#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

void Expression::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

QualType Expression::GetType() const {
    return m_Type;
}

void Expression::SetType(QualType type) {
    m_Type = type;
}

void Expression::SetLValue() {
    m_IsLValue = true;
}

void Expression::SetRValue() {
    m_IsLValue = false;
}

bool Expression::IsLValue() const {
    return m_IsLValue;
}

bool Expression::IsRValue() const {
    return !m_IsLValue;
}

bool Expression::IsModifiableLValue() const {
    // TODO: check type
    return m_IsLValue;
}

}  // namespace ast
