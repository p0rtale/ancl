#pragma once

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Visitor/AstVisitor.hpp>

#include <Ancl/Grammar/AST/Value/Value.hpp>

#include <Ancl/Logger/Logger.hpp>


namespace ast {

class IntConstExprAstVisitor: public AstVisitor {
public:
    enum class Status {
        kNone = 0,
        kOk,
        kError,
    };

public:
    IntConstExprAstVisitor() = default;

    Status Evaluate(ConstExpression& constExpr) {
        Visit(constExpr);
        if (!m_Value.IsInteger()) {
            return Status::kError;
        }
        return Status::kOk;
    }

public:
    /*
    =================================================================
                                Declaration
    =================================================================
    */

    // Skip
    void Visit(Declaration&) override {}
    void Visit(EnumConstDeclaration& enumConstDecl) override {}
    void Visit(EnumDeclaration& enumDecl) override {}
    void Visit(FieldDeclaration& fieldDecl) override {}
    void Visit(FunctionDeclaration& funcDecl) override {}
    void Visit(LabelDeclaration& labelDecl) override {}
    void Visit(ParameterDeclaration& paramDecl) override {}
    void Visit(RecordDeclaration& recordDecl) override {}
    void Visit(TagDeclaration&) override {}
    void Visit(TranslationUnit& unit) override {}
    void Visit(TypeDeclaration&) override {}
    void Visit(TypedefDeclaration& typedefDecl) override {}
    void Visit(ValueDeclaration& valueDecl) override {}
    void Visit(VariableDeclaration& varDecl) override {}


    /*
    =================================================================
                                Statement
    =================================================================
    */

    // Skip
    void Visit(Statement&) override {}
    void Visit(CaseStatement& caseStmt) override {}
    void Visit(CompoundStatement& compoundStmt) override {}
    void Visit(DeclStatement& declStmt) override {}
    void Visit(DefaultStatement& defaultStmt) override {}
    void Visit(DoStatement& doStmt) override {}
    void Visit(ForStatement& forStmt) override {}
    void Visit(GotoStatement& gotoStmt) override {}
    void Visit(IfStatement& ifStmt) override {}
    void Visit(LabelStatement& labelStmt) override {}
    void Visit(LoopJumpStatement& loopJmpStmt) override {}
    void Visit(ReturnStatement& returnStmt) override {}
    void Visit(SwitchCase& switchCase) override {}
    void Visit(SwitchStatement& switchStmt) override {}
    void Visit(ValueStatement& valueStmt) override {}
    void Visit(WhileStatement& whileStmt) override {}


    /*
    =================================================================
                                Expression
    =================================================================
    */

    // TODO: template Visitor
    Value Accept(Expression& expr) {
        expr.Accept(*this);  // -> m_Value
        return m_Value;
    }

    void Visit(Expression&) override {
        // Base class
    }

    void Visit(BinaryExpression& binaryExpr) override {
        BinaryExpression::OpType opType = binaryExpr.GetOpType();
        switch (opType) {
        case BinaryExpression::OpType::kAssign:
        case BinaryExpression::OpType::kMulAssign:
        case BinaryExpression::OpType::kDivAssign:
        case BinaryExpression::OpType::kRemAssign:
        case BinaryExpression::OpType::kAddAssign:
        case BinaryExpression::OpType::kSubAssign:
        case BinaryExpression::OpType::kShiftLAssign:
        case BinaryExpression::OpType::kShiftRAssign:
        case BinaryExpression::OpType::kAndAssign:
        case BinaryExpression::OpType::kXorAssign:
        case BinaryExpression::OpType::kOrAssign:
        case BinaryExpression::OpType::kArrSubscript:
        case BinaryExpression::OpType::kDirectMember:
        case BinaryExpression::OpType::kArrowMember:
            printSemanticError("expression is not a constant expression",
                               binaryExpr.GetLocation());
        default:
            break;
        }

        Expression* leftOperand = binaryExpr.GetLeftOperand();
        Value leftValue = Accept(*leftOperand);

        Expression* rightOperand = binaryExpr.GetRightOperand();
        
        // Lazy evaluation
        if (opType == BinaryExpression::OpType::kLogAnd || 
                opType == BinaryExpression::OpType::kLogOr) {
            int64_t leftSValue = 0;
            if (leftValue.IsInteger()) {
                IntValue leftIntValue = leftValue.GetIntValue();
                leftSValue = leftIntValue.GetSignedValue();
            } else {
                double leftFloatValue = leftValue.GetFloatValue().GetValue();
                leftSValue = reinterpret_cast<int64_t&>(leftFloatValue);
            }

            if (opType == BinaryExpression::OpType::kLogAnd && leftSValue == 0) {
                m_Value = IntValue(1);
                return;
            }
            if (opType == BinaryExpression::OpType::kLogOr && leftSValue != 0) {
                m_Value = IntValue(1);
                return;
            }

            Value rightValue = Accept(*rightOperand);
            int64_t rightSValue = 0;
            if (rightValue.IsInteger()) {
                IntValue rightIntValue = rightValue.GetIntValue();
                rightSValue = rightIntValue.GetSignedValue();
            } else {
                double rightFloatValue = rightValue.GetFloatValue().GetValue();
                rightSValue = reinterpret_cast<int64_t&>(rightFloatValue);
            }

            if (opType == BinaryExpression::OpType::kLogAnd) {
                m_Value = IntValue(leftSValue && rightSValue);
            } else if (opType == BinaryExpression::OpType::kLogOr) {
                m_Value = IntValue(leftSValue || rightSValue);
            }
            return;
        }

        Value rightValue = Accept(*rightOperand);

        // TODO: Simplify
        if (leftValue.IsInteger()) {
            IntValue leftIntValue = leftValue.GetIntValue();
            IntValue rightIntValue = rightValue.GetIntValue();
            if (leftIntValue.IsSigned()) {
                int64_t leftSValue = leftIntValue.GetSignedValue();
                int64_t rightSValue = rightIntValue.GetSignedValue();

                if (opType == BinaryExpression::OpType::kAdd) {
                    m_Value = IntValue(leftSValue + rightSValue);
                } else if (opType == BinaryExpression::OpType::kSub) {
                    m_Value = IntValue(leftSValue - rightSValue);
                } else if (opType == BinaryExpression::OpType::kAnd) {
                    m_Value = IntValue(leftSValue & rightSValue);
                } else if (opType == BinaryExpression::OpType::kXor) {
                    m_Value = IntValue(leftSValue ^ rightSValue);
                } else if (opType == BinaryExpression::OpType::kOr) {
                    m_Value = IntValue(leftSValue | rightSValue);
                } else if (opType == BinaryExpression::OpType::kRem) {
                    m_Value = IntValue(leftSValue % rightSValue);
                } else if (opType == BinaryExpression::OpType::kMul) {
                    m_Value = IntValue(leftSValue * rightSValue);
                } else if (opType == BinaryExpression::OpType::kDiv) {
                    m_Value = IntValue(leftSValue / rightSValue);
                } else if (opType == BinaryExpression::OpType::kShiftL) {
                    m_Value = IntValue(leftSValue << rightSValue);
                } else if (opType == BinaryExpression::OpType::kShiftR) {
                    m_Value = IntValue(leftSValue >> rightSValue);
                }

                if (opType == BinaryExpression::OpType::kLess) {
                    m_Value = IntValue(leftSValue < rightSValue);
                } else if (opType == BinaryExpression::OpType::kGreater) {
                    m_Value = IntValue(leftSValue > rightSValue);
                } else if (opType == BinaryExpression::OpType::kLessEq) {
                    m_Value = IntValue(leftSValue <= rightSValue);
                } else if (opType == BinaryExpression::OpType::kGreaterEq) {
                    m_Value = IntValue(leftSValue >= rightSValue);
                } else if (opType == BinaryExpression::OpType::kEqual) {
                    m_Value = IntValue(leftSValue == rightSValue);
                } else if (opType == BinaryExpression::OpType::kNEqual) {
                    m_Value = IntValue(leftSValue != rightSValue);
                }
            } else {
                uint64_t leftUValue = leftIntValue.GetUnsignedValue();
                uint64_t rightUValue = rightIntValue.GetUnsignedValue();

                if (opType == BinaryExpression::OpType::kAdd) {
                    m_Value = IntValue(leftUValue + rightUValue, /*isSigned=*/false);
                } else if (opType == BinaryExpression::OpType::kSub) {
                    m_Value = IntValue(leftUValue - rightUValue, /*isSigned=*/false);
                } else if (opType == BinaryExpression::OpType::kAnd) {
                    m_Value = IntValue(leftUValue & rightUValue, /*isSigned=*/false);
                } else if (opType == BinaryExpression::OpType::kXor) {
                    m_Value = IntValue(leftUValue ^ rightUValue, /*isSigned=*/false);
                } else if (opType == BinaryExpression::OpType::kOr) {
                    m_Value = IntValue(leftUValue | rightUValue, /*isSigned=*/false);
                } else if (opType == BinaryExpression::OpType::kRem) {
                    m_Value = IntValue(leftUValue % rightUValue, /*isSigned=*/false);
                } else if (opType == BinaryExpression::OpType::kMul) {
                    m_Value = IntValue(leftUValue * rightUValue, /*isSigned=*/false);
                } else if (opType == BinaryExpression::OpType::kDiv) {
                    m_Value = IntValue(leftUValue / rightUValue, /*isSigned=*/false);
                } else if (opType == BinaryExpression::OpType::kShiftL) {
                    m_Value = IntValue(leftUValue << rightUValue, /*isSigned=*/false);
                } else if (opType == BinaryExpression::OpType::kShiftR) {
                    m_Value = IntValue(leftUValue >> rightUValue, /*isSigned=*/false);
                }

                if (opType == BinaryExpression::OpType::kLess) {
                    m_Value = IntValue(leftUValue < rightUValue);
                } else if (opType == BinaryExpression::OpType::kGreater) {
                    m_Value = IntValue(leftUValue > rightUValue);
                } else if (opType == BinaryExpression::OpType::kLessEq) {
                    m_Value = IntValue(leftUValue <= rightUValue);
                } else if (opType == BinaryExpression::OpType::kGreaterEq) {
                    m_Value = IntValue(leftUValue >= rightUValue);
                } else if (opType == BinaryExpression::OpType::kEqual) {
                    m_Value = IntValue(leftUValue == rightUValue);
                } else if (opType == BinaryExpression::OpType::kNEqual) {
                    m_Value = IntValue(leftUValue != rightUValue);
                }
            }
        } else {
            double leftFValue = leftValue.GetFloatValue().GetValue();
            double rightFValue = rightValue.GetFloatValue().GetValue();

            if (opType == BinaryExpression::OpType::kAdd) {
                m_Value = FloatValue(leftFValue + rightFValue);
            } else if (opType == BinaryExpression::OpType::kSub) {
                m_Value = FloatValue(leftFValue - rightFValue);
            } else if (opType == BinaryExpression::OpType::kMul) {
                m_Value = FloatValue(leftFValue * rightFValue);
            } else if (opType == BinaryExpression::OpType::kDiv) {
                m_Value = FloatValue(leftFValue / rightFValue);
            }

            if (opType == BinaryExpression::OpType::kLess) {
                m_Value = IntValue(leftFValue < rightFValue);
            } else if (opType == BinaryExpression::OpType::kGreater) {
                m_Value = IntValue(leftFValue > rightFValue);
            } else if (opType == BinaryExpression::OpType::kLessEq) {
                m_Value = IntValue(leftFValue <= rightFValue);
            } else if (opType == BinaryExpression::OpType::kGreaterEq) {
                m_Value = IntValue(leftFValue >= rightFValue);
            } else if (opType == BinaryExpression::OpType::kEqual) {
                m_Value = IntValue(leftFValue == rightFValue);
            } else if (opType == BinaryExpression::OpType::kNEqual) {
                m_Value = IntValue(leftFValue != rightFValue);
            }
        }
    }

    void Visit(CallExpression& callExpr) override {
        printSemanticError("expression is not a constant expression",
                           callExpr.GetLocation());
    }

    void Visit(CastExpression& castExpr) override {
        Expression* subExpr = castExpr.GetSubExpression();
        Value exprValue = Accept(*subExpr);

        QualType toQualType = castExpr.GetToType();
        Type* toType = toQualType.GetSubType();

        auto* builtinType = dynamic_cast<BuiltinType*>(toType);
        if (builtinType) {
            if (builtinType->IsSignedInteger()) {
                if (exprValue.IsInteger()) {
                    IntValue value = exprValue.GetIntValue();
                    if (!value.IsSigned()) {
                        m_Value = IntValue(value.GetUnsignedValue());
                    }
                } else {
                    m_Value = IntValue(exprValue.GetFloatValue().GetValue());
                }
            } else if (builtinType->IsUnsignedInteger()) {
                if (exprValue.IsInteger()) {
                    IntValue value = exprValue.GetIntValue();
                    if (value.IsSigned()) {
                        m_Value = IntValue(value.GetSignedValue(), /*isSigned=*/false);
                    }
                } else {
                    m_Value = IntValue(exprValue.GetFloatValue().GetValue(), /*isSigned=*/false);
                }
            } else {
                if (exprValue.IsInteger()) {
                    IntValue value = exprValue.GetIntValue();
                    if (value.IsSigned()) {
                        if (builtinType->IsDoublePrecision()) {
                            double doubleValue = value.GetSignedValue();
                            m_Value = FloatValue(doubleValue);
                        } else {
                            float floatValue = value.GetSignedValue();
                            m_Value = FloatValue(floatValue);
                        }
                    } else {
                        if (builtinType->IsDoublePrecision()) {
                            double doubleValue = value.GetUnsignedValue();
                            m_Value = FloatValue(doubleValue);
                        } else {
                            float floatValue = value.GetUnsignedValue();
                            m_Value = FloatValue(floatValue);
                        }
                    }
                }
            }

            return;
        }
    }

    void Visit(CharExpression& charExpr) override {
        m_Value = IntValue(charExpr.GetCharValue());
    }

    void Visit(ConditionalExpression& condExpr) override {
        Expression* condition = condExpr.GetCondition();
        Value condValue = Accept(*condition);

        int64_t condSValue = 0;
        if (condValue.IsInteger()) {
            IntValue condIntValue = condValue.GetIntValue();
            condSValue = condIntValue.GetSignedValue();
        } else {
            double condFloatValue = condValue.GetFloatValue().GetValue();
            condSValue = reinterpret_cast<int64_t&>(condFloatValue);
        }

        if (condSValue) {
            Expression* trueExpr = condExpr.GetTrueExpression();
            m_Value = Accept(*trueExpr);
        } else {
            Expression* falseExpr = condExpr.GetFalseExpression();
            m_Value = Accept(*falseExpr);  
        }
    }

    void Visit(ConstExpression& constExpr) override {
        m_Value = Accept(*constExpr.GetExpression());
        constExpr.SetValue(m_Value);
    }

    void Visit(DeclRefExpression& declrefExpr) override {
        ValueDeclaration* decl = declrefExpr.GetDeclaration();
        if (auto* enumConstDecl = dynamic_cast<EnumConstDeclaration*>(decl)) {
            ConstExpression* init = enumConstDecl->GetInit();
            assert(init->IsEvaluated());
            m_Value = init->GetValue();
            return;
        }

        printSemanticError("expression is not an integer constant expression",
                           declrefExpr.GetLocation());
    }

    // Skip?
    void Visit(ExpressionList& exprList) override {}

    void Visit(FloatExpression& floatExpr) override {
        m_Value = floatExpr.GetFloatValue();
    }

    // Skip?
    void Visit(InitializerList& initList) override {}

    void Visit(IntExpression& intExpr) override {
        m_Value = intExpr.GetIntValue();
    }

    void Visit(SizeofTypeExpression& sizeofTypeExpr) override {
        QualType qualType = sizeofTypeExpr.GetType();
        Type* type = qualType.GetSubType();
        // TODO: type byte size
    }

    void Visit(StringExpression& stringExpr) override {
        printSemanticError("integer constant expression must have integer type",
                           stringExpr.GetLocation());
    }

    void Visit(UnaryExpression& unaryExpr) override {
        UnaryExpression::OpType opType = unaryExpr.GetOpType();
        switch (opType) {
        case UnaryExpression::OpType::kPreInc:
        case UnaryExpression::OpType::kPreDec:
        case UnaryExpression::OpType::kPostInc:
        case UnaryExpression::OpType::kPostDec:
        case UnaryExpression::OpType::kAddrOf:
        case UnaryExpression::OpType::kDeref:
            printSemanticError("expression is not a constant expression",
                               unaryExpr.GetLocation());
        default:
            break;
        }

        Expression* operand = unaryExpr.GetOperand();
        if (opType == UnaryExpression::OpType::kSizeof) {
            QualType qualType = operand->GetType();
            Type* type = qualType.GetSubType();
            // TODO: type byte size
            return;
        }

        Value operandValue = Accept(*operand);

        if (opType == UnaryExpression::OpType::kPlus) {
            return;
        }

        if (operandValue.IsInteger()) {
            IntValue value = operandValue.GetIntValue();
            if (value.IsSigned()) {
                int64_t signedValue = value.GetSignedValue();
                if (opType == UnaryExpression::OpType::kMinus) {
                    m_Value = IntValue(-signedValue);
                } else if (opType == UnaryExpression::OpType::kNot) {
                    m_Value = IntValue(~signedValue);
                } else if (opType == UnaryExpression::OpType::kLogNot) {
                    m_Value = IntValue(!signedValue);
                }
            } else {
                uint64_t unsignedValue = value.GetUnsignedValue();
                if (opType == UnaryExpression::OpType::kMinus) {
                    m_Value = IntValue(-unsignedValue, /*isSigned=*/false);
                } else if (opType == UnaryExpression::OpType::kNot) {
                    m_Value = IntValue(~unsignedValue, /*isSigned=*/false);
                } else if (opType == UnaryExpression::OpType::kLogNot) {
                    m_Value = IntValue(!unsignedValue);
                }
            }
        } else {
            double floatValue = operandValue.GetFloatValue().GetValue();
            if (opType == UnaryExpression::OpType::kMinus) {
                m_Value = FloatValue(-floatValue);
            } else if (opType == UnaryExpression::OpType::kLogNot) {
                m_Value = IntValue(!floatValue);
            }
        }
    }


    /*
    =================================================================
                                Type
    =================================================================
    */

    // Skip
    void Visit(ArrayType& arrayType) override {}
    void Visit(BuiltinType& builtinType) override {}
    void Visit(EnumType& enumType) override {}
    void Visit(FunctionType& funcType) override {}
    void Visit(PointerType& ptrType) override {}
    void Visit(RecordType& recordType) override {}
    void Visit(TagType&) override {}
    void Visit(Type&) override {}
    void Visit(TypedefType& typedefType) override {}

private:
    void printSemanticWarning(const std::string& text, const Location& location) {
        ANCL_WARN("{} {}", location.ToString(), text);
    }

    void printSemanticError(const std::string& text, const Location& location) {
        ANCL_ERROR("{} {}", location.ToString(), text);

        // TODO: Handle error
        throw std::runtime_error("Semantic error");
        // exit(EXIT_FAILURE);
    }

private:
    Value m_Value = IntValue(0);
};

}  // namespace ast
