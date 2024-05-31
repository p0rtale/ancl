#include <Ancl/Grammar/AST/Statement/Expression/BinaryExpression.hpp>


namespace ast {

BinaryExpression::BinaryExpression(Expression* leftOperand, Expression* rightOperand,
                    OpType opType)
    : m_LeftOperand(leftOperand), m_RightOperand(rightOperand),
        m_OpType(opType) {}

void BinaryExpression::Accept(AstVisitor& visitor) {
    visitor.Visit(*this);
}

void BinaryExpression::SetLeftOperand(Expression* leftOperand) {
    m_LeftOperand = leftOperand;
}

Expression* BinaryExpression::GetLeftOperand() const {
    return m_LeftOperand;
}

void BinaryExpression::SetRightOperand(Expression* rightOperand) {
    m_RightOperand = rightOperand;
}

Expression* BinaryExpression::GetRightOperand() const {
    return m_RightOperand;
}

bool BinaryExpression::IsAssignment() const {
    switch (m_OpType) {
        case OpType::kAssign:
        case OpType::kMulAssign:
        case OpType::kDivAssign:
        case OpType::kRemAssign:
        case OpType::kAddAssign:
        case OpType::kSubAssign:
        case OpType::kShiftLAssign:
        case OpType::kShiftRAssign:
        case OpType::kAndAssign:
        case OpType::kXorAssign:
        case OpType::kOrAssign:
            return true;
        default:
            return false;
    }
}

bool BinaryExpression::IsRelational() const {
    switch (m_OpType) {
        case OpType::kLess:
        case OpType::kGreater:
        case OpType::kLessEq:
        case OpType::kGreaterEq:
            return true;
        default:
            return false;
    }
}

bool BinaryExpression::IsEquality() const {
    switch (m_OpType) {
        case OpType::kEqual:
        case OpType::kNEqual:
            return true;
        default:
            return false;
    }
}

bool BinaryExpression::IsBitwiseAssign() const {
    switch (m_OpType) {
        case OpType::kShiftLAssign:
        case OpType::kShiftRAssign:
        case OpType::kAndAssign:
        case OpType::kXorAssign:
        case OpType::kOrAssign:
            return true;
        default:
            return false;
    }    
}

bool BinaryExpression::IsBitwise() const {
    switch (m_OpType) {
        case OpType::kShiftL:
        case OpType::kShiftR:
        case OpType::kAnd:
        case OpType::kXor:
        case OpType::kOr:
            return true;
        default:
            return false;
    }    
}

bool BinaryExpression::IsArithmetic() const {
    switch (m_OpType) {
        case OpType::kMul:
        case OpType::kDiv:
        case OpType::kRem:
        case OpType::kAdd:
        case OpType::kSub:
        case OpType::kShiftL:
        case OpType::kShiftR:
        case OpType::kAnd:
        case OpType::kXor:
        case OpType::kOr:
        case OpType::kLogAnd:
        case OpType::kLogOr:
            return true;
        default:
            return false;
    }
}

BinaryExpression::OpType BinaryExpression::GetOpType() const {
    return m_OpType;
}

std::string BinaryExpression::GetOpTypeStr() const {
    switch (m_OpType) {
        case OpType::kNone:  return "None";

        case OpType::kMul:     return "*";
        case OpType::kDiv:     return "/";
        case OpType::kRem:     return "%";
        case OpType::kAdd:     return "+";
        case OpType::kSub:     return "-";
        case OpType::kShiftL:  return "<<";
        case OpType::kShiftR:  return ">>";

        case OpType::kAnd:     return "&";
        case OpType::kXor:     return "^";
        case OpType::kOr:      return "|";

        case OpType::kLogAnd:  return "&&";
        case OpType::kLogOr:   return "||";

        case OpType::kLess:       return "<";
        case OpType::kGreater:    return ">";
        case OpType::kLessEq:     return "<=";
        case OpType::kGreaterEq:  return ">=";
        case OpType::kEqual:      return "==";
        case OpType::kNEqual:     return "!=";

        case OpType::kAssign:        return "=";
        case OpType::kMulAssign:     return "*=";
        case OpType::kDivAssign:     return "/=";
        case OpType::kRemAssign:     return "%=";
        case OpType::kAddAssign:     return "+=";
        case OpType::kSubAssign:     return "-=";
        case OpType::kShiftLAssign:  return "<<=";
        case OpType::kShiftRAssign:  return ">>=";
        case OpType::kAndAssign:     return "&=";
        case OpType::kXorAssign:     return "^=";
        case OpType::kOrAssign:      return "|=";

        case OpType::kArrSubscript:  return "[]";

        case OpType::kDirectMember:  return ".";
        case OpType::kArrowMember:   return "->";

        default:
            return "";
    }
}

}  // namespace ast
