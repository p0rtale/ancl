#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class BinaryExpression: public Expression {
public:
    enum class OpType {
        kNone = 0,

        kMul, kDiv, kRem, kAdd, kSub,
        kShiftL, kShiftR,

        kAnd, kXor, kOr,

        kLogAnd, kLogOr,

        kLess, kGreater, kLessEq, kGreaterEq,
        kEqual, kNEqual,

        kAssign, kMulAssign, kDivAssign,
        kRemAssign, kAddAssign, kSubAssign,
        kShiftLAssign, kShiftRAssign,
        kAndAssign, kXorAssign, kOrAssign,

        // TODO: separate classes?
        kArrSubscript,
        kDirectMember,
        kArrowMember,
    };

public:
    BinaryExpression(Expression* leftOperand, Expression* rightOperand,
                     OpType opType)
        : m_LeftOperand(leftOperand), m_RightOperand(rightOperand),
          m_OpType(opType) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetLeftOperand(Expression* leftOperand) {
        m_LeftOperand = leftOperand;
    }

    Expression* GetLeftOperand() const {
        return m_LeftOperand;
    }

    void SetRightOperand(Expression* rightOperand) {
        m_RightOperand = rightOperand;
    }

    Expression* GetRightOperand() const {
        return m_RightOperand;
    }

    bool IsAssignment() const {
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

    bool IsRelational() const {
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

    bool IsEquality() const {
        switch (m_OpType) {
        case OpType::kEqual:
        case OpType::kNEqual:
            return true;
        default:
            return false;
        }
    }

    bool IsBitwiseAssign() const {
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

    bool IsBitwise() const {
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

    bool IsArithmetic() const {
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

    OpType GetOpType() const {
        return m_OpType;
    }

    std::string GetOpTypeStr() const {
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

private:
    Expression* m_LeftOperand;
    Expression* m_RightOperand;
    OpType m_OpType;
};

}  // namespace ast
