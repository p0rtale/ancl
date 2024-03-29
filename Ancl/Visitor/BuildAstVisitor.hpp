#include <format>

#include "antlr4-runtime.h"
#include "CParserBaseVisitor.h"

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Grammar/AST/Program.hpp>


using namespace ast;

namespace anclgrammar {

class BuildAstVisitor: public CParserBaseVisitor {
public:
    BuildAstVisitor(Program& program)
            : m_Program(program) {
        auto unit = m_Program.CreateAstNode<TranslationUnit>();
        // IntExpression* expr = m_Program.CreateAstNode<IntExpression>(IntValue(10));
    }

public:
    std::any visitPrimaryExpression(CParser::PrimaryExpressionContext *ctx) override {
        if (ctx->Identifier()) {
            auto name = ctx->Identifier()->getText();
            // auto identDeclOpt = m_CurrentScope->FindSymbol(Scope::NamespaceType::Ident, symbol);
            // if (!identDeclOpt) {
            //     // TODO: handle error
            // }

            // auto identDecl = *identDeclOpt;
            // if (identDecl->IsTypeDecl()) {
            //     // TODO: handle error
            // }

            // auto identValDecl = static_cast<ValueDeclaration*>(identDecl); 

            auto valueDecl = m_Program.CreateAstNode<ValueDeclaration>(std::move(name));           
            auto declExpression = m_Program.CreateAstNode<DeclRefExpression>(valueDecl);
            return static_cast<Expression*>(declExpression);
        }
        if (ctx->numberConstant()) {
            auto numberAny = visitNumberConstant(ctx->numberConstant());
            auto number = std::any_cast<Expression*>(numberAny);
            return number;
        }
        if (ctx->expression()) {
            return visitExpression(ctx->expression());
        }

        auto stringLiterals = ctx->StringLiteral();
        // TODO: handle multiple string literals
        std::string literalWithQuotes = stringLiterals[0]->getText();
        std::string literal = literalWithQuotes.substr(1, literalWithQuotes.size() - 2);
        auto stringExpr = m_Program.CreateAstNode<StringExpression>(std::move(literal));
        return stringExpr;
    }

    std::any visitNumberConstant(CParser::NumberConstantContext *ctx) override {
        std::string str = ctx->getText();
        if (ctx->IntegerConstant()) {
            errno = 0;
            char* endPtr = nullptr;
            const char* integerPtr = str.c_str();
            long integer = std::strtol(integerPtr, &endPtr, 0);
            if (integerPtr == endPtr) {
                throw std::runtime_error(std::format("invalid integer constant \"{}\"", str));
            }
            if (errno == ERANGE) {
                throw std::runtime_error(std::format("integer constant \"{}\" is out of range", str));
            }
            if (errno != 0 && integer == 0) {
                throw std::runtime_error(std::format("invalid integer constant \"{}\"", str));
            }
            if (errno == 0 && *endPtr == 0) {
                // TODO: handle suffix
                Expression* intExpr = m_Program.CreateAstNode<IntExpression>(IntValue(integer));
                return intExpr;
            }
            throw std::runtime_error(std::format("invalid integer constant \"{}\"", str));
        } else if (ctx->FloatingConstant()) {
            errno = 0;
            char* endPtr = nullptr;
            const char* realPtr = str.c_str();
            double real = std::strtod(realPtr, &endPtr);
            if (realPtr == endPtr) {
                throw std::runtime_error(std::format("invalid real number constant \"{}\"", str));
            }
            if (errno == ERANGE) {
                throw std::runtime_error(std::format("real number constant \"{}\" is out of range", str));
            }
            if (errno != 0 && real == 0) {
                throw std::runtime_error(std::format("invalid real number constant \"{}\"", str));
            }
            if (errno == 0 && *endPtr == 0) {
                // TODO: handle suffix
                Expression* floatExpr = m_Program.CreateAstNode<FloatExpression>(FloatValue(real));
                return floatExpr;
            }
            throw std::runtime_error(std::format("invalid real number constant \"{}\"", str));
        } else if (ctx->CharacterConstant()) {
            // TODO: handle multichar
            Expression* charExpr = m_Program.CreateAstNode<CharExpression>(str[1]);
            return charExpr;
        }
        return nullptr;
    }

    std::any visitEnumerationConstant(CParser::EnumerationConstantContext *ctx) override {
        // TODO: Declaration
        return visitChildren(ctx);
    }

    std::any visitPostfixExpression(CParser::PostfixExpressionContext *ctx) override {
        if (ctx->primaryExpression()) {
            return visitPrimaryExpression(ctx->primaryExpression());
        }

        auto leftExpressionAny = visitPostfixExpression(ctx->postfixExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        if (ctx->callee) {
            auto argsExpression = std::vector<Expression*>{};
            if (ctx->args) {
                auto argsExpressionAny = visitArgumentExpressionList(ctx->args);
                std::cout << "ABOBA?\n";
                argsExpression = std::any_cast<std::vector<Expression*>>(argsExpressionAny);
                std::cout << "KEEEEK\n";
            }
            auto callExpr = m_Program.CreateAstNode<CallExpression>(
                leftExpression, argsExpression
            );
            return static_cast<Expression*>(callExpr);      
        }

        if (ctx->array) {
            auto indexExpression = std::any_cast<Expression*>(visitExpression(ctx->index));
            auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
                leftExpression, indexExpression,
                BinaryExpression::OpType::kArrSubscript
            );
            return static_cast<Expression*>(binaryExpr);
        }

        if (ctx->tag) {
            auto name = ctx->Identifier()->getText();
            auto memberDecl = m_Program.CreateAstNode<FieldDeclaration>(std::move(name));
            auto memberExpression = m_Program.CreateAstNode<DeclRefExpression>(memberDecl);

            auto op = BinaryExpression::OpType::kNone;
            if (ctx->member->getText() == ".") {
                op = BinaryExpression::OpType::kDirectMember;
            } else if (ctx->member->getText() == "->") {
                op = BinaryExpression::OpType::kArrowMember;
            }
    
            auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
                leftExpression, memberExpression, op
            );
            return static_cast<Expression*>(binaryExpr);
        }

        auto op = UnaryExpression::OpType::kNone;
        if (ctx->inc) {
            op = UnaryExpression::OpType::kPostInc;
        } else if (ctx->dec) {
            op = UnaryExpression::OpType::kPostDec;
        }

        auto unaryExpr = m_Program.CreateAstNode<UnaryExpression>(
            leftExpression, op
        );
        return static_cast<Expression*>(unaryExpr);   
    }

    std::any visitArgumentExpressionList(CParser::ArgumentExpressionListContext *ctx) override {
        auto firstArgAny = visitAssignmentExpression(ctx->expr);
        auto argsExpression = std::vector<Expression*>{std::any_cast<Expression*>(firstArgAny)};
        argsExpression.reserve(ctx->exprTail.size() + 1);

        for (auto argCtx : ctx->exprTail) {
            auto argExpressionAny = visitAssignmentExpression(argCtx);
            auto argExpression = std::any_cast<Expression*>(argExpressionAny);
            argsExpression.push_back(argExpression);
        }

        return argsExpression;
    }

    std::any visitUnaryExpression(CParser::UnaryExpressionContext *ctx) override {
        if (ctx->unaryExpressionTail()) {
            return visitUnaryExpressionTail(ctx->unaryExpressionTail());
        } 

        auto expressionAny = visitUnaryExpression(ctx->unaryExpression());
        auto expression = std::any_cast<Expression*>(expressionAny);

        auto op = UnaryExpression::OpType::kNone;
        if (ctx->inc) {
            op = UnaryExpression::OpType::kPreInc;
        } else if (ctx->dec) {
            op = UnaryExpression::OpType::kPreDec;
        } else if (ctx->size) {
            op = UnaryExpression::OpType::kSizeof;
        }

        auto unaryExpr = m_Program.CreateAstNode<UnaryExpression>(
            expression, op
        );
        return static_cast<Expression*>(unaryExpr);
    }

    std::any visitUnaryExpressionTail(CParser::UnaryExpressionTailContext *ctx) override {
        if (ctx->postfixExpression()) {
            return visitPostfixExpression(ctx->postfixExpression());
        }

        if (ctx->unaryOperator()) {
            auto expressionAny = visitCastExpression(ctx->castExpression());
            auto expression = std::any_cast<Expression*>(expressionAny);
            auto opStr = ctx->unaryOperator()->getText();
            auto op = UnaryExpression::OpType::kNone;
            if (opStr == "&") {
                op = UnaryExpression::OpType::kAddrOf;
            } else if (opStr == "*") {
                op = UnaryExpression::OpType::kDeref;
            } else if (opStr == "+") {
                op = UnaryExpression::OpType::kPlus;
            } else if (opStr == "-") {
                op = UnaryExpression::OpType::kMinus;
            } else if (opStr == "~") {
                op = UnaryExpression::OpType::kNot;
            } else if (opStr == "!") {
                op = UnaryExpression::OpType::kLogNot;
            }

            auto unaryExpr = m_Program.CreateAstNode<UnaryExpression>(
                expression, op
            );
            return static_cast<Expression*>(unaryExpr);
        }

        auto typeNameAny = visitTypeName(ctx->typeName());
        auto typeName = std::any_cast<QualType*>(typeNameAny);

        auto sizeofExpr = m_Program.CreateAstNode<SizeofTypeExpression>(typeName);
        return static_cast<Expression*>(sizeofExpr);
    }

    // Skip
    std::any visitUnaryOperator(CParser::UnaryOperatorContext *ctx) override {
        return nullptr;
    }

    std::any visitCastExpression(CParser::CastExpressionContext *ctx) override {
        if (ctx->unaryExpression()) {
            return visitUnaryExpression(ctx->unaryExpression());
        }

        auto typeNameAny = visitTypeName(ctx->typeName());
        auto typeName = std::any_cast<QualType*>(typeNameAny);

        auto subExpressionAny = visitCastExpression(ctx->castExpression());
        auto subExpression = std::any_cast<Expression*>(subExpressionAny);

        auto castExpr = m_Program.CreateAstNode<CastExpression>(subExpression, typeName);
        return static_cast<Expression*>(castExpr);
    }

    std::any visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext *ctx) override {
        if (ctx->castexpr) {
            return visitCastExpression(ctx->castexpr);
        }

        auto leftExpressionAny = visitCastExpression(ctx->castExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitMultiplicativeExpression(ctx->multiplicativeExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto op = BinaryExpression::OpType::kNone;
        if (ctx->mul) {
            op = BinaryExpression::OpType::kMul;
        } else if (ctx->div) {
            op = BinaryExpression::OpType::kDiv;
        } else if (ctx->rem) {
            op = BinaryExpression::OpType::kRem;
        }

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, op
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitAdditiveExpression(CParser::AdditiveExpressionContext *ctx) override {
        if (ctx->mulexpr) {
            return visitMultiplicativeExpression(ctx->mulexpr);
        }

        auto leftExpressionAny = visitMultiplicativeExpression(ctx->multiplicativeExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitAdditiveExpression(ctx->additiveExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto op = BinaryExpression::OpType::kNone;
        if (ctx->add) {
            op = BinaryExpression::OpType::kAdd;
        } else if (ctx->sub) {
            op = BinaryExpression::OpType::kSub;
        }

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, op
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitShiftExpression(CParser::ShiftExpressionContext *ctx) override {
        if (ctx->addexpr) {
            return visitAdditiveExpression(ctx->addexpr);
        }

        auto leftExpressionAny = visitAdditiveExpression(ctx->additiveExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitShiftExpression(ctx->shiftExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto op = BinaryExpression::OpType::kNone;
        if (ctx->shiftl) {
            op = BinaryExpression::OpType::kShiftL;
        } else if (ctx->shiftr) {
            op = BinaryExpression::OpType::kShiftR;
        }

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, op
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitRelationalExpression(CParser::RelationalExpressionContext *ctx) override {
        if (ctx->shiftexpr) {
            return visitShiftExpression(ctx->shiftexpr);
        }

        auto leftExpressionAny = visitShiftExpression(ctx->shiftExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitRelationalExpression(ctx->relationalExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto op = BinaryExpression::OpType::kNone;
        if (ctx->less) {
            op = BinaryExpression::OpType::kLess;
        } else if (ctx->greater) {
            op = BinaryExpression::OpType::kGreater;
        } else if (ctx->lesseq) {
            op = BinaryExpression::OpType::kLessEq;
        } else if (ctx->greatereq) {
            op = BinaryExpression::OpType::kGreaterEq;
        }

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, op
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitEqualityExpression(CParser::EqualityExpressionContext *ctx) override {
        if (ctx->relexpr) {
            return visitRelationalExpression(ctx->relexpr);
        }

        auto leftExpressionAny = visitRelationalExpression(ctx->relationalExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitEqualityExpression(ctx->equalityExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto op = BinaryExpression::OpType::kNone;
        if (ctx->equal) {
            op = BinaryExpression::OpType::kEqual;
        } else if (ctx->nequal) {
            op = BinaryExpression::OpType::kNEqual;
        }

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, op
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitAndExpression(CParser::AndExpressionContext *ctx) override {
        if (ctx->eqexpr) {
            return visitEqualityExpression(ctx->eqexpr);
        }

        auto leftExpressionAny = visitEqualityExpression(ctx->equalityExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitAndExpression(ctx->andExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kAnd
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitExclusiveOrExpression(CParser::ExclusiveOrExpressionContext *ctx) override {
        if (ctx->andexpr) {
            return visitAndExpression(ctx->andexpr);
        }

        auto leftExpressionAny = visitAndExpression(ctx->andExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitAndExpression(ctx->andExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kXor
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitInclusiveOrExpression(CParser::InclusiveOrExpressionContext *ctx) override {
        if (ctx->exorexpr) {
            return visitExclusiveOrExpression(ctx->exorexpr);
        }

        auto leftExpressionAny = visitExclusiveOrExpression(ctx->exclusiveOrExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitInclusiveOrExpression(ctx->inclusiveOrExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kOr
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitLogicalAndExpression(CParser::LogicalAndExpressionContext *ctx) override {
        if (ctx->incorexpr) {
            return visitInclusiveOrExpression(ctx->incorexpr);
        }

        auto leftExpressionAny = visitInclusiveOrExpression(ctx->inclusiveOrExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitLogicalAndExpression(ctx->logicalAndExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kLogAnd
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitLogicalOrExpression(CParser::LogicalOrExpressionContext *ctx) override {
        if (ctx->logandexpr) {
            return visitLogicalAndExpression(ctx->logandexpr);
        }

        auto leftExpressionAny = visitLogicalAndExpression(ctx->logicalAndExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitLogicalOrExpression(ctx->logicalOrExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kLogOr
        );
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitConditionalExpression(CParser::ConditionalExpressionContext *ctx) override {
        if (ctx->logorexpr) {
            return visitLogicalOrExpression(ctx->logorexpr);
        }

        auto conditionAny = visitLogicalOrExpression(ctx->logicalOrExpression());
        auto condition = std::any_cast<Expression*>(conditionAny);

        auto trueExpressionAny = visitExpression(ctx->expression());
        auto trueExpression = std::any_cast<Expression*>(trueExpressionAny);

        auto falseExpressionAny = visitConditionalExpression(ctx->conditionalExpression());
        auto falseExpression = std::any_cast<Expression*>(falseExpressionAny);

        auto condExpr = m_Program.CreateAstNode<ConditionalExpression>(
            condition, trueExpression, falseExpression
        );
        return static_cast<Expression*>(condExpr);
    }

    std::any visitAssignmentExpression(CParser::AssignmentExpressionContext *ctx) override {
        if (ctx->condexpr) {
            return visitConditionalExpression(ctx->condexpr);
        }

        auto leftExpressionAny = visitUnaryExpression(ctx->unaryExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitAssignmentExpression(ctx->assignmentExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto opStr = ctx->assignmentOperator()->getText();
        auto op = BinaryExpression::OpType::kNone;
        if (opStr == "=") {
            op = BinaryExpression::OpType::kAssign;
        } else if (opStr == "*=") {
            op = BinaryExpression::OpType::kMulAssign;
        } else if (opStr == "/=") {
            op = BinaryExpression::OpType::kDivAssign;
        } else if (opStr == "%=") {
            op = BinaryExpression::OpType::kRemAssign;
        } else if (opStr == "+=") {
            op = BinaryExpression::OpType::kAddAssign;
        } else if (opStr == "-=") {
            op = BinaryExpression::OpType::kSubAssign;
        } else if (opStr == "<<=") {
            op = BinaryExpression::OpType::kShiftLAssign;
        } else if (opStr == ">>=") {
            op = BinaryExpression::OpType::kShiftRAssign;
        } else if (opStr == "&=") {
            op = BinaryExpression::OpType::kAndAssign;
        } else if (opStr == "^=") {
            op = BinaryExpression::OpType::kXorAssign;
        } else if (opStr == "|=") {
            op = BinaryExpression::OpType::kOrAssign;
        }

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, op
        );
        return static_cast<Expression*>(binaryExpr);
    }

    // Skip
    std::any visitAssignmentOperator(CParser::AssignmentOperatorContext *ctx) override {
        return nullptr;
    }

    std::any visitExpression(CParser::ExpressionContext *ctx) override {
        auto exprList = m_Program.CreateAstNode<ExpressionList>();

        auto firstExpressionAny = visitAssignmentExpression(ctx->expr);
        auto firstExpression = std::any_cast<Expression*>(firstExpressionAny);
        exprList->AddExpression(firstExpression);

        for (auto exprCtx : ctx->exprTail) {
            auto expressionAny = visitAssignmentExpression(exprCtx);
            auto expression = std::any_cast<Expression*>(expressionAny);
            exprList->AddExpression(expression);
        }

        return static_cast<Expression*>(exprList);
    }

    std::any visitConstantExpression(CParser::ConstantExpressionContext *ctx) override {
        auto expressionAny = visitConditionalExpression(ctx->conditionalExpression());
        auto expression = std::any_cast<Expression*>(expressionAny);

        auto constExpr = m_Program.CreateAstNode<ConstExpression>(expression);
        return constExpr;
    }

    std::any visitDeclaration(CParser::DeclarationContext *ctx) override {
        // DeclarationSpecifiers:
        // 1) Declaration (struct, union, enum) -> add and wrap with var declaration
        // 2) Type -> just wrap with declaration with declarator

        std::vector<Declaration*> decls;
        auto declSpecsAny = visitDeclarationSpecifiers(ctx->declspecs);

        QualType* qualType = nullptr;
        auto declPtr = std::any_cast<Declaration*>(&declSpecsAny);
        if (declPtr) {  // is declaration
            auto decl = *declPtr;
            decls.push_back(decl);
            auto typeDecl = static_cast<TypeDeclaration*>(decl);
            qualType = typeDecl->GetType();
        } else {  // is type
            qualType = std::any_cast<QualType*>(declSpecsAny);
        }

        if (ctx->initdecl) {
            auto declListAny = visitInitDeclaratorList(ctx->initdecl);
            auto declList = std::any_cast<std::vector<Declaration*>>(declListAny);
        }

        return decls;
    }

public:
    enum class StorageClass {
        kNone = 0,
        kTypedef,
        kExtern,
        kStatic,
        kAuto,
        kRegister,
    };

    enum class BuiltinTypeSpecifier {
        kVoid = 0,
        kChar,
        kShort,
        kInt,
        kLong,
        kFloat,
        kDouble,
        kSigned,
        kUnsigned,
    };

    enum class Qualifier {
        kNone = 0,
        kConst,
        kRestrict,
        kVolatile,
    };

    enum class FunctionSpecifier {
        kNone = 0,
        kInline,
    };

    struct DeclSpecifiers {
        StorageClass Storage = StorageClass::kNone;
        FunctionSpecifier FunSpec = FunctionSpecifier::kNone;
        QualType* Type = nullptr;
    };

public:
    std::any visitDeclarationSpecifiers(CParser::DeclarationSpecifiersContext *ctx) override {
        auto declSpecs = DeclSpecifiers{};

        auto qualType = m_Program.CreateType<QualType>();
        declSpecs.Type = qualType;

        for (auto specCtx : ctx->specs) {
            auto declSpecAny = visitDeclarationSpecifier(specCtx);

            auto storageClassPtr = std::any_cast<StorageClass>(&declSpecAny);
            if (storageClassPtr) {
                auto storageClass = *storageClassPtr;

                if (storageClass == StorageClass::kTypedef) {
                    // TODO: check typedef
                } else {
                    if (storageClass != StorageClass::kNone &&
                            declSpecs.Storage != StorageClass::kNone) {
                        // TODO: handle static + extern, static + static, ... errors
                    }

                    declSpecs.Storage = storageClass;
                }
            }

            auto builtinTypeSpecPtr = std::any_cast<BuiltinTypeSpecifier>(&declSpecAny);
            if (builtinTypeSpecPtr) {
                auto builtinTypeSpec = *builtinTypeSpecPtr;

                auto builtinType = m_Program.CreateType<BuiltinType>();

                switch (builtinTypeSpec) {
                case BuiltinTypeSpecifier::kVoid:
                    builtinType->SetKind(BuiltinType::Kind::kVoid);
                    break;
                case BuiltinTypeSpecifier::kChar:
                    builtinType->SetKind(BuiltinType::Kind::kChar);
                    break;
                case BuiltinTypeSpecifier::kShort:
                    builtinType->SetKind(BuiltinType::Kind::kShort);
                    break;
                case BuiltinTypeSpecifier::kInt:
                    builtinType->SetKind(BuiltinType::Kind::kInt);
                    break;
                case BuiltinTypeSpecifier::kLong:
                    builtinType->SetKind(BuiltinType::Kind::kLong);
                    break;
                case BuiltinTypeSpecifier::kFloat:
                    builtinType->SetKind(BuiltinType::Kind::kFloat);
                    break;
                case BuiltinTypeSpecifier::kDouble:
                    builtinType->SetKind(BuiltinType::Kind::kDouble);
                    break;
                case BuiltinTypeSpecifier::kSigned:
                    builtinType->SetKind(BuiltinType::Kind::kInt);
                    break;
                case BuiltinTypeSpecifier::kUnsigned:
                    if (builtinType->GetKind() == BuiltinType::Kind::kChar) {
                        builtinType->SetKind(BuiltinType::Kind::kUChar);
                    } else if (builtinType->GetKind() == BuiltinType::Kind::kInt) {
                        builtinType->SetKind(BuiltinType::Kind::kUInt);
                    } else if (builtinType->GetKind() == BuiltinType::Kind::kShort) {
                        builtinType->SetKind(BuiltinType::Kind::kUShort);
                    } else if (builtinType->GetKind() == BuiltinType::Kind::kLong) {
                        builtinType->SetKind(BuiltinType::Kind::kULong);
                    }
                    break;
                default:
                    // TODO: handle error
                    break;
                }
            }

            auto namedTypeSpecPtr = std::any_cast<Type*>(&declSpecAny);
            if (namedTypeSpecPtr) {
                auto namedType = *namedTypeSpecPtr;
                auto qualType = declSpecs.Type;
                qualType->SetSubType(namedType);
            }

            auto qualifierPtr = std::any_cast<Qualifier>(&declSpecAny);
            if (qualifierPtr) {
                auto qualifier = *qualifierPtr;
                auto declSpecsQualType = declSpecs.Type;
    
                if (qualifier == Qualifier::kConst) {
                    if (declSpecsQualType->IsConst()) {
                        // TODO: handle error
                    }
                    declSpecsQualType->AddConst();
                }

                if (qualifier == Qualifier::kRestrict) {
                    if (declSpecsQualType->IsRestrict()) {
                        // TODO: handle error
                    }
                    declSpecsQualType->AddRestrict();
                }

                if (qualifier == Qualifier::kVolatile) {
                    if (declSpecsQualType->IsVolatile()) {
                        // TODO: handle error
                    }
                    declSpecsQualType->AddVolatile();
                }
            }

            auto functionSpecPtr = std::any_cast<FunctionSpecifier>(&declSpecAny);
            if (functionSpecPtr) {
                declSpecs.FunSpec = *functionSpecPtr;
            }
        }

        return declSpecs;
    }

    std::any visitDeclarationSpecifiers2(CParser::DeclarationSpecifiers2Context *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitDeclarationSpecifier(CParser::DeclarationSpecifierContext *ctx) override {
        if (ctx->storage) {
            auto storageClassAny = visitStorageClassSpecifier(ctx->storage);
            auto storageClass = std::any_cast<StorageClass>(storageClassAny);
            return storageClass;
        }

        if (ctx->typespec) {
            auto typeSpecifierAny = visitTypeSpecifier(ctx->typespec);
            // Type(RecordType | EnumType | TypedefType) | BuiltinTypeSpecifier

            auto builtinTypeSpecPtr = std::any_cast<BuiltinTypeSpecifier>(&typeSpecifierAny);
            if (builtinTypeSpecPtr) {
                return *builtinTypeSpecPtr;
            }

            auto namedTypeSpecifierPtr = std::any_cast<Type*>(&typeSpecifierAny);
            return *namedTypeSpecifierPtr;
        }

        if (ctx->qualifier) {
            auto typeQualifierAny = visitTypeQualifier(ctx->qualifier);
            auto typeQualifier = std::any_cast<Qualifier>(typeQualifierAny);
        }

        if (ctx->funcspec) {
            auto funcSpecifierAny = visitFunctionSpecifier(ctx->funcspec);
            auto funcSpecifier = std::any_cast<FunctionSpecifier>(funcSpecifierAny);
            return funcSpecifier;
        }

        return nullptr;
    }

    std::any visitInitDeclaratorList(CParser::InitDeclaratorListContext *ctx) override {
        auto firstDeclAny = visitInitDeclarator(ctx->init);
        auto firstDecl = std::any_cast<Declaration*>(firstDeclAny);
        std::vector<Declaration*> decls{firstDecl};

        decls.reserve(ctx->initTail.size() + 1);
        for (auto declCtx : ctx->initTail) {
            auto declAny = visitInitDeclarator(declCtx);
            auto decl = std::any_cast<Declaration*>(declAny);
            decls.push_back(decl);
        }

        return decls;
    }

    std::any visitInitDeclarator(CParser::InitDeclaratorContext *ctx) override {
        auto declAny = visitDeclarator(ctx->decl);
        auto decl = std::any_cast<Declaration*>(declAny);

        Expression* init = nullptr;
        if (ctx->init) {
            auto initAny = visitInitializer(ctx->init);
            init = std::any_cast<Expression*>(initAny);

            auto varDecl = dynamic_cast<VariableDeclaration*>(decl);
            if (!varDecl) {
                // TODO: handle error
                return nullptr;
            }
            varDecl->SetInit(init);
        }

        return decl;
    }

public:
    std::any visitStorageClassSpecifier(CParser::StorageClassSpecifierContext *ctx) override {
        auto storageStr = ctx->getText();
        if (storageStr == "typedef") {
            return StorageClass::kTypedef;
        }

        if (storageStr == "extern") {
            return StorageClass::kExtern;
        }
        if (storageStr == "static") {
            return StorageClass::kStatic;
        }
        if (storageStr == "auto") {
            return StorageClass::kAuto;
        }
        if (storageStr == "register") {
            return StorageClass::kRegister;
        }
        return StorageClass::kNone;
    }

public:
    std::any visitTypeSpecifier(CParser::TypeSpecifierContext *ctx) override {
        if (ctx->recordspec) {
            auto recordTypeAny = visitStructOrUnionSpecifier(ctx->recordspec);
            auto recordType = std::any_cast<RecordType*>(recordTypeAny);
            return static_cast<Type*>(recordType);
        }

        if (ctx->enumspec) {
            auto enumTypeAny = visitEnumSpecifier(ctx->enumspec);
            auto enumType = std::any_cast<RecordType*>(enumTypeAny);
            return static_cast<Type*>(enumType);
        }

        if (ctx->typedefname) {
            auto name = ctx->typedefname->getText();
            auto typedefDecl = m_Program.CreateAstNode<TypedefDeclaration>();
            typedefDecl->SetName(name);
            auto typedefType = m_Program.CreateType<TypedefType>(typedefDecl);
            return static_cast<Type*>(typedefType);
        }

        auto typeStr = ctx->type->getText();
        if (typeStr == "void") {
            return BuiltinTypeSpecifier::kVoid;
        }
        if (typeStr == "char") {
            return BuiltinTypeSpecifier::kChar;
        }
        if (typeStr == "short") {
            return BuiltinTypeSpecifier::kShort;
        }
        if (typeStr == "int") {
            return BuiltinTypeSpecifier::kInt;
        }
        if (typeStr == "long") {
            return BuiltinTypeSpecifier::kLong;
        }
        if (typeStr == "float") {
            return BuiltinTypeSpecifier::kFloat;
        }
        if (typeStr == "double") {
            return BuiltinTypeSpecifier::kDouble;
        }
        if (typeStr == "signed") {
            return BuiltinTypeSpecifier::kSigned;
        }
        if (typeStr == "unsigned") {
            return BuiltinTypeSpecifier::kUnsigned;
        }
        return BuiltinTypeSpecifier::kVoid;
    }

    std::any visitStructOrUnionSpecifier(CParser::StructOrUnionSpecifierContext *ctx) override {
        bool isUnion = false;
        auto structOrUnionCtx = ctx->structOrUnion();
        if (structOrUnionCtx->getText() == "union") {
            isUnion = true;
        }

        std::string name;
        auto ident = ctx->Identifier();
        if (ident) {
            name = ident->getText();
        }

        std::vector<FieldDeclaration*> fields;
        if (ctx->body) {
            auto fieldsAny = visitStructDeclarationList(ctx->body);
            fields = std::any_cast<std::vector<FieldDeclaration*>>(fieldsAny);
        }

        auto recordDecl = m_Program.CreateAstNode<RecordDeclaration>(std::move(fields), isUnion);
        auto recordType = m_Program.CreateType<RecordType>(recordDecl);
        auto recordQualType = m_Program.CreateType<QualType>(recordType);
        recordDecl->SetName(name);
        recordDecl->SetType(recordQualType);

        // if (ctx->body) {
        //     return recordDecl;
        // }

        return recordType;
    }

    std::any visitStructOrUnion(CParser::StructOrUnionContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitStructDeclarationList(CParser::StructDeclarationListContext *ctx) override {
        std::vector<FieldDeclaration*> fields;
        for (auto declCtx : ctx->decls) {
            auto fieldAny = visitStructDeclaration(declCtx);
            auto field = std::any_cast<FieldDeclaration*>(fieldAny);
            fields.push_back(field);
        }
        return fields;
    }

    std::any visitStructDeclaration(CParser::StructDeclarationContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitSpecifierQualifierList(CParser::SpecifierQualifierListContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitStructDeclaratorList(CParser::StructDeclaratorListContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitStructDeclarator(CParser::StructDeclaratorContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitEnumSpecifier(CParser::EnumSpecifierContext *ctx) override {
        std::string name;
        auto ident = ctx->Identifier();
        if (ident) {
            name = ident->getText();
        }

        std::vector<EnumConstDeclaration*> enumerators;
        if (ctx->enumerators) {
            auto enumeratorsAny = visitEnumeratorList(ctx->enumerators);
            enumerators = std::any_cast<std::vector<EnumConstDeclaration*>>(enumeratorsAny);
        }

        auto enumDecl = m_Program.CreateAstNode<EnumDeclaration>(std::move(enumerators));
        auto enumType = m_Program.CreateType<EnumType>(enumDecl);
        auto enumQualType = m_Program.CreateType<QualType>(enumType);
        enumDecl->SetType(enumQualType);

        // if (ctx->enumerators) {
        //     return enumDecl;
        // }

        return enumType;
    }

    std::any visitEnumeratorList(CParser::EnumeratorListContext *ctx) override {
        auto firstEnumAny = visitEnumerator(ctx->firstenum);
        auto firstEnum = std::any_cast<EnumConstDeclaration*>(firstEnumAny);
        std::vector<EnumConstDeclaration*> enumerators{firstEnum};

        enumerators.reserve(ctx->enumTail.size() + 1);
        for (auto enumeratorCtx : ctx->enumTail) {
            auto enumeratorAny = visitEnumerator(enumeratorCtx);
            auto enumerator = std::any_cast<EnumConstDeclaration*>(enumeratorAny);
            enumerators.push_back(enumerator);
        }

        return enumerators;
    }

    std::any visitEnumerator(CParser::EnumeratorContext *ctx) override {
        auto ident = ctx->ident;
        auto name = ident->getText();

        ConstExpression* init = nullptr;
        if (ctx->init) {
            auto initAny = visitConstantExpression(ctx->init);
            init = std::any_cast<ConstExpression*>(initAny);
        }

        auto enumConstDecl = m_Program.CreateAstNode<EnumConstDeclaration>(init);
        enumConstDecl->SetName(name);

        auto intType = m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt);
        auto intQualType = m_Program.CreateType<QualType>(intType);
        enumConstDecl->SetType(intQualType);

        return enumConstDecl;
    }

public:
    std::any visitTypeQualifier(CParser::TypeQualifierContext *ctx) override {
        auto qualifierStr = ctx->getText();
        if (qualifierStr == "const") {
            return Qualifier::kConst;
        }
        if (qualifierStr == "restrict") {
            return Qualifier::kRestrict;
        }
        if (qualifierStr == "volatile") {
            return Qualifier::kVolatile;
        }
        return Qualifier::kNone;
    }

public:
    std::any visitFunctionSpecifier(CParser::FunctionSpecifierContext *ctx) override {
        auto specStr = ctx->getText();
        if (specStr == "inline") {
            return FunctionSpecifier::kInline;
        }
        return FunctionSpecifier::kNone;
    }

public:
    // TODO: avoid identifier string copying
    struct DeclaratorInfo {
        std::string Identifier;

        QualType* HeadType = nullptr;
        QualType* TailType = nullptr;

        FunctionDeclaration* FunctionDecl = nullptr;
    };

public:
    std::any visitDeclarator(CParser::DeclaratorContext *ctx) override {
        return visitChildren(ctx);
    }

public:
    struct ParamTypesInfo {
        std::vector<VariableDeclaration*> ParamDecls;
        bool HaveVariadic = false;
    };

public:
    std::any visitDirectDeclarator(CParser::DirectDeclaratorContext *ctx) override {
        auto identTerm = ctx->Identifier();
        if (identTerm) {
            auto identifier = identTerm->getText();
            return DeclaratorInfo{ .Identifier = identifier };
        }

        if (ctx->nested) {
            auto declInfoAny = visitDeclarator(ctx->nested);
            auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
            return declInfo;
        }

        if (ctx->arrdecl) {
            auto declInfoAny = visitDirectDeclarator(ctx->arrdecl);
            auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);

            auto arraySize = IntValue(0);  // NB: check init later
            if (ctx->sizeexpr) {
                // TODO: evaluate const expression
            }

            auto arrType = m_Program.CreateType<ArrayType>(declInfo.HeadType, arraySize);
            auto arrQualType = m_Program.CreateType<QualType>(arrType);
            declInfo.HeadType = arrQualType;
            return declInfo;
        }

        if (ctx->fundecl) {
            auto declInfoAny = visitDirectDeclarator(ctx->fundecl);
            auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
    
            auto funType = m_Program.CreateType<FunctionType>(declInfo.HeadType);
            auto funQualType = m_Program.CreateType<QualType>(funType);

            ParamTypesInfo paramTypesInfo;
            if (ctx->paramlist) {
                auto paramTypesAny = visitParameterTypeList(ctx->paramlist);
                paramTypesInfo = std::any_cast<ParamTypesInfo>(paramTypesAny);
            }

            std::vector<QualType*> paramTypes;
            for (const auto& paramDecl : paramTypesInfo.ParamDecls) {
                paramTypes.push_back(paramDecl->GetType());
            }
            funType->SetParamTypes(std::move(paramTypes));

            if (paramTypesInfo.HaveVariadic) {
                funType->SetVariadic();
            }
            if (!declInfo.HeadType) {  // is function declaration
                auto funcDecl = m_Program.CreateAstNode<FunctionDeclaration>();
                funcDecl->SetType(funQualType);
                declInfo.FunctionDecl = funcDecl;
                if (funType->IsVariadic()) {  // TODO: fix
                    declInfo.FunctionDecl->SetVariadic();
                }
                declInfo.FunctionDecl->SetParams(std::move(paramTypesInfo.ParamDecls));
            }

            declInfo.HeadType = funQualType;
            return declInfo;
        }

        return nullptr;
    }

    std::any visitPointer(CParser::PointerContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitTypeQualifierList(CParser::TypeQualifierListContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitParameterTypeList(CParser::ParameterTypeListContext *ctx) override {
        ParamTypesInfo paramsInfo;
        auto paramListAny = visitParameterList(ctx->params);
        paramsInfo.ParamDecls = std::any_cast<std::vector<VariableDeclaration*>>(paramListAny);

        if (ctx->vararg) {
            paramsInfo.HaveVariadic = true;
        }

        return paramsInfo;
    }

    std::any visitParameterList(CParser::ParameterListContext *ctx) override {
        auto firstDeclAny = visitParameterDeclaration(ctx->decl);
        auto firstDecl = std::any_cast<Declaration*>(firstDeclAny);
        std::vector<Declaration*> declList{firstDecl};

        declList.reserve(ctx->declTail.size() + 1);
        for (auto declCtx : ctx->declTail) {
            auto declAny = visitParameterDeclaration(declCtx);
            auto decl = std::any_cast<Declaration*>(declAny);
            declList.push_back(decl);
        }

        std::vector<VariableDeclaration*> paramList;
        paramList.reserve(declList.size());
        for (const auto& decl : declList) {
            if (decl->IsValueDecl()) {
                paramList.push_back(static_cast<VariableDeclaration*>(decl));
            } else {
                // TODO: handle error
            }
        }

        return paramList;
    }

    std::any visitParameterDeclaration(CParser::ParameterDeclarationContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitTypeName(CParser::TypeNameContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitAbstractDeclarator(CParser::AbstractDeclaratorContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitDirectAbstractDeclarator(CParser::DirectAbstractDeclaratorContext *ctx) override {
        if (ctx->nested) {
            auto declInfoAny = visitAbstractDeclarator(ctx->nested);
            auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
            return declInfo;
        }

        if (ctx->arrdecl || ctx->leafarr) {
            auto declInfo = DeclaratorInfo{};
            if (ctx->arrdecl) {
                auto declInfoAny = visitDirectAbstractDeclarator(ctx->arrdecl);
                declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
            }

            auto arraySize = IntValue(0);  // NB: check init later
            if (ctx->sizeexpr) {
                // TODO: evaluate const expression
            }

            auto arrType = m_Program.CreateType<ArrayType>(declInfo.HeadType, arraySize);
            auto arrQualType = m_Program.CreateType<QualType>(arrType);
            declInfo.HeadType = arrQualType;
            return declInfo;
        }

        if (ctx->fundecl || ctx->leaffun) {
            auto declInfo = DeclaratorInfo{};
            if (ctx->fundecl) {
                auto declInfoAny = visitDirectAbstractDeclarator(ctx->fundecl);
                declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
            }
    
            auto funType = m_Program.CreateType<FunctionType>(declInfo.HeadType);
            auto funQualType = m_Program.CreateType<QualType>(funType);

            ParamTypesInfo paramTypesInfo;
            if (ctx->paramlist) {
                auto paramTypesAny = visitParameterTypeList(ctx->paramlist);
                paramTypesInfo = std::any_cast<ParamTypesInfo>(paramTypesAny);
            }

            std::vector<QualType*> paramTypes;
            for (const auto& paramDecl : paramTypesInfo.ParamDecls) {
                paramTypes.push_back(paramDecl->GetType());
            }
            funType->SetParamTypes(std::move(paramTypes));

            if (paramTypesInfo.HaveVariadic) {
                funType->SetVariadic();
            }

            declInfo.HeadType = funQualType;
            return declInfo;
        }

        return nullptr;
    }

    // Skip
    std::any visitTypedefName(CParser::TypedefNameContext *ctx) override {
        return nullptr;
    }

    std::any visitInitializer(CParser::InitializerContext *ctx) override {
        if (ctx->init) {
            auto initAny = visitAssignmentExpression(ctx->init);
            auto init = std::any_cast<Expression*>(initAny);
            return init;
        }

        if (ctx->initlist) {
            auto initlistAny = visitInitializerList(ctx->initlist);
            auto initlist = std::any_cast<InitializerList*>(initlistAny);
            return static_cast<Expression*>(initlist);
        }

        return nullptr;
    }

    std::any visitInitializerList(CParser::InitializerListContext *ctx) override {
        auto firstInitAny = visitInitializer(ctx->init);
        auto firstInit = std::any_cast<Expression*>(firstInitAny);
        std::vector<Expression*> inits{firstInit};

        inits.reserve(ctx->initTail.size() + 1);
        for (auto initCtx : ctx->initTail) {
            auto initAny = visitInitializer(initCtx);
            auto init = std::any_cast<Expression*>(initAny);
            inits.push_back(init);
        }

        auto initList = m_Program.CreateAstNode<InitializerList>(std::move(inits));
        return initList;
    }

    // Dispatch
    std::any visitStatement(CParser::StatementContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitLabeledStatement(CParser::LabeledStatementContext *ctx) override {
        if (ctx->Identifier()) {
            auto labelDecl = m_Program.CreateAstNode<LabelDeclaration>();
            auto name = ctx->Identifier()->getText();
            labelDecl->SetName(name);

            Statement* statement = nullptr;
            if (ctx->statement()) {
                auto statementAny = ctx->statement();
                statement = std::any_cast<Statement*>(statementAny);
            }
            labelDecl->SetStatement(statement);
            
            auto labelStmt = m_Program.CreateAstNode<LabelStatement>(labelDecl, statement);
            return static_cast<Statement*>(labelStmt);
        }

        if (ctx->Case()) {
            auto constExprAny = visitConstantExpression(ctx->constantExpression());
            auto constExpr = std::any_cast<ConstExpression*>(constExprAny);

            auto bodyAny = visitStatement(ctx->statement());
            auto body = std::any_cast<Statement*>(bodyAny);

            auto caseStmt = m_Program.CreateAstNode<CaseStatement>(constExpr, body);
            return static_cast<Statement*>(caseStmt);
        }
        
        if (ctx->Default()) {
            auto bodyAny = visitStatement(ctx->statement());
            auto body = std::any_cast<Statement*>(bodyAny);

            auto defaultStmt = m_Program.CreateAstNode<DefaultStatement>(body);
            return static_cast<Statement*>(defaultStmt);
        }

        return nullptr;
    }

    std::any visitCompoundStatement(CParser::CompoundStatementContext *ctx) override {
        if (!ctx->blockItemList()) {
            auto compoundStmt = m_Program.CreateAstNode<CompoundStatement>();
            return static_cast<Statement*>(compoundStmt);
        }

        // auto outerScope = m_CurrentScope;
        // m_CurrentScope = m_SymbolTable.CreateScope("", m_CurrentScope);

        auto compoundStmtAny = visitBlockItemList(ctx->blockItemList());
        auto compoundStmt = std::any_cast<Statement*>(compoundStmtAny);

        // m_CurrentScope = outerScope;

        return static_cast<Statement*>(compoundStmt);
    }

    std::any visitBlockItemList(CParser::BlockItemListContext *ctx) override {
        auto compoundStmt = m_Program.CreateAstNode<CompoundStatement>();
        for (const auto& item : ctx->items) {
            auto statementAny = visitBlockItem(item);
            auto statement = std::any_cast<Statement*>(statementAny);
            compoundStmt->AddStatement(statement);
        }
        return static_cast<Statement*>(compoundStmt);
    }

    std::any visitBlockItem(CParser::BlockItemContext *ctx) override {
        if (ctx->statement()) {
            return visitStatement(ctx->statement());
        }

        if (ctx->declaration()) {
            auto declarationAny = visitDeclaration(ctx->declaration());
            auto declaration = std::any_cast<Declaration*>(declarationAny);
            auto declStatement = m_Program.CreateAstNode<DeclStatement>(declaration);
            return static_cast<Statement*>(declStatement);
        }

        return nullptr;
    }

    std::any visitExpressionStatement(CParser::ExpressionStatementContext *ctx) override {
        if (ctx->expression()) {
            auto expressionAny = visitExpression(ctx->expression());
            auto expression = std::any_cast<Expression*>(expressionAny);
            return static_cast<Statement*>(expression);
        }
        return nullptr;
    }

    std::any visitSelectionStatement(CParser::SelectionStatementContext *ctx) override {
        if (ctx->If()) {
            auto condExpressionAny = visitExpression(ctx->cond);
            auto condExpression = std::any_cast<Expression*>(condExpressionAny);

            auto thenStatementAny = visitStatement(ctx->thenstmt);
            auto thenStatement = std::any_cast<Statement*>(thenStatementAny);

            Statement* elseStatement = nullptr;
            if (ctx->elsestmt) {
                auto elseStatementAny = visitStatement(ctx->elsestmt);
                elseStatement = std::any_cast<Statement*>(elseStatementAny);
            }

            auto ifStmt = m_Program.CreateAstNode<IfStatement>(
                condExpression, thenStatement, elseStatement
            );
            return static_cast<Statement*>(ifStmt);
        }

        if (ctx->Switch()) {
            auto expressionAny = visitExpression(ctx->expression());
            auto expression = std::any_cast<Expression*>(expressionAny);

            auto statementAny = visitStatement(ctx->switchstmt);
            auto statement = std::any_cast<Statement*>(statementAny);

            auto switchStmt = m_Program.CreateAstNode<SwitchStatement>(
                expression, statement
            );
            return static_cast<Statement*>(switchStmt);  
        }

        return nullptr;
    }

    std::any visitIterationStatement(CParser::IterationStatementContext *ctx) override {
        if (ctx->loop->getType() == CParser::While) {
            auto conditionAny = visitExpression(ctx->cond);
            auto condition = std::any_cast<Expression*>(conditionAny);

            auto bodyAny = visitStatement(ctx->stmt);
            auto body = std::any_cast<Statement*>(bodyAny);

            auto whileStmt = m_Program.CreateAstNode<WhileStatement>(condition, body);
            return static_cast<Statement*>(whileStmt);
        }
        
        if (ctx->loop->getType() == CParser::Do) {
            auto conditionAny = visitExpression(ctx->cond);
            auto condition = std::any_cast<Expression*>(conditionAny);

            auto bodyAny = visitStatement(ctx->stmt);
            auto body = std::any_cast<Statement*>(bodyAny);

            auto doStmt = m_Program.CreateAstNode<DoStatement>(condition, body);
            return static_cast<Statement*>(doStmt);
        }

        if (ctx->loop->getType() == CParser::For) {
            auto conditionAny = visitForCondition(ctx->forCondition());
            auto condition = std::any_cast<
                    std::tuple<Statement*, Expression*, Expression*>>(conditionAny);
            auto [init, cond, step] = condition;

            auto bodyAny = visitStatement(ctx->stmt);
            auto body = std::any_cast<Statement*>(bodyAny);

            auto forStmt = m_Program.CreateAstNode<ForStatement>(init, cond, step, body);
            return static_cast<Statement*>(forStmt);
        }

        return nullptr;
    }

    std::any visitForCondition(CParser::ForConditionContext *ctx) override {
        Statement* initStatement = nullptr;
        if (ctx->decl) {
            auto declAny = visitForDeclaration(ctx->decl);
            auto decl = std::any_cast<Declaration*>(declAny);
            initStatement = m_Program.CreateAstNode<DeclStatement>(decl);
        } else if (ctx->expr) {
            auto exprAny = visitExpression(ctx->expr);
            initStatement = std::any_cast<Expression*>(exprAny);
        }

        Expression* condExpression = nullptr;
        if (ctx->cond) {
            auto condAny = visitForExpression(ctx->cond);
            condExpression = std::any_cast<Expression*>(condAny);
        }

        Expression* stepExpression = nullptr;
        if (ctx->step) {
            auto stepAny = visitForExpression(ctx->step);
            stepExpression = std::any_cast<Expression*>(stepAny);
        }

        return std::tuple{initStatement, condExpression, stepExpression};
    }

    std::any visitForDeclaration(CParser::ForDeclarationContext *ctx) override {
        // TODO: declaration
        return visitChildren(ctx);
    }

    std::any visitForExpression(CParser::ForExpressionContext *ctx) override {
        auto exprList = m_Program.CreateAstNode<ExpressionList>();

        auto firstExpressionAny = visitAssignmentExpression(ctx->expr);
        auto firstExpression = std::any_cast<Expression*>(firstExpressionAny);
        exprList->AddExpression(firstExpression);

        for (auto exprCtx : ctx->exprTail) {
            auto expressionAny = visitAssignmentExpression(exprCtx);
            auto expression = std::any_cast<Expression*>(expressionAny);
            exprList->AddExpression(expression);
        }

        return static_cast<Expression*>(exprList);
    }

    std::any visitJumpStatement(CParser::JumpStatementContext *ctx) override {
        if (ctx->Goto()) {
            auto labelDecl = m_Program.CreateAstNode<LabelDeclaration>();
            auto name = ctx->Identifier()->getText();
            labelDecl->SetName(name);
            auto gotoStmt = m_Program.CreateAstNode<GotoStatement>(labelDecl);
            return static_cast<Statement*>(gotoStmt);
        }

        if (ctx->Continue()) {
            auto continueStmt = m_Program.CreateAstNode<LoopJumpStatement>(LoopJumpStatement::Type::kContinue);
            return static_cast<Statement*>(continueStmt);
        }

        if (ctx->Break()) {
            auto breakStmt = m_Program.CreateAstNode<LoopJumpStatement>(LoopJumpStatement::Type::kBreak);
            return static_cast<Statement*>(breakStmt);
        }

        if (ctx->Return()) {
            Expression* expression = nullptr;
            if (ctx->expression()) {
                auto expressionAny = visitExpression(ctx->expression());
                expression = std::any_cast<Expression*>(expressionAny);
            }
            auto returnStmt = m_Program.CreateAstNode<ReturnStatement>(expression);
            return static_cast<Statement*>(returnStmt);
        }
    
        return nullptr;
    }

    std::any visitTranslationUnit(CParser::TranslationUnitContext *ctx) override {
        auto translationUnit = m_Program.CreateAstNode<TranslationUnit>();
        for (auto declCtx : ctx->decls) {
            auto declarationAny = visitExternalDeclaration(declCtx);
            auto declaration = std::any_cast<Declaration*>(declarationAny);
            translationUnit->AddDeclaration(declaration);
        }
        return translationUnit;
    }

    // Dispatch
    std::any visitExternalDeclaration(CParser::ExternalDeclarationContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitFunctionDefinition(CParser::FunctionDefinitionContext *ctx) override {
        auto functionDecl = m_Program.CreateAstNode<FunctionDeclaration>();

        auto bodyAny = visitCompoundStatement(ctx->body);
        auto body = std::any_cast<Statement*>(bodyAny);
        functionDecl->SetBody(body);

        return functionDecl;
    }

private:
    Program& m_Program;

    // SymbolTable m_SymbolTable;
    // Scope* m_CurrentScope = m_SymbolTable.GetGlobalScope();
};

};