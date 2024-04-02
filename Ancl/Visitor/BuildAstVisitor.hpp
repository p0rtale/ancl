#include <format>

#include "antlr4-runtime.h"
#include "CParserBaseVisitor.h"

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Grammar/AST/Program.hpp>

#include <Ancl/Grammar/AST/Base/Location.hpp>


using namespace ast;

namespace anclgrammar {

class BuildAstVisitor: public CParserBaseVisitor {
public:
    BuildAstVisitor(Program& program): m_Program(program) {}

public:
    std::any visitPrimaryExpression(CParser::PrimaryExpressionContext *ctx) override {
        if (ctx->Identifier()) {
            auto name = ctx->getText();
            auto valueDecl = m_Program.CreateAstNode<ValueDeclaration>(std::move(name));
            auto declExpression = m_Program.CreateAstNode<DeclRefExpression>(valueDecl);
            declExpression->SetLocation(createASTLocation(ctx));
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
        stringExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(stringExpr);
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
                intExpr->SetLocation(createASTLocation(ctx));
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
                floatExpr->SetLocation(createASTLocation(ctx));
                return floatExpr;
            }
            throw std::runtime_error(std::format("invalid real number constant \"{}\"", str));
        } else if (ctx->CharacterConstant()) {
            // TODO: handle multichar
            Expression* charExpr = m_Program.CreateAstNode<CharExpression>(str[1]);
            charExpr->SetLocation(createASTLocation(ctx));
            return charExpr;
        }
        return nullptr;
    }

    std::any visitEnumerationConstant(CParser::EnumerationConstantContext *ctx) override {
        auto name = ctx->getText();
        auto enumConstDecl = m_Program.CreateAstNode<EnumConstDeclaration>();
        enumConstDecl->SetLocation(createASTLocation(ctx));
        enumConstDecl->SetName(name);

        // 6.7.2.2 - Enumerators have type int
        auto intType = m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt);
        auto intQualType = m_Program.CreateType<QualType>(intType);
        enumConstDecl->SetType(intQualType);

        return enumConstDecl;
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
                argsExpression = std::any_cast<std::vector<Expression*>>(argsExpressionAny);
            }
            auto callExpr = m_Program.CreateAstNode<CallExpression>(
                leftExpression, argsExpression
            );
            callExpr->SetLocation(createASTLocation(ctx));
            return static_cast<Expression*>(callExpr);      
        }

        if (ctx->array) {
            auto indexExpression = std::any_cast<Expression*>(visitExpression(ctx->index));
            auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
                leftExpression, indexExpression,
                BinaryExpression::OpType::kArrSubscript
            );
            binaryExpr->SetLocation(createASTLocation(ctx));
            return static_cast<Expression*>(binaryExpr);
        }

        if (ctx->tag) {
            auto name = ctx->Identifier()->getText();
            auto memberDecl = m_Program.CreateAstNode<FieldDeclaration>(std::move(name));
            auto memberExpression = m_Program.CreateAstNode<DeclRefExpression>(memberDecl);
            memberExpression->SetLocation(createASTLocation(ctx));

            auto op = BinaryExpression::OpType::kNone;
            if (ctx->member->getText() == ".") {
                op = BinaryExpression::OpType::kDirectMember;
            } else if (ctx->member->getText() == "->") {
                op = BinaryExpression::OpType::kArrowMember;
            }
    
            auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
                leftExpression, memberExpression, op
            );
            binaryExpr->SetLocation(createASTLocation(ctx));
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
        unaryExpr->SetLocation(createASTLocation(ctx));
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
        unaryExpr->SetLocation(createASTLocation(ctx));
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
            unaryExpr->SetLocation(createASTLocation(ctx));
            return static_cast<Expression*>(unaryExpr);
        }

        auto typeNameAny = visitTypeName(ctx->typeName());
        auto typeName = std::any_cast<QualType*>(typeNameAny);

        auto sizeofExpr = m_Program.CreateAstNode<SizeofTypeExpression>(typeName);
        sizeofExpr->SetLocation(createASTLocation(ctx));
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
        castExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(castExpr);
    }

    std::any visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext *ctx) override {
        if (ctx->castexpr) {
            return visitCastExpression(ctx->castexpr);
        }

        auto leftExpressionAny = visitMultiplicativeExpression(ctx->multiplicativeExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitCastExpression(ctx->castExpression());
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
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitAdditiveExpression(CParser::AdditiveExpressionContext *ctx) override {
        if (ctx->mulexpr) {
            return visitMultiplicativeExpression(ctx->mulexpr);
        }

        auto leftExpressionAny = visitAdditiveExpression(ctx->additiveExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitMultiplicativeExpression(ctx->multiplicativeExpression());
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
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitShiftExpression(CParser::ShiftExpressionContext *ctx) override {
        if (ctx->addexpr) {
            return visitAdditiveExpression(ctx->addexpr);
        }

        auto leftExpressionAny = visitShiftExpression(ctx->shiftExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitAdditiveExpression(ctx->additiveExpression());
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
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitRelationalExpression(CParser::RelationalExpressionContext *ctx) override {
        if (ctx->shiftexpr) {
            return visitShiftExpression(ctx->shiftexpr);
        }

        auto leftExpressionAny = visitRelationalExpression(ctx->relationalExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitShiftExpression(ctx->shiftExpression());
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
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitEqualityExpression(CParser::EqualityExpressionContext *ctx) override {
        if (ctx->relexpr) {
            return visitRelationalExpression(ctx->relexpr);
        }

        auto leftExpressionAny = visitEqualityExpression(ctx->equalityExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitRelationalExpression(ctx->relationalExpression());
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
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitAndExpression(CParser::AndExpressionContext *ctx) override {
        if (ctx->eqexpr) {
            return visitEqualityExpression(ctx->eqexpr);
        }

        auto leftExpressionAny = visitAndExpression(ctx->andExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitEqualityExpression(ctx->equalityExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kAnd
        );
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitExclusiveOrExpression(CParser::ExclusiveOrExpressionContext *ctx) override {
        if (ctx->andexpr) {
            return visitAndExpression(ctx->andexpr);
        }

        auto leftExpressionAny = visitExclusiveOrExpression(ctx->exclusiveOrExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitAndExpression(ctx->andExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kXor
        );
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitInclusiveOrExpression(CParser::InclusiveOrExpressionContext *ctx) override {
        if (ctx->exorexpr) {
            return visitExclusiveOrExpression(ctx->exorexpr);
        }

        auto leftExpressionAny = visitInclusiveOrExpression(ctx->inclusiveOrExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitExclusiveOrExpression(ctx->exclusiveOrExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kOr
        );
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitLogicalAndExpression(CParser::LogicalAndExpressionContext *ctx) override {
        if (ctx->incorexpr) {
            return visitInclusiveOrExpression(ctx->incorexpr);
        }

        auto leftExpressionAny = visitLogicalAndExpression(ctx->logicalAndExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitInclusiveOrExpression(ctx->inclusiveOrExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kLogAnd
        );
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    std::any visitLogicalOrExpression(CParser::LogicalOrExpressionContext *ctx) override {
        if (ctx->logandexpr) {
            return visitLogicalAndExpression(ctx->logandexpr);
        }

        auto leftExpressionAny = visitLogicalOrExpression(ctx->logicalOrExpression());
        auto leftExpression = std::any_cast<Expression*>(leftExpressionAny);

        auto rightExpressionAny = visitLogicalAndExpression(ctx->logicalAndExpression());
        auto rightExpression = std::any_cast<Expression*>(rightExpressionAny);

        auto binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, rightExpression, BinaryExpression::OpType::kLogOr
        );
        binaryExpr->SetLocation(createASTLocation(ctx));
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
        condExpr->SetLocation(createASTLocation(ctx));
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
        binaryExpr->SetLocation(createASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    // Skip
    std::any visitAssignmentOperator(CParser::AssignmentOperatorContext *ctx) override {
        return nullptr;
    }

    std::any visitExpression(CParser::ExpressionContext *ctx) override {
        auto exprList = m_Program.CreateAstNode<ExpressionList>();
        exprList->SetLocation(createASTLocation(ctx));

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
        constExpr->SetLocation(createASTLocation(ctx));
        return constExpr;
    }

public:
    // TODO: avoid identifier string copying
    struct DeclaratorInfo {
        std::string Identifier;

        QualType* HeadType = nullptr;
        QualType* TailType = nullptr;

        FunctionDeclaration* FunctionDecl = nullptr;
    };

    struct AbstractDeclaratorInfo {
        QualType* HeadType = nullptr;
        QualType* TailType = nullptr;
    };

    struct InitDeclaratorInfo {
        DeclaratorInfo DeclInfo;
        Expression* Init = nullptr;
    };

    enum class FunctionSpecifier {
        kNone = 0,
        kInline,
    };

    enum class StorageClassInfo {
        kNone = 0,
        kTypedef,
        kExtern,
        kStatic,
        kAuto,
        kRegister,
    };

    struct DeclSpecifiers {
        bool IsTypeDef = false;
        StorageClassInfo Storage = StorageClassInfo::kNone;
        FunctionSpecifier FunSpec = FunctionSpecifier::kNone;
        QualType* Type = nullptr;
    };

    struct DeclarationInfo {
        TagDeclaration* TagPreDecl = nullptr;
        Declaration* Decl = nullptr;
    };

private:
    DeclarationInfo createDeclaration(const DeclSpecifiers& declSpecs,
                                      const std::vector<DeclaratorInfo>& declList) {
        auto initDeclList = std::vector<InitDeclaratorInfo>{};
        initDeclList.reserve(declList.size());

        for (const auto& decl : declList) {
            initDeclList.push_back(InitDeclaratorInfo{
                .DeclInfo=decl,
                .Init=nullptr,
            });
        }

        return createDeclaration(declSpecs, initDeclList);
    }

    DeclarationInfo createDeclaration(const DeclSpecifiers& declSpecs,
                                      const std::vector<InitDeclaratorInfo>& initDeclList) {
        auto resInfo = DeclarationInfo{};
        
        auto qualType = declSpecs.Type;
        auto type = qualType->GetSubType();
        auto recordTypePtr = dynamic_cast<RecordType*>(type);
        if (recordTypePtr) {
            auto recordType = *recordTypePtr;
            auto recordDecl = recordType.GetDeclaration();
            resInfo.TagPreDecl = recordDecl;
        }

        auto enumTypePtr = dynamic_cast<EnumType*>(type);
        if (enumTypePtr) {
            auto enumType = *enumTypePtr;
            auto enumDecl = enumType.GetDeclaration();
            resInfo.TagPreDecl = enumDecl;
        }

        if (initDeclList.empty()) {
            return resInfo;
        }

        for (auto& initDecl : initDeclList) {
            auto declInfo = initDecl.DeclInfo;

            if (declInfo.TailType) {
                auto tailQualType = declInfo.TailType;
                auto tailType = dynamic_cast<INodeType*>(tailQualType->GetSubType());
                tailType->SetSubType(qualType);
            } else {  // declspecs Ident
                declInfo.HeadType = qualType;
                declInfo.TailType = declInfo.HeadType;
            }

            if (declSpecs.IsTypeDef) {
                auto typedefDecl = m_Program.CreateAstNode<TypedefDeclaration>();

                if (initDecl.Init) {
                    // TODO: handle error
                }

                typedefDecl->SetName(declInfo.Identifier);
                typedefDecl->SetType(declInfo.HeadType);

                resInfo.Decl = typedefDecl;
            } else if (declInfo.FunctionDecl) {
                if (initDecl.Init) {
                    // TODO: handle error
                }
                auto funDecl = declInfo.FunctionDecl;

                if (declSpecs.FunSpec == FunctionSpecifier::kInline) {
                    funDecl->SetInline();
                }

                switch (declSpecs.Storage) {
                case StorageClassInfo::kExtern:
                    funDecl->SetStorageClass(StorageClass::kExtern);
                    break;
                case StorageClassInfo::kStatic:
                    funDecl->SetStorageClass(StorageClass::kStatic);
                    break;
                case StorageClassInfo::kAuto:
                    funDecl->SetStorageClass(StorageClass::kAuto);
                    break;
                case StorageClassInfo::kRegister:
                    funDecl->SetStorageClass(StorageClass::kRegister);
                    break;
                }

                resInfo.Decl = funDecl;
            } else {
                if (declSpecs.FunSpec != FunctionSpecifier::kNone) {
                    // TODO: handle error
                }

                auto varDecl = m_Program.CreateAstNode<VariableDeclaration>();
                varDecl->SetName(declInfo.Identifier);
                varDecl->SetType(declInfo.HeadType);
                varDecl->SetInit(initDecl.Init);

                switch (declSpecs.Storage) {
                case StorageClassInfo::kExtern:
                    varDecl->SetStorageClass(StorageClass::kExtern);
                    break;
                case StorageClassInfo::kStatic:
                    varDecl->SetStorageClass(StorageClass::kStatic);
                    break;
                case StorageClassInfo::kAuto:
                    varDecl->SetStorageClass(StorageClass::kAuto);
                    break;
                case StorageClassInfo::kRegister:
                    varDecl->SetStorageClass(StorageClass::kRegister);
                    break;
                }

                resInfo.Decl = varDecl;
            }
        }

        return resInfo;
    }

public:
    std::any visitDeclaration(CParser::DeclarationContext *ctx) override {
        // DeclarationSpecifiers:
        // 1) Declaration (struct, union, enum) -> add and wrap with var declaration
        // 2) Type -> just wrap with declaration with declarator

        auto declSpecsAny = visitDeclarationSpecifiers(ctx->declspecs);
        auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

        auto initDeclList = std::vector<InitDeclaratorInfo>{};
        if (ctx->initdecl) {
            auto initDeclListAny = visitInitDeclaratorList(ctx->initdecl);
            initDeclList = std::any_cast<std::vector<InitDeclaratorInfo>>(initDeclListAny);
        }

        auto declarationInfo = createDeclaration(declSpecs, initDeclList);
        return declarationInfo;
    }

public:
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

    struct BuiltinTypeInfo {
        enum class SizeModifier {
            kNone, kShort, kLong, kLongLong,
        } Size;

        enum class BasicType {
            kNone, kVoid, kChar, kInt, kFloat, kDouble,
        } Type;

        enum class SignModifier {
            kNone, kSigned, kUnsigned,
        } Sign;
    };

private:
    Type* createBuiltinTypeFromInfo(const BuiltinTypeInfo& info) {
        if (info.Type == BuiltinTypeInfo::BasicType::kNone &&
                info.Size == BuiltinTypeInfo::SizeModifier::kNone) {
            if (info.Sign == BuiltinTypeInfo::SignModifier::kSigned) {
                return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt);
            }
            if (info.Sign == BuiltinTypeInfo::SignModifier::kUnsigned) {
                return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kUInt);
            }
            return nullptr;
        }

        if (info.Type == BuiltinTypeInfo::BasicType::kVoid) {
            if (info.Size != BuiltinTypeInfo::SizeModifier::kNone) {
                // TODO: handle error
            }
            if (info.Sign != BuiltinTypeInfo::SignModifier::kNone) {
                // TODO: handle error
            }
            return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kVoid);
        }

        if (info.Type == BuiltinTypeInfo::BasicType::kChar) {
            if (info.Size != BuiltinTypeInfo::SizeModifier::kNone) {
                // TODO: handle error
            }
            if (info.Sign == BuiltinTypeInfo::SignModifier::kUnsigned) {
                return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kUChar);
            }
            return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kChar);
        }

        if (info.Type == BuiltinTypeInfo::BasicType::kFloat) {
            if (info.Size != BuiltinTypeInfo::SizeModifier::kNone) {
                // TODO: handle error
            }
            if (info.Sign != BuiltinTypeInfo::SignModifier::kNone) {
                // TODO: handle error
            }
            return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kFloat);
        }

        if (info.Type == BuiltinTypeInfo::BasicType::kDouble) {
            if (info.Sign != BuiltinTypeInfo::SignModifier::kNone) {
                // TODO: handle error
            }
            if (info.Size == BuiltinTypeInfo::SizeModifier::kShort) {
                // TODO: handle error
            } else if (info.Size == BuiltinTypeInfo::SizeModifier::kLong) {
                return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kLongDouble);
            }
            return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kDouble);
        }

        if (info.Size == BuiltinTypeInfo::SizeModifier::kShort) {
            if (info.Sign == BuiltinTypeInfo::SignModifier::kUnsigned) {
                return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kUShort);
            }
            return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kShort);
        }

        if (info.Size == BuiltinTypeInfo::SizeModifier::kLong ||
                info.Size == BuiltinTypeInfo::SizeModifier::kLongLong) {
            if (info.Sign == BuiltinTypeInfo::SignModifier::kUnsigned) {
                return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kULong);
            }
            return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kLong);  
        }

        if (info.Type == BuiltinTypeInfo::BasicType::kInt) {
            if (info.Sign == BuiltinTypeInfo::SignModifier::kUnsigned) {
                return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kUInt);
            }
            return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt);  
        }

        return nullptr;
    }

private:
    DeclSpecifiers createDeclSpecs(const std::vector<std::any>& declSpecsAny) {
        auto declSpecs = DeclSpecifiers{};

        auto qualType = m_Program.CreateType<QualType>();
        declSpecs.Type = qualType;

        Type* declSpecType = nullptr;
        auto builtinTypeInfo = BuiltinTypeInfo{};

        for (const auto& declSpecAny : declSpecsAny) {
            auto storageClassPtr = std::any_cast<StorageClassInfo>(&declSpecAny);
            if (storageClassPtr) {
                auto storageClass = *storageClassPtr;

                if (storageClass == StorageClassInfo::kTypedef) {
                    if (declSpecs.IsTypeDef || declSpecs.Storage != StorageClassInfo::kNone) {
                        // TODO: handle error
                    }
                    declSpecs.IsTypeDef = true;
                } else {
                    if (storageClass != StorageClassInfo::kNone &&
                            declSpecs.Storage != StorageClassInfo::kNone) {
                        // TODO: handle static + extern, static + static, ... errors
                    }
                    declSpecs.Storage = storageClass;
                }
            }

            auto builtinTypeSpecPtr = std::any_cast<BuiltinTypeSpecifier>(&declSpecAny);
            if (builtinTypeSpecPtr) {
                auto builtinTypeSpec = *builtinTypeSpecPtr;

                if (builtinTypeSpec == BuiltinTypeSpecifier::kSigned) {
                    if (builtinTypeInfo.Sign != BuiltinTypeInfo::SignModifier::kNone) {
                        // TODO: handle error
                    }
                    builtinTypeInfo.Sign = BuiltinTypeInfo::SignModifier::kSigned;
                } else if (builtinTypeSpec == BuiltinTypeSpecifier::kUnsigned) {
                    if (builtinTypeInfo.Sign != BuiltinTypeInfo::SignModifier::kNone) {
                        // TODO: handle error
                    }
                    builtinTypeInfo.Sign = BuiltinTypeInfo::SignModifier::kUnsigned;
                } else if (builtinTypeSpec == BuiltinTypeSpecifier::kShort) {
                    if (builtinTypeInfo.Size != BuiltinTypeInfo::SizeModifier::kNone) {
                        // TODO: handle error
                    }
                    builtinTypeInfo.Size = BuiltinTypeInfo::SizeModifier::kShort;
                } else if (builtinTypeSpec == BuiltinTypeSpecifier::kLong) {
                    if (builtinTypeInfo.Size == BuiltinTypeInfo::SizeModifier::kLong) {
                        builtinTypeInfo.Size = BuiltinTypeInfo::SizeModifier::kLongLong; 
                    } else if (builtinTypeInfo.Size != BuiltinTypeInfo::SizeModifier::kNone) {
                        // TODO: handle error
                    } else {
                        builtinTypeInfo.Size = BuiltinTypeInfo::SizeModifier::kLong; 
                    }
                } else {  // basic type
                    if (builtinTypeInfo.Type != BuiltinTypeInfo::BasicType::kNone) {
                        // TODO: handle error
                    }
                    switch (builtinTypeSpec) {
                    case BuiltinTypeSpecifier::kVoid:
                        builtinTypeInfo.Type = BuiltinTypeInfo::BasicType::kVoid;
                        break;
                    case BuiltinTypeSpecifier::kChar:
                        builtinTypeInfo.Type = BuiltinTypeInfo::BasicType::kChar;
                        break;
                    case BuiltinTypeSpecifier::kInt:
                        builtinTypeInfo.Type = BuiltinTypeInfo::BasicType::kInt;
                        break;
                    case BuiltinTypeSpecifier::kFloat:
                        builtinTypeInfo.Type = BuiltinTypeInfo::BasicType::kFloat;
                        break;
                    case BuiltinTypeSpecifier::kDouble:
                        builtinTypeInfo.Type = BuiltinTypeInfo::BasicType::kDouble;
                        break;
                    default:
                        // TODO: handle error
                        break;
                    }
                }
            }

            auto namedTypeSpecPtr = std::any_cast<Type*>(&declSpecAny);
            if (namedTypeSpecPtr) {
                auto namedType = *namedTypeSpecPtr;
                auto qualType = declSpecs.Type;
                if (declSpecType) {
                    // TODO: handle error
                }
                declSpecType = namedType;
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
                if (declSpecs.FunSpec != FunctionSpecifier::kNone) {
                    // TODO: handle error
                }
                declSpecs.FunSpec = *functionSpecPtr;
            }
        }

        Type* builtinType = createBuiltinTypeFromInfo(builtinTypeInfo);
        if (builtinType) {
            if (declSpecType) {
                // TODO: handle error
            }
            declSpecType = builtinType;
        }

        qualType->SetSubType(declSpecType);

        return declSpecs;
    }

public:
    std::any visitDeclarationSpecifiers(CParser::DeclarationSpecifiersContext *ctx) override {
        auto specsCtx = ctx->specs;
        auto declSpecsAny = std::vector<std::any>{};
        declSpecsAny.reserve(specsCtx.size());
        for (auto specCtx : specsCtx) {
            auto declSpecAny = visitDeclarationSpecifier(specCtx);
            declSpecsAny.push_back(declSpecAny);
        }

        return createDeclSpecs(declSpecsAny);
    }

    std::any visitDeclarationSpecifier(CParser::DeclarationSpecifierContext *ctx) override {
        if (ctx->storage) {
            auto storageClassAny = visitStorageClassSpecifier(ctx->storage);
            auto storageClass = std::any_cast<StorageClassInfo>(storageClassAny);
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
            return typeQualifier;
        }

        if (ctx->funcspec) {
            auto funcSpecifierAny = visitFunctionSpecifier(ctx->funcspec);
            auto funcSpecifier = std::any_cast<FunctionSpecifier>(funcSpecifierAny);
            return funcSpecifier;
        }

        return nullptr;
    }

    std::any visitInitDeclaratorList(CParser::InitDeclaratorListContext *ctx) override {
        auto firstDeclInfoAny = visitInitDeclarator(ctx->init);
        auto firstDeclInfo = std::any_cast<InitDeclaratorInfo>(firstDeclInfoAny);
        std::vector<InitDeclaratorInfo> decls{firstDeclInfo};

        decls.reserve(ctx->initTail.size() + 1);
        for (auto declCtx : ctx->initTail) {
            auto declInfoAny = visitInitDeclarator(declCtx);
            auto declInfo = std::any_cast<InitDeclaratorInfo>(declInfoAny);
            decls.push_back(declInfo);
        }

        return decls;
    }

    std::any visitInitDeclarator(CParser::InitDeclaratorContext *ctx) override {
        auto declInfoAny = visitDeclarator(ctx->decl);
        auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);

        auto initDeclInfo = InitDeclaratorInfo{ .DeclInfo = std::move(declInfo) };

        Expression* init = nullptr;
        if (ctx->init) {
            auto initAny = visitInitializer(ctx->init);
            init = std::any_cast<Expression*>(initAny);
            initDeclInfo.Init = init;
        }

        return initDeclInfo;
    }

    std::any visitStorageClassSpecifier(CParser::StorageClassSpecifierContext *ctx) override {
        auto storageStr = ctx->getText();
        if (storageStr == "typedef") {
            return StorageClassInfo::kTypedef;
        }

        if (storageStr == "extern") {
            return StorageClassInfo::kExtern;
        }
        if (storageStr == "static") {
            return StorageClassInfo::kStatic;
        }
        if (storageStr == "auto") {
            return StorageClassInfo::kAuto;
        }
        if (storageStr == "register") {
            return StorageClassInfo::kRegister;
        }
        return StorageClassInfo::kNone;
    }

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

public:
    struct FieldInfo {
        TagDeclaration* TagPreDecl;
        FieldDeclaration* FieldDecl;
    };

public:
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

        std::vector<FieldInfo> fieldsInfo;
        if (ctx->body) {
            auto fieldsInfoAny = visitStructDeclarationList(ctx->body);
            fieldsInfo = std::any_cast<std::vector<FieldInfo>>(fieldsInfoAny);
            if (fieldsInfo.empty()) {
                // TODO: handle error
            }
        }

        auto recordDecl = m_Program.CreateAstNode<RecordDeclaration>(isUnion);
        recordDecl->SetLocation(createASTLocation(ctx));

        for (const auto& fieldInfo : fieldsInfo) {
            if (fieldInfo.TagPreDecl) {
                recordDecl->AddInternalTagDecl(fieldInfo.TagPreDecl);
            }
            if (fieldInfo.FieldDecl) {
                recordDecl->AddField(fieldInfo.FieldDecl);
            }
        }

        auto recordType = m_Program.CreateType<RecordType>(recordDecl);
        auto recordQualType = m_Program.CreateType<QualType>(recordType);
        recordDecl->SetName(name);
        recordDecl->SetType(recordQualType);

        return recordType;
    }

    // Skip
    std::any visitStructOrUnion(CParser::StructOrUnionContext *ctx) override {
        return nullptr;
    }

    std::any visitStructDeclarationList(CParser::StructDeclarationListContext *ctx) override {
        std::vector<FieldInfo> fieldsInfo;
        for (auto declCtx : ctx->decls) {
            auto declarationInfoAny = visitStructDeclaration(declCtx);
            auto declarationInfo = std::any_cast<DeclarationInfo>(declarationInfoAny);

            FieldDeclaration* fieldDecl = nullptr;
            if (declarationInfo.Decl) {
                auto varDecl = dynamic_cast<VariableDeclaration*>(declarationInfo.Decl);
                if (!varDecl) {
                    // TODO: handle error
                }

                fieldDecl = m_Program.CreateAstNode<FieldDeclaration>(
                    varDecl->GetName(), varDecl->GetType());
                fieldDecl->SetLocation(createASTLocation(declCtx));
            }

            auto fieldInfo = FieldInfo{
                .TagPreDecl = declarationInfo.TagPreDecl,
                .FieldDecl = fieldDecl,
            };

            fieldsInfo.push_back(fieldInfo);
        }
        return fieldsInfo;
    }

    std::any visitStructDeclaration(CParser::StructDeclarationContext *ctx) override {
        auto declSpecsAny = visitSpecifierQualifierList(ctx->specqual);
        auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

        auto declList = std::vector<DeclaratorInfo>{};
        if (ctx->decls) {
            auto declListAny = visitStructDeclaratorList(ctx->decls);
            declList = std::any_cast<std::vector<DeclaratorInfo>>(declListAny);
        }

        auto declarationInfo = createDeclaration(declSpecs, declList);
        return declarationInfo;
    }

    std::any visitSpecifierQualifierList(CParser::SpecifierQualifierListContext *ctx) override {
        auto specsCtx = ctx->specs;
        auto declSpecsAny = std::vector<std::any>{};
        declSpecsAny.reserve(specsCtx.size());
        for (auto specCtx : specsCtx) {
            auto declSpecAny = visitSpecifierQualifier(specCtx);
            declSpecsAny.push_back(declSpecAny);
        }

        return createDeclSpecs(declSpecsAny);
    }

    std::any visitSpecifierQualifier(CParser::SpecifierQualifierContext *ctx) override {
        if (ctx->typespec) {
            auto typeSpecifierAny = visitTypeSpecifier(ctx->typespec);

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
            return typeQualifier;
        }
        
        return nullptr;
    }

    std::any visitStructDeclaratorList(CParser::StructDeclaratorListContext *ctx) override {
        auto firstDeclInfoAny = visitStructDeclarator(ctx->decl);
        auto firstDeclInfo = std::any_cast<DeclaratorInfo>(firstDeclInfoAny);

        auto declsInfo = std::vector<DeclaratorInfo>{std::move(firstDeclInfo)};
        auto declTail = ctx->declTail;
        declsInfo.reserve(declTail.size() + 1);
        for (auto declCtx : declTail) {
            auto declInfoAny = visitStructDeclarator(declCtx);
            auto declInfo = std::any_cast<DeclaratorInfo>(firstDeclInfoAny);
            declsInfo.push_back(std::move(declInfo));
        }

        return declsInfo;
    }

    std::any visitStructDeclarator(CParser::StructDeclaratorContext *ctx) override {
        auto declaratorInfoAny = visitDeclarator(ctx->declarator());
        auto declaratorInfo = std::any_cast<DeclaratorInfo>(declaratorInfoAny);
        return declaratorInfo;
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
        
            if (enumerators.empty()) {
                // TODO: handle error
            }
        }

        auto enumDecl = m_Program.CreateAstNode<EnumDeclaration>(std::move(enumerators));
        enumDecl->SetLocation(createASTLocation(ctx));
        auto enumType = m_Program.CreateType<EnumType>(enumDecl);
        auto enumQualType = m_Program.CreateType<QualType>(enumType);
        enumDecl->SetType(enumQualType);

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
        enumConstDecl->SetLocation(createASTLocation(ctx));
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
    struct PointerInfo {
        QualType* HeadType = nullptr;
        QualType* TailType = nullptr;
    };

public:
    std::any visitDeclarator(CParser::DeclaratorContext *ctx) override {
        auto pointerCtx = ctx->pointer();
        auto ptrInfo = PointerInfo{};
        if (pointerCtx) {
            auto ptrInfoAny = visitPointer(pointerCtx);
            ptrInfo = std::any_cast<PointerInfo>(ptrInfoAny);
        }

        auto declaratorAny = visitDirectDeclarator(ctx->directDeclarator());
        auto declarator = std::any_cast<DeclaratorInfo>(declaratorAny);

        if (!ptrInfo.HeadType) {  // no pointers
            return declarator;
        }

        if (declarator.TailType) {
            auto declTailQualType = declarator.TailType;
            auto declTailType = dynamic_cast<INodeType*>(declTailQualType->GetSubType());
            declTailType->SetSubType(ptrInfo.HeadType);
        } else {  // * ident
            declarator.HeadType = ptrInfo.HeadType;
        }
        declarator.TailType = ptrInfo.TailType;

        return declarator;
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

            auto arrType = m_Program.CreateType<ArrayType>(nullptr, arraySize);
            auto arrQualType = m_Program.CreateType<QualType>(arrType);

            if (!declInfo.HeadType) {  // Ident []
                declInfo.HeadType = arrQualType;
                declInfo.TailType = declInfo.HeadType;
                return declInfo;
            }

            auto declTailQualType = declInfo.TailType;
            auto tailNodeType = dynamic_cast<INodeType*>(declTailQualType->GetSubType());
            if (tailNodeType->IsFunctionType()) {
                // TODO: handle error
            }

            tailNodeType->SetSubType(arrQualType);
            declInfo.TailType = arrQualType;
            return declInfo;
        }

        if (ctx->fundecl) {
            auto declInfoAny = visitDirectDeclarator(ctx->fundecl);
            auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
    
            auto funType = m_Program.CreateType<FunctionType>(nullptr);
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
                funcDecl->SetName(declInfo.Identifier);
                funcDecl->SetType(funQualType);
                declInfo.FunctionDecl = funcDecl;
                if (funType->IsVariadic()) {  // TODO: make easier
                    declInfo.FunctionDecl->SetVariadic();
                }
                declInfo.FunctionDecl->SetParams(std::move(paramTypesInfo.ParamDecls));

                declInfo.HeadType = funQualType;
                declInfo.TailType = declInfo.HeadType;
                return declInfo;
            }

            auto declTailQualType = declInfo.TailType;
            auto tailNodeType = dynamic_cast<INodeType*>(declTailQualType->GetSubType());
            if (tailNodeType->IsFunctionType()) {
                // TODO: handle error
            }
            if (tailNodeType->IsArrayType()) {
                // TODO: handle error
            }

            tailNodeType->SetSubType(funQualType);
            declInfo.TailType = funQualType;
            return declInfo;
        }

        return nullptr;
    }

public:
    std::any visitPointer(CParser::PointerContext *ctx) override {
        auto pointerInfo = PointerInfo{};
        auto pointerCtx = ctx->pointer();
        if (pointerCtx) {
            auto pointerInfoAny = visitPointer(pointerCtx);
            pointerInfo = std::any_cast<PointerInfo>(pointerInfoAny);
        }
        
        auto ptrType = m_Program.CreateType<PointerType>(pointerInfo.HeadType);
        auto ptrQualType = m_Program.CreateType<QualType>(ptrType);
        
        auto qualCtx = ctx->qual;
        if (qualCtx) {
            auto typeQualifiersAny = visitTypeQualifierList(qualCtx);
            auto typeQualifiers = std::any_cast<TypeQualifiers>(typeQualifiersAny);
            if (typeQualifiers.Const) {
                ptrQualType->AddConst();
            }
            if (typeQualifiers.Restrict) {
                ptrQualType->AddRestrict();
            }
            if (typeQualifiers.Volatile) {
                ptrQualType->AddVolatile();
            }
        }

        pointerInfo.HeadType = ptrQualType;
        if (!pointerInfo.TailType) {
            pointerInfo.TailType = ptrQualType;
        }

        return pointerInfo;
    }

public:
    struct TypeQualifiers {
        bool Const = false;
        bool Restrict = false;
        bool Volatile = false;
    };

public:
    std::any visitTypeQualifierList(CParser::TypeQualifierListContext *ctx) override {
        auto typeQualifiers = TypeQualifiers{};

        auto qualifiersCtx = ctx->qualifiers;
        for (auto qualifierCtx : qualifiersCtx) {
            auto qualifierAny = visitTypeQualifier(qualifierCtx);
            auto qualifier = std::any_cast<Qualifier>(qualifierAny);
            if (qualifier == Qualifier::kConst) {
                if (typeQualifiers.Const) {
                    // TODO: handle error
                }
                typeQualifiers.Const = true;
            } else if (qualifier == Qualifier::kRestrict) {
                if (typeQualifiers.Restrict) {
                    // TODO: handle error
                }
                typeQualifiers.Restrict = true;
            } else if (qualifier == Qualifier::kVolatile) {
                if (typeQualifiers.Volatile) {
                    // TODO: handle error
                }
                typeQualifiers.Volatile = true;
            }
        }

        return typeQualifiers;
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
        auto firstDecl = std::any_cast<VariableDeclaration*>(firstDeclAny);
        std::vector<VariableDeclaration*> paramList{firstDecl};

        paramList.reserve(ctx->declTail.size() + 1);
        for (auto declCtx : ctx->declTail) {
            auto declAny = visitParameterDeclaration(declCtx);
            auto decl = std::any_cast<VariableDeclaration*>(declAny);
            paramList.push_back(decl);
        }

        return paramList;
    }

    std::any visitParameterDeclaration(CParser::ParameterDeclarationContext *ctx) override {
        auto declSpecsAny = visitDeclarationSpecifiers(ctx->declspecs);
        auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

        auto declList = std::vector<DeclaratorInfo>{};
        if (ctx->decl) {
            auto declInfoAny = visitDeclarator(ctx->decl);
            auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
            declList.push_back(declInfo);
        }

        if (ctx->abstrdecl) {
            auto abstrDeclInfoAny = visitAbstractDeclarator(ctx->abstrdecl);
            auto abstrDeclInfo = std::any_cast<AbstractDeclaratorInfo>(abstrDeclInfoAny);

            auto declInfo = DeclaratorInfo{
                .Identifier = "",
                .HeadType = abstrDeclInfo.HeadType,
                .TailType = abstrDeclInfo.TailType,
                .FunctionDecl = nullptr,
            };

            declList.push_back(declInfo);
        }

        auto declarationInfo = createDeclaration(declSpecs, declList);

        // TODO: handle declarationInfo.TagPreDecl
        auto decl = declarationInfo.Decl;
        if (!decl) {
            auto varDecl = m_Program.CreateAstNode<VariableDeclaration>();
            varDecl->SetType(declSpecs.Type);
            return varDecl;
        }

        if (!decl->IsValueDecl()) {
            // TODO: handle error
        }

        return static_cast<VariableDeclaration*>(decl);
    }

    std::any visitTypeName(CParser::TypeNameContext *ctx) override {
        auto declSpecsAny = visitSpecifierQualifierList(ctx->specqual);
        auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

        auto qualType = declSpecs.Type;
        auto abstrDeclCtx = ctx->abstrdecl;
        auto abstrDeclInfo = AbstractDeclaratorInfo{};
        if (abstrDeclCtx) {
            auto abstrDeclInfoAny = visitAbstractDeclarator(abstrDeclCtx);
            abstrDeclInfo = std::any_cast<AbstractDeclaratorInfo>(abstrDeclInfoAny);
        }

        if (abstrDeclInfo.TailType) {
            auto tailQualType = abstrDeclInfo.TailType;
            auto tailType = dynamic_cast<INodeType*>(tailQualType->GetSubType());
            tailType->SetSubType(qualType);
        } else {  // declspecs only
            abstrDeclInfo.HeadType = qualType;
            abstrDeclInfo.TailType = abstrDeclInfo.HeadType;
        }

        return abstrDeclInfo.HeadType;
    }

    std::any visitAbstractDeclarator(CParser::AbstractDeclaratorContext *ctx) override {
        auto ptrInfo = PointerInfo{};
        auto ptrCtx = ctx->pointer();
        if (ptrCtx) {
            auto ptrInfoAny = visitPointer(ptrCtx);
            ptrInfo = std::any_cast<PointerInfo>(ptrInfoAny);
        }
        
        auto abstrDeclInfo = AbstractDeclaratorInfo{};
        if (ctx->decl) {
            auto abstrDeclInfoAny = visitDirectAbstractDeclarator(ctx->decl);
            abstrDeclInfo = std::any_cast<AbstractDeclaratorInfo>(abstrDeclInfoAny);
            auto tailQualType = abstrDeclInfo.TailType;
            auto tailType = dynamic_cast<INodeType*>(tailQualType->GetSubType());
            tailType->SetSubType(ptrInfo.HeadType);
        } else {
            abstrDeclInfo.HeadType = ptrInfo.HeadType;
        }
        abstrDeclInfo.TailType = ptrInfo.TailType;

        return abstrDeclInfo;
    }

    std::any visitDirectAbstractDeclarator(CParser::DirectAbstractDeclaratorContext *ctx) override {
        if (ctx->nested) {
            auto declInfoAny = visitAbstractDeclarator(ctx->nested);
            auto declInfo = std::any_cast<AbstractDeclaratorInfo>(declInfoAny);
            return declInfo;
        }

        if (ctx->arrdecl || ctx->leafarr) {
            auto declInfo = AbstractDeclaratorInfo{};
            if (ctx->arrdecl) {
                auto declInfoAny = visitDirectAbstractDeclarator(ctx->arrdecl);
                declInfo = std::any_cast<AbstractDeclaratorInfo>(declInfoAny);
            }

            auto arraySize = IntValue(0);  // NB: check init later
            if (ctx->sizeexpr) {
                // TODO: evaluate const expression
            }

            auto arrType = m_Program.CreateType<ArrayType>(nullptr, arraySize);
            auto arrQualType = m_Program.CreateType<QualType>(arrType);

            if (ctx->leafarr) {  // []
                declInfo.HeadType = arrQualType;
                declInfo.TailType = declInfo.HeadType;
                return declInfo;
            }

            auto declTailQualType = declInfo.TailType;
            auto tailNodeType = dynamic_cast<INodeType*>(declTailQualType->GetSubType());
            if (tailNodeType->IsFunctionType()) {
                // TODO: handle error
            }

            tailNodeType->SetSubType(arrQualType);
            declInfo.TailType = arrQualType;
            return declInfo;
        }

        if (ctx->fundecl || ctx->leaffun) {
            auto declInfo = AbstractDeclaratorInfo{};
            if (ctx->fundecl) {
                auto declInfoAny = visitDirectAbstractDeclarator(ctx->fundecl);
                declInfo = std::any_cast<AbstractDeclaratorInfo>(declInfoAny);
            }
    
            auto funType = m_Program.CreateType<FunctionType>(nullptr);
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

            if (ctx->leaffun) {  // ()
                declInfo.HeadType = funQualType;
                declInfo.TailType = declInfo.HeadType;
                return declInfo;
            }

            auto declTailQualType = declInfo.TailType;
            auto tailNodeType = dynamic_cast<INodeType*>(declTailQualType->GetSubType());
            if (tailNodeType->IsFunctionType()) {
                // TODO: handle error
            }
            if (tailNodeType->IsArrayType()) {
                // TODO: handle error
            }

            tailNodeType->SetSubType(funQualType);
            declInfo.TailType = funQualType;
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
            labelStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(labelStmt);
        }

        if (ctx->Case()) {
            auto constExprAny = visitConstantExpression(ctx->constantExpression());
            auto constExpr = std::any_cast<ConstExpression*>(constExprAny);

            auto bodyAny = visitStatement(ctx->statement());
            auto body = std::any_cast<Statement*>(bodyAny);

            auto caseStmt = m_Program.CreateAstNode<CaseStatement>(constExpr, body);
            caseStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(caseStmt);
        }
        
        if (ctx->Default()) {
            auto bodyAny = visitStatement(ctx->statement());
            auto body = std::any_cast<Statement*>(bodyAny);

            auto defaultStmt = m_Program.CreateAstNode<DefaultStatement>(body);
            defaultStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(defaultStmt);
        }

        return nullptr;
    }

    std::any visitCompoundStatement(CParser::CompoundStatementContext *ctx) override {
        if (!ctx->blockItemList()) {
            auto compoundStmt = m_Program.CreateAstNode<CompoundStatement>();
            compoundStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(compoundStmt);
        }

        auto compoundStmtAny = visitBlockItemList(ctx->blockItemList());
        auto compoundStmt = std::any_cast<Statement*>(compoundStmtAny);
        return static_cast<Statement*>(compoundStmt);
    }

    std::any visitBlockItemList(CParser::BlockItemListContext *ctx) override {
        auto compoundStmt = m_Program.CreateAstNode<CompoundStatement>();
        // TODO: block item list context --> compound context
        compoundStmt->SetLocation(createASTLocation(ctx));
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
            auto declarationInfoAny = visitDeclaration(ctx->declaration());
            auto declarationInfo = std::any_cast<DeclarationInfo>(declarationInfoAny);
            auto decls = std::vector<Declaration*>{};
            if (declarationInfo.TagPreDecl) {
                decls.push_back(declarationInfo.TagPreDecl);
            }
            decls.push_back(declarationInfo.Decl);
            auto declStatement = m_Program.CreateAstNode<DeclStatement>(decls);
            declStatement->SetLocation(createASTLocation(ctx));
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
            ifStmt->SetLocation(createASTLocation(ctx));
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
            switchStmt->SetLocation(createASTLocation(ctx));
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
            whileStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(whileStmt);
        }
        
        if (ctx->loop->getType() == CParser::Do) {
            auto conditionAny = visitExpression(ctx->cond);
            auto condition = std::any_cast<Expression*>(conditionAny);

            auto bodyAny = visitStatement(ctx->stmt);
            auto body = std::any_cast<Statement*>(bodyAny);

            auto doStmt = m_Program.CreateAstNode<DoStatement>(condition, body);
            doStmt->SetLocation(createASTLocation(ctx));
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
            forStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(forStmt);
        }

        return nullptr;
    }

    std::any visitForCondition(CParser::ForConditionContext *ctx) override {
        Statement* initStatement = nullptr;
        if (ctx->decl) {
            auto declarationInfoAny = visitForDeclaration(ctx->decl);
            auto declarationInfo = std::any_cast<DeclarationInfo>(declarationInfoAny);
            auto decls = std::vector<Declaration*>{};
            if (declarationInfo.TagPreDecl) {
                decls.push_back(declarationInfo.TagPreDecl);
            }
            decls.push_back(declarationInfo.Decl);
            initStatement = m_Program.CreateAstNode<DeclStatement>(decls);
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
        auto declSpecsAny = visitDeclarationSpecifiers(ctx->declspecs);
        auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

        auto initDeclList = std::vector<InitDeclaratorInfo>{};
        if (ctx->initdecl) {
            auto initDeclListAny = visitInitDeclaratorList(ctx->initdecl);
            initDeclList = std::any_cast<std::vector<InitDeclaratorInfo>>(initDeclListAny);
        }

        auto declarationInfo = createDeclaration(declSpecs, initDeclList);
        return declarationInfo;
    }

    std::any visitForExpression(CParser::ForExpressionContext *ctx) override {
        auto exprList = m_Program.CreateAstNode<ExpressionList>();
        exprList->SetLocation(createASTLocation(ctx));

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
            gotoStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(gotoStmt);
        }

        if (ctx->Continue()) {
            auto continueStmt = m_Program.CreateAstNode<LoopJumpStatement>(LoopJumpStatement::Type::kContinue);
            continueStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(continueStmt);
        }

        if (ctx->Break()) {
            auto breakStmt = m_Program.CreateAstNode<LoopJumpStatement>(LoopJumpStatement::Type::kBreak);
            breakStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(breakStmt);
        }

        if (ctx->Return()) {
            Expression* expression = nullptr;
            if (ctx->expression()) {
                auto expressionAny = visitExpression(ctx->expression());
                expression = std::any_cast<Expression*>(expressionAny);
            }
            auto returnStmt = m_Program.CreateAstNode<ReturnStatement>(expression);
            returnStmt->SetLocation(createASTLocation(ctx));
            return static_cast<Statement*>(returnStmt);
        }
    
        return nullptr;
    }

    std::any visitTranslationUnit(CParser::TranslationUnitContext *ctx) override {
        auto translationUnit = m_Program.CreateAstNode<TranslationUnit>();
        translationUnit->SetLocation(createASTLocation(ctx));
        for (auto declCtx : ctx->decls) {
            auto declarationAny = visitExternalDeclaration(declCtx);
            auto declarationInfo = std::any_cast<DeclarationInfo>(declarationAny);
            if (declarationInfo.TagPreDecl) {
                translationUnit->AddDeclaration(declarationInfo.TagPreDecl);
            }
            if (declarationInfo.Decl) {
                translationUnit->AddDeclaration(declarationInfo.Decl);
            }
        }

        m_Program.SetTranslationUnit(translationUnit);

        return nullptr;
    }

    // Dispatch
    std::any visitExternalDeclaration(CParser::ExternalDeclarationContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitFunctionDefinition(CParser::FunctionDefinitionContext *ctx) override {
        auto declSpecsAny = visitDeclarationSpecifiers(ctx->declspecs);
        auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

        auto declList = std::vector<DeclaratorInfo>{};
        if (ctx->decl) {
            auto declInfoAny = visitDeclarator(ctx->decl);
            auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
            declList.push_back(declInfo);
        }

        auto declarationInfo = createDeclaration(declSpecs, declList);

        auto decl = declarationInfo.Decl;
        if (!decl) {
            // TODO: handle error
        }

        auto functionDecl = dynamic_cast<FunctionDeclaration*>(decl);
        if (!functionDecl) {
            // TODO: handle error
        }

        auto bodyAny = visitCompoundStatement(ctx->body);
        auto body = std::any_cast<Statement*>(bodyAny);
        functionDecl->SetBody(body);

        return declarationInfo;
    }

private:
    Location createASTLocation(antlr4::ParserRuleContext* ctx) {
        auto startToken = ctx->getStart();
        size_t startLine = startToken->getLine();
        size_t startColumn = startToken->getCharPositionInLine();

        auto stopToken = ctx->getStop();
        size_t stopLine = startToken->getLine();
        size_t stopColumn = startToken->getCharPositionInLine();

        auto inputStream = startToken->getInputStream();
        auto sourceName = inputStream->getSourceName();

        auto start = Position(sourceName, startLine, startColumn);
        auto stop = Position(std::move(sourceName), stopLine, stopColumn);
        return Location(std::move(start), std::move(stop));
    }

private:
    Program& m_Program;
};

};
