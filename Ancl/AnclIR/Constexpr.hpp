#pragma once

#include <Ancl/AnclIR/IR.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>


namespace ir {

class Constexpr {
public:
    Constexpr(IRProgram& program);

    Constant* TryToEvaluate(Instruction* instruction);

    Constant* EvaluateBinaryConstExpr(Constant* leftValue, Constant* rightValue,
                                      BinaryInstruction::OpType opType);

    Constant* EvaluateCompareConstExpr(Constant* leftValue, Constant* rightValue,
                                       CompareInstruction::OpType opType);

    Constant* EvaluateCastConstExpr(Constant* value, Type* toType);

private:
    Constant* evaluateIntegerBinaryConstExpr(IntConstant* leftValue, IntConstant* rightValue,
                                             BinaryInstruction::OpType opType);

    Constant* evaluateFloatBinaryConstExpr(FloatConstant* leftValue, FloatConstant* rightValue,
                                           BinaryInstruction::OpType opType);

    Constant* evaluateIntegerCompareConstExpr(IntConstant* leftValue, IntConstant* rightValue,
                                              CompareInstruction::OpType opType);

    Constant* evaluateFloatCompareConstExpr(FloatConstant* leftValue, FloatConstant* rightValue,
                                            CompareInstruction::OpType opType);

    Constant* toNumberConstant(Value* value) const {
        if (auto* intConstant = dynamic_cast<IntConstant*>(value)) {
            return intConstant;
        }
        if (auto* floatConstant = dynamic_cast<FloatConstant*>(value)) {
            return floatConstant;
        }
        return nullptr;
    }

private:
    IRProgram& m_IRProgram;
};

}  //  namespace ir
