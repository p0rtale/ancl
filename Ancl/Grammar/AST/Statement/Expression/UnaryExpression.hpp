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
    UnaryExpression(Expression* operand, OpType opType);

    void Accept(AstVisitor& visitor) override;

    void SetOperand(Expression* operand);
    Expression* GetOperand() const;

    OpType GetOpType() const;
    std::string GetOpTypeStr() const;

private:
    Expression* m_Operand;
    OpType m_OpType;
};

}  // namespace ast
