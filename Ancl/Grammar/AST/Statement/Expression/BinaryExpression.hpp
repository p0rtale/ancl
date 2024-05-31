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
                     OpType opType);

    void Accept(AstVisitor& visitor) override;

    void SetLeftOperand(Expression* leftOperand);

    Expression* GetLeftOperand() const;

    void SetRightOperand(Expression* rightOperand);

    Expression* GetRightOperand() const;

    bool IsAssignment() const;
    bool IsRelational() const;
    bool IsEquality() const;
    bool IsBitwiseAssign() const;
    bool IsBitwise() const;
    bool IsArithmetic() const;

    OpType GetOpType() const;
    std::string GetOpTypeStr() const;

private:
    Expression* m_LeftOperand;
    Expression* m_RightOperand;
    OpType m_OpType;
};

}  // namespace ast
