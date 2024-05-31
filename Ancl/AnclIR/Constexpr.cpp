#include <Ancl/AnclIR/Constexpr.hpp>

#include <cassert>

#include <Ancl/DataLayout/Alignment.hpp>


namespace ir {

Constexpr::Constexpr(IRProgram& program): m_IRProgram(program) {}

Constant* Constexpr::TryToEvaluate(Instruction* instruction) {
    if (auto* binary = dynamic_cast<BinaryInstruction*>(instruction)) {
        auto* leftConstant = toNumberConstant(binary->GetLeftOperand());
        auto* rightConstant = toNumberConstant(binary->GetRightOperand());
        if (leftConstant && rightConstant) {
            return EvaluateBinaryConstExpr(leftConstant, rightConstant, binary->GetOpType());
        }

        return nullptr;
    }

    if (auto* cast = dynamic_cast<CastInstruction*>(instruction)) {
        auto* constant = toNumberConstant(cast->GetFromOperand());
        if (constant) {
            return EvaluateCastConstExpr(constant, cast->GetToType());
        }

        return nullptr;
    }

    if (auto* compare = dynamic_cast<CompareInstruction*>(instruction)) {
        auto* leftConstant = toNumberConstant(compare->GetLeftOperand());
        auto* rightConstant = toNumberConstant(compare->GetRightOperand());
        if (leftConstant && rightConstant) {
            return EvaluateCompareConstExpr(leftConstant, rightConstant, compare->GetOpType());
        }

        return nullptr;
    }

    return nullptr;
}

Constant* Constexpr::EvaluateBinaryConstExpr(Constant* leftValue, Constant* rightValue,
                                             BinaryInstruction::OpType opType) {
    Type* leftType = leftValue->GetType();
    Type* rightType = rightValue->GetType();
    assert(Alignment::GetTypeSize(leftType) == Alignment::GetTypeSize(rightType));

    switch (opType) {
        case BinaryInstruction::OpType::kMul:
        case BinaryInstruction::OpType::kSDiv:
        case BinaryInstruction::OpType::kUDiv:
        case BinaryInstruction::OpType::kSRem:
        case BinaryInstruction::OpType::kURem:
        case BinaryInstruction::OpType::kAdd:
        case BinaryInstruction::OpType::kSub:
        case BinaryInstruction::OpType::kShiftL:
        case BinaryInstruction::OpType::kLShiftR:
        case BinaryInstruction::OpType::kAShiftR:
        case BinaryInstruction::OpType::kAnd:
        case BinaryInstruction::OpType::kXor:
        case BinaryInstruction::OpType::kOr:
            return evaluateIntegerBinaryConstExpr(static_cast<IntConstant*>(leftValue),
                                                    static_cast<IntConstant*>(rightValue),
                                                    opType);

        case BinaryInstruction::OpType::kFMul:
        case BinaryInstruction::OpType::kFDiv:
        case BinaryInstruction::OpType::kFAdd:
        case BinaryInstruction::OpType::kFSub:
            return evaluateFloatBinaryConstExpr(static_cast<FloatConstant*>(leftValue),
                                                static_cast<FloatConstant*>(rightValue),
                                                opType);

        default:
            assert(false);
            break;
    }
}

Constant* Constexpr::EvaluateCompareConstExpr(Constant* leftValue, Constant* rightValue,
                                              CompareInstruction::OpType opType) {
    Type* leftType = leftValue->GetType();
    Type* rightType = rightValue->GetType();
    assert(Alignment::GetTypeSize(leftType) == Alignment::GetTypeSize(rightType));

    switch (opType) {
        case CompareInstruction::OpType::kIULess:
        case CompareInstruction::OpType::kIUGreater:
        case CompareInstruction::OpType::kIULessEq:
        case CompareInstruction::OpType::kIUGreaterEq:
        case CompareInstruction::OpType::kISLess:
        case CompareInstruction::OpType::kISGreater:
        case CompareInstruction::OpType::kISLessEq:
        case CompareInstruction::OpType::kISGreaterEq:
            return evaluateIntegerCompareConstExpr(static_cast<IntConstant*>(leftValue),
                                                    static_cast<IntConstant*>(rightValue),
                                                    opType);

        case CompareInstruction::OpType::kFLess:
        case CompareInstruction::OpType::kFGreater:
        case CompareInstruction::OpType::kFLessEq:
        case CompareInstruction::OpType::kFGreaterEq:
        case CompareInstruction::OpType::kFEqual:
        case CompareInstruction::OpType::kFNEqual:
            return evaluateFloatCompareConstExpr(static_cast<FloatConstant*>(leftValue),
                                                    static_cast<FloatConstant*>(rightValue),
                                                    opType);

        default:
            assert(false);
            break;
    }
}

Constant* Constexpr::EvaluateCastConstExpr(Constant* value, Type* toType) {
    uint64_t toSize = ir::Alignment::GetTypeBitSize(toType);

    ir::Type* fromType = value->GetType();
    bool fromFloat = dynamic_cast<ir::FloatType*>(fromType);
    bool toFloat = dynamic_cast<ir::FloatType*>(toType);

    auto* intToType = dynamic_cast<ir::IntType*>(toType);
    auto* floatToType = dynamic_cast<ir::FloatType*>(toType);

    auto* intConst = dynamic_cast<ir::IntConstant*>(value);
    auto* floatConst = dynamic_cast<ir::FloatConstant*>(value);

    if (!fromFloat && !toFloat) {
        IntValue intValue = intConst->GetValue();
        uint64_t value = intValue.GetUnsignedValue();
        if (toSize < 64) {
            value = value & ((1ULL << toSize) - 1);
        }

        return m_IRProgram.CreateValue<ir::IntConstant>(intToType, IntValue(value));
    }

    if (!fromFloat && toFloat) {
        IntValue intValue = intConst->GetValue();
        double value = intValue.GetUnsignedValue();
        return m_IRProgram.CreateValue<ir::FloatConstant>(floatToType, FloatValue(value));
    }

    if (fromFloat && !toFloat) {
        FloatValue floatValue = floatConst->GetValue();
        uint64_t value = floatValue.GetValue();
        return m_IRProgram.CreateValue<ir::IntConstant>(intToType, IntValue(value));
    }

    // TODO: AMD64 requires the materialization of float constants,
    //       so the castes between float and double cannot be removed.
    //       Add this requirement to TargetMachine?

    // FloatValue floatValue = floatConst->GetValue();
    // if (toSize == 32) {
    //     return m_IRProgram.CreateValue<ir::FloatConstant>(floatToType, FloatValue((float)floatValue.GetValue()));
    // }
    // return floatConst;

    return value;
}

Constant* Constexpr::evaluateIntegerBinaryConstExpr(IntConstant* leftValue, IntConstant* rightValue,
                                                    BinaryInstruction::OpType opType) {
    auto* type = static_cast<IntType*>(leftValue->GetType());

    IntValue leftIntValue = leftValue->GetValue();
    int64_t lhs = leftIntValue.GetSignedValue();

    IntValue rightIntValue = rightValue->GetValue();
    int64_t rhs = rightIntValue.GetSignedValue();

    int64_t result = 0;
    switch (opType) {
        case BinaryInstruction::OpType::kMul:
            result = lhs * rhs;
            break;
        case BinaryInstruction::OpType::kSDiv:
            result = lhs / rhs;
            break;
        case BinaryInstruction::OpType::kUDiv:
            result = (uint64_t)lhs / rhs;
            break;
        case BinaryInstruction::OpType::kSRem:
            result = lhs % rhs;
            break;
        case BinaryInstruction::OpType::kURem:
            result = (uint64_t)lhs % rhs;
            break;
        case BinaryInstruction::OpType::kAdd:
            result = lhs + rhs;
            break;
        case BinaryInstruction::OpType::kSub:
            result = lhs - rhs;
            break;
        case BinaryInstruction::OpType::kShiftL:
            result = lhs << rhs;
            break;
        case BinaryInstruction::OpType::kLShiftR:
            result = (uint64_t)lhs >> rhs;
            break;
        case BinaryInstruction::OpType::kAShiftR:
            result = lhs >> rhs;
            break;
        case BinaryInstruction::OpType::kAnd:
            result = lhs & rhs;
            break;
        case BinaryInstruction::OpType::kXor:
            result = lhs ^ rhs;
            break;
        case BinaryInstruction::OpType::kOr:
            result = lhs | rhs;
            break;
        default:
            assert(false);
            break;
    }

    return m_IRProgram.CreateValue<IntConstant>(type, IntValue(result));
}

Constant* Constexpr::evaluateFloatBinaryConstExpr(FloatConstant* leftValue, FloatConstant* rightValue,
                                                  BinaryInstruction::OpType opType) {
    auto* type = static_cast<FloatType*>(leftValue->GetType());

    FloatValue leftFloatValue = leftValue->GetValue();
    double lhs = leftFloatValue.GetValue();

    FloatValue rightFloatValue = rightValue->GetValue();
    double rhs = rightFloatValue.GetValue();

    double result = 0;
    switch (opType) {
        case BinaryInstruction::OpType::kFMul:
            result = lhs * rhs;
            break;
        case BinaryInstruction::OpType::kFDiv:
            result = lhs / rhs;
            break;
        case BinaryInstruction::OpType::kFAdd:
            result = lhs + rhs;
            break;
        case BinaryInstruction::OpType::kFSub:
            result = lhs - rhs;
            break;
        default:
            assert(false);
            break;
    }

    // return m_IRProgram.CreateValue<FloatConstant>(type, FloatValue(result));
    return nullptr;
}

Constant* Constexpr::evaluateIntegerCompareConstExpr(IntConstant* leftValue, IntConstant* rightValue,
                                                     CompareInstruction::OpType opType) {
    IntValue leftIntValue = leftValue->GetValue();
    int64_t lhs = leftIntValue.GetSignedValue();

    IntValue rightIntValue = rightValue->GetValue();
    int64_t rhs = rightIntValue.GetSignedValue();

    int result = 0;
    switch (opType) {
        case CompareInstruction::OpType::kIULess:
            result = (uint64_t)lhs < rhs;
            break;
        case CompareInstruction::OpType::kIUGreater:
            result = (uint64_t)lhs > rhs;
            break;
        case CompareInstruction::OpType::kIULessEq:
            result = (uint64_t)lhs <= rhs;
            break;
        case CompareInstruction::OpType::kIUGreaterEq:
            result = (uint64_t)lhs >= rhs;
            break;
        case CompareInstruction::OpType::kISLess:
            result = lhs < rhs;
            break;
        case CompareInstruction::OpType::kISGreater:
            result = lhs > rhs;
            break;
        case CompareInstruction::OpType::kISLessEq:
            result = lhs <= rhs;
            break;
        case CompareInstruction::OpType::kISGreaterEq:
            result = lhs >= rhs;
            break;
        case CompareInstruction::OpType::kIEqual:
            result = lhs == rhs;
            break;
        case CompareInstruction::OpType::kINEqual:
            result = lhs != rhs;
            break;
        default:
            assert(false);
            break;
    }

    return m_IRProgram.CreateValue<IntConstant>(IntType::Create(m_IRProgram, 1), IntValue(result));
}

Constant* Constexpr::evaluateFloatCompareConstExpr(FloatConstant* leftValue, FloatConstant* rightValue,
                                                   CompareInstruction::OpType opType) {
    FloatValue leftFloatValue = leftValue->GetValue();
    double lhs = leftFloatValue.GetValue();

    FloatValue rightFloatValue = rightValue->GetValue();
    double rhs = rightFloatValue.GetValue();

    int result = 0;
    switch (opType) {
        case CompareInstruction::OpType::kFLess:
            result = lhs < rhs;
            break;
        case CompareInstruction::OpType::kFGreater:
            result = lhs > rhs;
            break;
        case CompareInstruction::OpType::kFLessEq:
            result = lhs <= rhs;
            break;
        case CompareInstruction::OpType::kFGreaterEq:
            result = lhs >= rhs;
            break;
        case CompareInstruction::OpType::kFEqual:
            result = lhs == rhs;
            break;
        case CompareInstruction::OpType::kFNEqual:
            result = lhs != rhs;
            break;
        default:
            assert(false);
            break;
    }

    return m_IRProgram.CreateValue<IntConstant>(IntType::Create(m_IRProgram, 1), IntValue(result));
}

}  //  namespace ir
