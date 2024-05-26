#pragma once

#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>


namespace ast {

class UnaryExpression: public Expression {
public:
    enum class OpType {
        kNone = 0,
        kPreInc,
        kPreDec,
        kPostInc,
        kPostDec,
        kAddrOf,
        kDeref,
        kPlus,
        kMinus,
        kNot,
        kLogNot,
        kSizeof,
    };

public:
    UnaryExpression(Expression* operand, OpType opType)
        : m_Operand(operand), m_OpType(opType) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    void SetOperand(Expression* operand) {
        m_Operand = operand;
    }

    Expression* GetOperand() const {
        return m_Operand;
    }

    OpType GetOpType() const {
        return m_OpType;
    }

    std::string GetOpTypeStr() const {
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

private:
    Expression* m_Operand;
    OpType m_OpType;
};

}  // namespace ast
