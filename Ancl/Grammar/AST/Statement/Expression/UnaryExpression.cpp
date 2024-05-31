#include <Ancl/Grammar/AST/Statement/Expression/UnaryExpression.hpp>


namespace ast {

UnaryExpression::UnaryExpression(Expression* operand, OpType opType)
    : m_Operand(operand), m_OpType(opType) {}

void UnaryExpression::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

void UnaryExpression::SetOperand(Expression* operand) {
    m_Operand = operand;
}

Expression* UnaryExpression::GetOperand() const {
    return m_Operand;
}

UnaryExpression::OpType UnaryExpression::GetOpType() const {
    return m_OpType;
}

std::string UnaryExpression::GetOpTypeStr() const {
    switch (m_OpType) {
        case OpType::kNone:  return "None";

        case OpType::kPreInc:   return "prefix ++";
        case OpType::kPreDec:   return "prefix --";
        case OpType::kPostInc:  return "postfix ++";
        case OpType::kPostDec:  return "postfix --";

        case OpType::kAddrOf:  return "&";
        case OpType::kDeref:   return "*";
        case OpType::kPlus:    return "+";
        case OpType::kMinus:   return "-";
        case OpType::kNot:     return "~";
        case OpType::kLogNot:  return "!";

        case OpType::kSizeof:  return "sizeof";

        default: {
            return "";
        }
    }
}

}  // namespace ast
