#include <Ancl/Visitor/BuildAstVisitor.hpp>

using namespace ast;


namespace anclgrammar {

// Returns Expression*
std::any BuildAstVisitor::visitPrimaryExpression(CParser::PrimaryExpressionContext* ctx) {
    if (ctx->Identifier()) {
        std::string name = ctx->getText();
        auto* valueDecl = m_Program.CreateAstNode<ValueDeclaration>(name);
        auto* declExpression = m_Program.CreateAstNode<DeclRefExpression>(valueDecl);
        declExpression->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Expression*>(declExpression);
    }
    if (ctx->numberConstant()) {
        std::any numberAny = visitNumberConstant(ctx->numberConstant());
        return std::any_cast<Expression*>(numberAny);
    }
    if (ctx->expression()) {
        return visitExpression(ctx->expression());
    }

    std::vector<antlr4::tree::TerminalNode*> stringLiterals = ctx->StringLiteral();
    // TODO: Handle multiple string literals
    std::string literalWithQuotes = stringLiterals[0]->getText();
    std::string literal = literalWithQuotes.substr(1, literalWithQuotes.size() - 2);
    auto* stringExpr = m_Program.CreateAstNode<StringExpression>(literal);
    stringExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(stringExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitNumberConstant(CParser::NumberConstantContext* ctx) {
    std::string str = ctx->getText();
    if (ctx->IntegerConstant()) {
        errno = 0;
        char* endPtr = nullptr;
        const char* integerPtr = str.c_str();
        long integer = std::strtol(integerPtr, &endPtr, 0);
        if (integerPtr == endPtr) {
            printSemanticError(std::format("invalid integer constant \"{}\"", str),
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (errno == ERANGE) {
            printSemanticError(std::format("integer constant \"{}\" is out of range", str),
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (errno != 0 && integer == 0) {
            printSemanticError(std::format("invalid integer constant \"{}\"", str),
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (errno == 0 && *endPtr == 0) {
            // TODO: Handle suffix
            Expression* intExpr = m_Program.CreateAstNode<IntExpression>(IntValue(integer));
            intExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
            return intExpr;
        }

        printSemanticError(std::format("invalid integer constant \"{}\"", str),
                           m_LocationBuilder.CreateASTLocation(ctx));
    } else if (ctx->FloatingConstant()) {
        errno = 0;
        char* endPtr = nullptr;
        const char* realPtr = str.c_str();
        double real = std::strtod(realPtr, &endPtr);
        if (realPtr == endPtr) {
            printSemanticError(std::format("invalid real number constant \"{}\"", str),
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (errno == ERANGE) {
            printSemanticError(std::format("real number constant \"{}\" is out of range", str),
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (errno != 0 && real == 0) {
            printSemanticError(std::format("invalid real number constant \"{}\"", str),
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (errno == 0 && *endPtr == 0) {
            // TODO: Handle suffix
            Expression* floatExpr = m_Program.CreateAstNode<FloatExpression>(FloatValue(real));
            floatExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
            return floatExpr;
        }

        printSemanticError(std::format("invalid real number constant \"{}\"", str),
                           m_LocationBuilder.CreateASTLocation(ctx));
    } else if (ctx->CharacterConstant()) {
        // TODO: Handle multichar
        Expression* charExpr = m_Program.CreateAstNode<CharExpression>(str[1]);
        charExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return charExpr;
    }
    return nullptr;
}

// Returns Expression*
std::any BuildAstVisitor::visitPostfixExpression(CParser::PostfixExpressionContext* ctx) {
    if (ctx->primaryExpression()) {
        return visitPrimaryExpression(ctx->primaryExpression());
    }

    std::any leftExpressionAny = visitPostfixExpression(ctx->postfixExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    if (ctx->callee) {
        std::vector<Expression*> argsExpression;
        if (ctx->args) {
            std::any argsExpressionAny = visitArgumentExpressionList(ctx->args);
            argsExpression = std::any_cast<std::vector<Expression*>>(argsExpressionAny);
        }
        auto* callExpr = m_Program.CreateAstNode<CallExpression>(
            leftExpression, argsExpression
        );
        callExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Expression*>(callExpr);      
    }

    if (ctx->array) {
        auto* indexExpression = std::any_cast<Expression*>(visitExpression(ctx->index));
        auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, indexExpression,
            BinaryExpression::OpType::kArrSubscript
        );
        binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    if (ctx->tag) {
        std::string name = ctx->Identifier()->getText();
        auto* memberDecl = m_Program.CreateAstNode<FieldDeclaration>(name);
        auto* memberExpression = m_Program.CreateAstNode<DeclRefExpression>(memberDecl);
        memberExpression->SetLocation(m_LocationBuilder.CreateASTLocation(ctx->Identifier()));

        auto op = BinaryExpression::OpType::kNone;
        if (ctx->member->getText() == ".") {
            op = BinaryExpression::OpType::kDirectMember;
        } else if (ctx->member->getText() == "->") {
            op = BinaryExpression::OpType::kArrowMember;
        }

        auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
            leftExpression, memberExpression, op
        );
        binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Expression*>(binaryExpr);
    }

    auto op = UnaryExpression::OpType::kNone;
    if (ctx->inc) {
        op = UnaryExpression::OpType::kPostInc;
    } else if (ctx->dec) {
        op = UnaryExpression::OpType::kPostDec;
    }

    auto* unaryExpr = m_Program.CreateAstNode<UnaryExpression>(
        leftExpression, op
    );
    unaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(unaryExpr);   
}

// Returns std::vector<Expression*>
std::any BuildAstVisitor::visitArgumentExpressionList(CParser::ArgumentExpressionListContext* ctx) {
    std::any firstArgAny = visitAssignmentExpression(ctx->expr);
    std::vector<Expression*> argsExpression = {std::any_cast<Expression*>(firstArgAny)};
    argsExpression.reserve(ctx->exprTail.size() + 1);

    for (CParser::AssignmentExpressionContext* argCtx : ctx->exprTail) {
        std::any argExpressionAny = visitAssignmentExpression(argCtx);
        auto* argExpression = std::any_cast<Expression*>(argExpressionAny);
        argsExpression.push_back(argExpression);
    }

    return argsExpression;
}

// Returns Expression*
std::any BuildAstVisitor::visitUnaryExpression(CParser::UnaryExpressionContext* ctx) {
    if (ctx->unaryExpressionTail()) {
        return visitUnaryExpressionTail(ctx->unaryExpressionTail());
    } 

    std::any expressionAny = visitUnaryExpression(ctx->unaryExpression());
    auto* expression = std::any_cast<Expression*>(expressionAny);

    auto op = UnaryExpression::OpType::kNone;
    if (ctx->inc) {
        op = UnaryExpression::OpType::kPreInc;
    } else if (ctx->dec) {
        op = UnaryExpression::OpType::kPreDec;
    } else if (ctx->size) {
        op = UnaryExpression::OpType::kSizeof;
    }

    auto* unaryExpr = m_Program.CreateAstNode<UnaryExpression>(
        expression, op
    );
    unaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(unaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitUnaryExpressionTail(CParser::UnaryExpressionTailContext* ctx) {
    if (ctx->postfixExpression()) {
        return visitPostfixExpression(ctx->postfixExpression());
    }

    if (ctx->unaryOperator()) {
        std::any expressionAny = visitCastExpression(ctx->castExpression());
        auto* expression = std::any_cast<Expression*>(expressionAny);

        std::string opStr = ctx->unaryOperator()->getText();
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

        auto* unaryExpr = m_Program.CreateAstNode<UnaryExpression>(
            expression, op
        );
        unaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Expression*>(unaryExpr);
    }

    std::any typeNameAny = visitTypeName(ctx->typeName());
    auto typeNameQualType = std::any_cast<QualType>(typeNameAny);

    auto* sizeofExpr = m_Program.CreateAstNode<SizeofTypeExpression>(typeNameQualType);
    sizeofExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(sizeofExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitCastExpression(CParser::CastExpressionContext* ctx) {
    if (ctx->unaryExpression()) {
        return visitUnaryExpression(ctx->unaryExpression());
    }

    std::any typeNameAny = visitTypeName(ctx->typeName());
    auto typeNameQualType = std::any_cast<QualType>(typeNameAny);

    std::any subExpressionAny = visitCastExpression(ctx->castExpression());
    auto* subExpression = std::any_cast<Expression*>(subExpressionAny);

    auto* castExpr = m_Program.CreateAstNode<CastExpression>(subExpression, typeNameQualType);
    castExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(castExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext* ctx) {
    if (ctx->castexpr) {
        return visitCastExpression(ctx->castexpr);
    }

    std::any leftExpressionAny = visitMultiplicativeExpression(ctx->multiplicativeExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitCastExpression(ctx->castExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    auto op = BinaryExpression::OpType::kNone;
    if (ctx->mul) {
        op = BinaryExpression::OpType::kMul;
    } else if (ctx->div) {
        op = BinaryExpression::OpType::kDiv;
    } else if (ctx->rem) {
        op = BinaryExpression::OpType::kRem;
    }

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, op
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitAdditiveExpression(CParser::AdditiveExpressionContext* ctx) {
    if (ctx->mulexpr) {
        return visitMultiplicativeExpression(ctx->mulexpr);
    }

    std::any leftExpressionAny = visitAdditiveExpression(ctx->additiveExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitMultiplicativeExpression(ctx->multiplicativeExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    auto op = BinaryExpression::OpType::kNone;
    if (ctx->add) {
        op = BinaryExpression::OpType::kAdd;
    } else if (ctx->sub) {
        op = BinaryExpression::OpType::kSub;
    }

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, op
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitShiftExpression(CParser::ShiftExpressionContext* ctx) {
    if (ctx->addexpr) {
        return visitAdditiveExpression(ctx->addexpr);
    }

    std::any leftExpressionAny = visitShiftExpression(ctx->shiftExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitAdditiveExpression(ctx->additiveExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    auto op = BinaryExpression::OpType::kNone;
    if (ctx->shiftl) {
        op = BinaryExpression::OpType::kShiftL;
    } else if (ctx->shiftr) {
        op = BinaryExpression::OpType::kShiftR;
    }

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, op
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitRelationalExpression(CParser::RelationalExpressionContext* ctx) {
    if (ctx->shiftexpr) {
        return visitShiftExpression(ctx->shiftexpr);
    }

    std::any leftExpressionAny = visitRelationalExpression(ctx->relationalExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitShiftExpression(ctx->shiftExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

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

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, op
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitEqualityExpression(CParser::EqualityExpressionContext* ctx) {
    if (ctx->relexpr) {
        return visitRelationalExpression(ctx->relexpr);
    }

    std::any leftExpressionAny = visitEqualityExpression(ctx->equalityExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitRelationalExpression(ctx->relationalExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    auto op = BinaryExpression::OpType::kNone;
    if (ctx->equal) {
        op = BinaryExpression::OpType::kEqual;
    } else if (ctx->nequal) {
        op = BinaryExpression::OpType::kNEqual;
    }

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, op
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitAndExpression(CParser::AndExpressionContext* ctx) {
    if (ctx->eqexpr) {
        return visitEqualityExpression(ctx->eqexpr);
    }

    std::any leftExpressionAny = visitAndExpression(ctx->andExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitEqualityExpression(ctx->equalityExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, BinaryExpression::OpType::kAnd
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitExclusiveOrExpression(CParser::ExclusiveOrExpressionContext* ctx) {
    if (ctx->andexpr) {
        return visitAndExpression(ctx->andexpr);
    }

    std::any leftExpressionAny = visitExclusiveOrExpression(ctx->exclusiveOrExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitAndExpression(ctx->andExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, BinaryExpression::OpType::kXor
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitInclusiveOrExpression(CParser::InclusiveOrExpressionContext* ctx) {
    if (ctx->exorexpr) {
        return visitExclusiveOrExpression(ctx->exorexpr);
    }

    std::any leftExpressionAny = visitInclusiveOrExpression(ctx->inclusiveOrExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitExclusiveOrExpression(ctx->exclusiveOrExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, BinaryExpression::OpType::kOr
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitLogicalAndExpression(CParser::LogicalAndExpressionContext* ctx) {
    if (ctx->incorexpr) {
        return visitInclusiveOrExpression(ctx->incorexpr);
    }

    std::any leftExpressionAny = visitLogicalAndExpression(ctx->logicalAndExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitInclusiveOrExpression(ctx->inclusiveOrExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, BinaryExpression::OpType::kLogAnd
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitLogicalOrExpression(CParser::LogicalOrExpressionContext* ctx) {
    if (ctx->logandexpr) {
        return visitLogicalAndExpression(ctx->logandexpr);
    }

    std::any leftExpressionAny = visitLogicalOrExpression(ctx->logicalOrExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitLogicalAndExpression(ctx->logicalAndExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, BinaryExpression::OpType::kLogOr
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitConditionalExpression(CParser::ConditionalExpressionContext* ctx) {
    if (ctx->logorexpr) {
        return visitLogicalOrExpression(ctx->logorexpr);
    }

    std::any conditionAny = visitLogicalOrExpression(ctx->logicalOrExpression());
    auto* condition = std::any_cast<Expression*>(conditionAny);

    std::any trueExpressionAny = visitExpression(ctx->expression());
    auto* trueExpression = std::any_cast<Expression*>(trueExpressionAny);

    std::any falseExpressionAny = visitConditionalExpression(ctx->conditionalExpression());
    auto* falseExpression = std::any_cast<Expression*>(falseExpressionAny);

    auto* condExpr = m_Program.CreateAstNode<ConditionalExpression>(
        condition, trueExpression, falseExpression
    );
    condExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(condExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitAssignmentExpression(CParser::AssignmentExpressionContext* ctx) {
    if (ctx->condexpr) {
        return visitConditionalExpression(ctx->condexpr);
    }

    std::any leftExpressionAny = visitUnaryExpression(ctx->unaryExpression());
    auto* leftExpression = std::any_cast<Expression*>(leftExpressionAny);

    std::any rightExpressionAny = visitAssignmentExpression(ctx->assignmentExpression());
    auto* rightExpression = std::any_cast<Expression*>(rightExpressionAny);

    std::string opStr = ctx->assignmentOperator()->getText();
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

    auto* binaryExpr = m_Program.CreateAstNode<BinaryExpression>(
        leftExpression, rightExpression, op
    );
    binaryExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return static_cast<Expression*>(binaryExpr);
}

// Returns Expression*
std::any BuildAstVisitor::visitExpression(CParser::ExpressionContext* ctx) {
    auto* exprList = m_Program.CreateAstNode<ExpressionList>();
    exprList->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));

    std::any firstExpressionAny = visitAssignmentExpression(ctx->expr);
    auto* firstExpression = std::any_cast<Expression*>(firstExpressionAny);
    exprList->AddExpression(firstExpression);

    for (auto exprCtx : ctx->exprTail) {
        std::any expressionAny = visitAssignmentExpression(exprCtx);
        auto* expression = std::any_cast<Expression*>(expressionAny);
        exprList->AddExpression(expression);
    }

    return static_cast<Expression*>(exprList);
}

// Returns ConstExpression*
std::any BuildAstVisitor::visitConstantExpression(CParser::ConstantExpressionContext* ctx) {
    std::any expressionAny = visitConditionalExpression(ctx->conditionalExpression());
    auto* expression = std::any_cast<Expression*>(expressionAny);

    auto* constExpr = m_Program.CreateAstNode<ConstExpression>(expression);
    constExpr->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return constExpr;
}

// Returns DeclarationInfo
std::any BuildAstVisitor::visitDeclaration(CParser::DeclarationContext* ctx) {
    std::any declSpecsAny = visitDeclarationSpecifiers(ctx->declspecs);
    auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

    std::vector<InitDeclaratorInfo> initDeclList;
    if (ctx->initdecl) {
        std::any initDeclListAny = visitInitDeclaratorList(ctx->initdecl);
        initDeclList = std::any_cast<std::vector<InitDeclaratorInfo>>(initDeclListAny);
    }

    DeclarationInfo declarationInfo = createDeclaration(declSpecs, initDeclList, ctx);
    if (!declarationInfo.TagPreDecl && !declarationInfo.Decl) {
        printSemanticWarning("declaration does not declare anything",
                             m_LocationBuilder.CreateASTLocation(ctx));
    }

    return declarationInfo;
}

// Returns DeclSpecifiers
std::any BuildAstVisitor::visitDeclarationSpecifiers(CParser::DeclarationSpecifiersContext* ctx) {
    std::vector<CParser::DeclarationSpecifierContext*> specsCtx = ctx->specs;
    DeclSpecifiers declSpecifiersInfo;
    declSpecifiersInfo.Type = QualType{};

    for (CParser::DeclarationSpecifierContext* specCtx : specsCtx) {
        std::any declSpecAny = visitDeclarationSpecifier(specCtx);
        updateDeclSpecs(declSpecifiersInfo, declSpecAny, specCtx);
    }

    QualType& qualType = declSpecifiersInfo.Type;
    if (!qualType.HasSubType()) {
        Type* builtinType = createBuiltinTypeFromInfo(declSpecifiersInfo.BuiltinType, ctx);
        if (builtinType) {
            qualType.SetSubType(builtinType);
        } else {
            printSemanticError("type specifier missing", m_LocationBuilder.CreateASTLocation(ctx));
        }
    }

    return declSpecifiersInfo;
}

// Returns StorageClassInfo | BuiltinTypeSpecifier | Type* | Qualifier | FunctionSpecifier
std::any BuildAstVisitor::visitDeclarationSpecifier(CParser::DeclarationSpecifierContext* ctx) {
    if (ctx->storage) {
        std::any storageClassAny = visitStorageClassSpecifier(ctx->storage);
        auto storageClass = std::any_cast<StorageClassInfo>(storageClassAny);
        return storageClass;
    }

    if (ctx->typespec) {
        std::any typeSpecifierAny = visitTypeSpecifier(ctx->typespec);
        // Type(RecordType | EnumType | TypedefType) | BuiltinTypeSpecifier

        auto* builtinTypeSpecOpt = std::any_cast<BuiltinTypeSpecifier>(&typeSpecifierAny);
        if (builtinTypeSpecOpt) {
            return *builtinTypeSpecOpt;
        }

        auto* namedTypeSpecifier = std::any_cast<Type*>(typeSpecifierAny);
        return namedTypeSpecifier;
    }

    if (ctx->qualifier) {
        std::any typeQualifierAny = visitTypeQualifier(ctx->qualifier);
        auto typeQualifier = std::any_cast<Qualifier>(typeQualifierAny);
        return typeQualifier;
    }

    if (ctx->funcspec) {
        std::any funcSpecifierAny = visitFunctionSpecifier(ctx->funcspec);
        auto funcSpecifier = std::any_cast<FunctionSpecifier>(funcSpecifierAny);
        return funcSpecifier;
    }

    return nullptr;
}

// Returns std::vector<InitDeclaratorInfo>
std::any BuildAstVisitor::visitInitDeclaratorList(CParser::InitDeclaratorListContext* ctx) {
    std::any firstDeclInfoAny = visitInitDeclarator(ctx->init);
    auto firstDeclInfo = std::any_cast<InitDeclaratorInfo>(firstDeclInfoAny);
    std::vector<InitDeclaratorInfo> decls = {firstDeclInfo};

    decls.reserve(ctx->initTail.size() + 1);
    for (CParser::InitDeclaratorContext* declCtx : ctx->initTail) {
        std::any declInfoAny = visitInitDeclarator(declCtx);
        auto declInfo = std::any_cast<InitDeclaratorInfo>(declInfoAny);
        decls.push_back(declInfo);
    }

    return decls;
}

// Returns InitDeclaratorInfo
std::any BuildAstVisitor::visitInitDeclarator(CParser::InitDeclaratorContext* ctx) {
    std::any declInfoAny = visitDeclarator(ctx->decl);
    auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);

    InitDeclaratorInfo initDeclInfo{
        .DeclInfo = std::move(declInfo)
    };

    if (ctx->init) {
        std::any initAny = visitInitializer(ctx->init);
        auto* init = std::any_cast<Expression*>(initAny);
        initDeclInfo.Init = init;
    }

    return initDeclInfo;
}

// Returns StorageClassInfo
std::any BuildAstVisitor::visitStorageClassSpecifier(CParser::StorageClassSpecifierContext* ctx) {
    std::string storageStr = ctx->getText();
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

// Returns BuiltinTypeSpecifier
std::any BuildAstVisitor::visitTypeSpecifier(CParser::TypeSpecifierContext* ctx) {
    if (ctx->recordspec) {
        std::any recordTypeAny = visitStructOrUnionSpecifier(ctx->recordspec);
        auto* recordType = std::any_cast<RecordType*>(recordTypeAny);
        return static_cast<Type*>(recordType);
    }

    if (ctx->enumspec) {
        std::any enumTypeAny = visitEnumSpecifier(ctx->enumspec);
        auto* enumType = std::any_cast<EnumType*>(enumTypeAny);
        return static_cast<Type*>(enumType);
    }

    if (ctx->typedefname) {
        std::string name = ctx->typedefname->getText();
        auto* typedefDecl = m_Program.CreateAstNode<TypedefDeclaration>();
        typedefDecl->SetName(name);
        auto* typedefType = m_Program.CreateType<TypedefType>(typedefDecl);
        return static_cast<Type*>(typedefType);
    }

    std::string typeStr = ctx->type->getText();
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

// Returns RecordType*
std::any BuildAstVisitor::visitStructOrUnionSpecifier(CParser::StructOrUnionSpecifierContext* ctx) {
    bool isUnion = false;
    CParser::StructOrUnionContext* structOrUnionCtx = ctx->structOrUnion();
    if (structOrUnionCtx->getText() == "union") {
        isUnion = true;
    }

    std::string name;
    antlr4::tree::TerminalNode* ident = ctx->Identifier();
    if (ident) {
        name = ident->getText();
    }

    std::vector<FieldInfo> fieldsInfo;
    if (ctx->body) {
        std::any fieldsInfoAny = visitStructDeclarationList(ctx->body);
        fieldsInfo = std::any_cast<std::vector<FieldInfo>>(fieldsInfoAny);
    }

    auto* recordDecl = m_Program.CreateAstNode<RecordDeclaration>(isUnion, ctx->body);
    recordDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));

    for (const FieldInfo& fieldInfo : fieldsInfo) {
        if (fieldInfo.TagPreDecl) {  // Definition of internal tag
            recordDecl->AddInternalTagDecl(fieldInfo.TagPreDecl);
        }
        if (fieldInfo.FieldDecl) {
            recordDecl->AddField(fieldInfo.FieldDecl);
        }
    }

    auto* recordType = m_Program.CreateType<RecordType>(recordDecl);
    QualType recordQualType{recordType};
    recordDecl->SetName(name);
    recordDecl->SetType(recordQualType);

    return recordType;
}

// Returns std::vector<FieldInfo>
std::any BuildAstVisitor::visitStructDeclarationList(CParser::StructDeclarationListContext* ctx) {
    std::vector<FieldInfo> fieldsInfo;
    for (CParser::StructDeclarationContext* declCtx : ctx->decls) {
        std::any declarationInfoAny = visitStructDeclaration(declCtx);
        auto declarationInfo = std::any_cast<DeclarationInfo>(declarationInfoAny);

        FieldDeclaration* fieldDecl = nullptr;
        if (declarationInfo.Decl) {
            auto* varDecl = dynamic_cast<VariableDeclaration*>(declarationInfo.Decl);
            assert(varDecl);

            if (varDecl->GetStorageClass() != StorageClass::kNone) {
                printSemanticError("type name does not allow storage class to be specified",
                                   varDecl->GetLocation());
            }

            fieldDecl = m_Program.CreateAstNode<FieldDeclaration>(varDecl->GetName(),
                                                                  varDecl->GetType());
            fieldDecl->SetLocation(m_LocationBuilder.CreateASTLocation(declCtx));
        }

        FieldInfo fieldInfo{
            .TagPreDecl = declarationInfo.TagPreDecl,
            .FieldDecl = fieldDecl,
        };

        fieldsInfo.push_back(fieldInfo);
    }
    return fieldsInfo;
}

// Returns DeclarationInfo
std::any BuildAstVisitor::visitStructDeclaration(CParser::StructDeclarationContext* ctx) {
    std::any declSpecsAny = visitSpecifierQualifierList(ctx->specqual);
    auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

    std::vector<DeclaratorInfo> declList;
    if (ctx->decls) {
        std::any declListAny = visitStructDeclaratorList(ctx->decls);
        declList = std::any_cast<std::vector<DeclaratorInfo>>(declListAny);
    }

    DeclarationInfo declarationInfo = createDeclaration(declSpecs, declList, ctx);
    if (!declarationInfo.TagPreDecl && !declarationInfo.Decl) {
        printSemanticWarning("declaration does not declare anything",
                             m_LocationBuilder.CreateASTLocation(ctx));
    }

    return declarationInfo;
}

// Returns DeclSpecifiers
std::any BuildAstVisitor::visitSpecifierQualifierList(CParser::SpecifierQualifierListContext* ctx) {
    std::vector<CParser::SpecifierQualifierContext*> specsCtx = ctx->specs;
    DeclSpecifiers declSpecifiersInfo;
    declSpecifiersInfo.Type = QualType{};

    for (CParser::SpecifierQualifierContext* specCtx : specsCtx) {
        std::any declSpecAny = visitSpecifierQualifier(specCtx);
        updateDeclSpecs(declSpecifiersInfo, declSpecAny, specCtx);
    }

    QualType& qualType = declSpecifiersInfo.Type;
    if (!qualType.HasSubType()) {
        Type* builtinType = createBuiltinTypeFromInfo(declSpecifiersInfo.BuiltinType, ctx);
        if (builtinType) {
            qualType.SetSubType(builtinType);
        } else {
            printSemanticError("type specifier missing", m_LocationBuilder.CreateASTLocation(ctx));
        }
    }

    return declSpecifiersInfo;
}

// Returns BuiltinTypeSpecifier | Type* | Qualifier
std::any BuildAstVisitor::visitSpecifierQualifier(CParser::SpecifierQualifierContext* ctx) {
    if (ctx->typespec) {
        std::any typeSpecifierAny = visitTypeSpecifier(ctx->typespec);

        auto* builtinTypeSpecOpt = std::any_cast<BuiltinTypeSpecifier>(&typeSpecifierAny);
        if (builtinTypeSpecOpt) {
            return *builtinTypeSpecOpt;
        }

        auto* namedTypeSpecifier = std::any_cast<Type*>(&typeSpecifierAny);
        return *namedTypeSpecifier;
    }

    if (ctx->qualifier) {
        std::any typeQualifierAny = visitTypeQualifier(ctx->qualifier);
        auto typeQualifier = std::any_cast<Qualifier>(typeQualifierAny);
        return typeQualifier;
    }

    return nullptr;
}

// Returns std::vector<DeclaratorInfo>
std::any BuildAstVisitor::visitStructDeclaratorList(CParser::StructDeclaratorListContext* ctx) {
    std::any firstDeclInfoAny = visitStructDeclarator(ctx->decl);
    auto firstDeclInfo = std::any_cast<DeclaratorInfo>(firstDeclInfoAny);

    std::vector<DeclaratorInfo> declsInfo = {firstDeclInfo};
    auto declTail = ctx->declTail;
    declsInfo.reserve(declTail.size() + 1);
    for (CParser::StructDeclaratorContext* declCtx : declTail) {
        std::any declInfoAny = visitStructDeclarator(declCtx);
        auto declInfo = std::any_cast<DeclaratorInfo>(firstDeclInfoAny);
        declsInfo.push_back(std::move(declInfo));
    }

    return declsInfo;
}

// Returns DeclaratorInfo
std::any BuildAstVisitor::visitStructDeclarator(CParser::StructDeclaratorContext* ctx) {
    DeclaratorInfo declInfo = std::any_cast<DeclaratorInfo>(visitDeclarator(ctx->declarator()));
    if (declInfo.FunctionDecl) {
        printSemanticError("field declared as a function", m_LocationBuilder.CreateASTLocation(ctx));
    }

    return declInfo;
}

// Returns EnumType*
std::any BuildAstVisitor::visitEnumSpecifier(CParser::EnumSpecifierContext* ctx) {
    std::string name;
    antlr4::tree::TerminalNode* ident = ctx->Identifier();
    if (ident) {
        name = ident->getText();
    }

    std::vector<EnumConstDeclaration*> enumerators;
    if (ctx->enumerators) {
        std::any enumeratorsAny = visitEnumeratorList(ctx->enumerators);
        enumerators = std::any_cast<std::vector<EnumConstDeclaration*>>(enumeratorsAny);
        assert(!enumerators.empty());
    }

    auto* enumDecl = m_Program.CreateAstNode<EnumDeclaration>(enumerators, ctx->enumerators);
    enumDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    enumDecl->SetName(name);

    auto* enumType = m_Program.CreateType<EnumType>(enumDecl);
    QualType enumQualType{enumType};
    enumDecl->SetType(enumQualType);

    return enumType;
}

// Returns std::vector<EnumConstDeclaration*>
std::any BuildAstVisitor::visitEnumeratorList(CParser::EnumeratorListContext* ctx) {
    std::any firstEnumAny = visitEnumerator(ctx->firstenum);
    auto* firstEnum = std::any_cast<EnumConstDeclaration*>(firstEnumAny);
    std::vector<EnumConstDeclaration*> enumerators = {firstEnum};

    enumerators.reserve(ctx->enumTail.size() + 1);
    for (CParser::EnumeratorContext* enumeratorCtx : ctx->enumTail) {
        std::any enumeratorAny = visitEnumerator(enumeratorCtx);
        auto* enumerator = std::any_cast<EnumConstDeclaration*>(enumeratorAny);
        enumerators.push_back(enumerator);
    }

    return enumerators;
}

// Returns EnumConstDeclaration*
std::any BuildAstVisitor::visitEnumerator(CParser::EnumeratorContext* ctx) {
    CParser::EnumerationConstantContext* ident = ctx->ident;
    std::string name = ident->getText();

    ConstExpression* init = nullptr;
    if (ctx->init) {
        std::any initAny = visitConstantExpression(ctx->init);
        init = std::any_cast<ConstExpression*>(initAny);
    }

    auto* enumConstDecl = m_Program.CreateAstNode<EnumConstDeclaration>(init);
    enumConstDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    enumConstDecl->SetName(name);

    auto* intType = m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt);
    QualType intQualType{intType};
    enumConstDecl->SetType(intQualType);

    return enumConstDecl;
}

// Returns Qualifier
std::any BuildAstVisitor::visitTypeQualifier(CParser::TypeQualifierContext* ctx) {
    std::string qualifierStr = ctx->getText();
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

// Returns FunctionSpecifier
std::any BuildAstVisitor::visitFunctionSpecifier(CParser::FunctionSpecifierContext* ctx) {
    std::string specStr = ctx->getText();
    if (specStr == "inline") {
        return FunctionSpecifier::kInline;
    }
    return FunctionSpecifier::kNone;
}

// Returns DeclaratorInfo
std::any BuildAstVisitor::visitDeclarator(CParser::DeclaratorContext* ctx) {
    PointerInfo ptrInfo;
    if (CParser::PointerContext* pointerCtx = ctx->pointer()) {
        std::any ptrInfoAny = visitPointer(pointerCtx);
        ptrInfo = std::any_cast<PointerInfo>(ptrInfoAny);
    }

    std::any declaratorAny = visitDirectDeclarator(ctx->directDeclarator());
    auto declarator = std::any_cast<DeclaratorInfo>(declaratorAny);

    if (!ptrInfo.HeadType.HasSubType()) {  // no pointers
        return declarator;
    }

    if (declarator.TailType.HasSubType()) {
        auto* declTailType = dynamic_cast<INodeType*>(declarator.TailType.GetSubType());
        declTailType->SetSubType(ptrInfo.HeadType);
    } else {  // * ident
        declarator.HeadType = ptrInfo.HeadType;
    }
    declarator.TailType = ptrInfo.TailType;

    return declarator;
}

// Returns DeclaratorInfo
std::any BuildAstVisitor::visitDirectDeclarator(CParser::DirectDeclaratorContext* ctx) {
    antlr4::tree::TerminalNode* identTerm = ctx->Identifier();
    if (identTerm) {
        std::string identifier = identTerm->getText();
        return DeclaratorInfo{ .Identifier = identifier };
    }

    if (ctx->nested) {
        return std::any_cast<DeclaratorInfo>(visitDeclarator(ctx->nested));
    }

    if (ctx->arrdecl) {
        std::any declInfoAny = visitDirectDeclarator(ctx->arrdecl);
        auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);

        // NB: It is calculated in Semantic AST Visitor
        ConstExpression* constExpr = nullptr;
        if (ctx->sizeexpr) {
            std::any constExprAny = visitConstantExpression(ctx->sizeexpr);
            constExpr = std::any_cast<ConstExpression*>(constExprAny);
        }

        ArrayType* arrType = m_Program.CreateType<ArrayType>(nullptr, constExpr);
        QualType arrQualType{arrType};

        if (!declInfo.HeadType.HasSubType()) {  // Ident []
            declInfo.HeadType = arrQualType;
            declInfo.TailType = declInfo.HeadType;
            return declInfo;
        }

        auto* tailNodeType = dynamic_cast<INodeType*>(declInfo.TailType.GetSubType());
        if (tailNodeType->IsFunctionType()) {
            printSemanticError("function cannot return array type",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }

        tailNodeType->SetSubType(arrQualType);
        declInfo.TailType = arrQualType;
        return declInfo;
    }

    if (ctx->fundecl) {
        std::any declInfoAny = visitDirectDeclarator(ctx->fundecl);
        auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);

        auto* funType = m_Program.CreateType<FunctionType>(nullptr);
        QualType funQualType{funType};

        ParamTypesInfo paramTypesInfo;
        if (ctx->paramlist) {
            std::any paramTypesAny = visitParameterTypeList(ctx->paramlist);
            paramTypesInfo = std::any_cast<ParamTypesInfo>(paramTypesAny);
        }

        std::vector<QualType> paramTypes;
        for (ParameterDeclaration* paramDecl : paramTypesInfo.ParamDecls) {
            paramTypes.push_back(paramDecl->GetType());
        }
        funType->SetParamTypes(paramTypes);

        if (paramTypesInfo.HasVariadic) {
            funType->SetVariadic();
        }
        if (!declInfo.HeadType.HasSubType()) {  // is function declaration
            auto* funcDecl = m_Program.CreateAstNode<FunctionDeclaration>();
            funcDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));

            funcDecl->SetName(declInfo.Identifier);
            funcDecl->SetType(funQualType);

            declInfo.FunctionDecl = funcDecl;
            if (funType->IsVariadic()) {  // TODO: Simplify
                declInfo.FunctionDecl->SetVariadic();
            }
            declInfo.FunctionDecl->SetParams(paramTypesInfo.ParamDecls);

            declInfo.HeadType = funQualType;
            declInfo.TailType = declInfo.HeadType;
            return declInfo;
        }

        auto* tailNodeType = dynamic_cast<INodeType*>(declInfo.TailType.GetSubType());
        if (tailNodeType->IsFunctionType()) {
            printSemanticError("function cannot return function type",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (tailNodeType->IsArrayType()) {
            printSemanticError("array cannot have elements of function type",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }

        tailNodeType->SetSubType(funQualType);
        declInfo.TailType = funQualType;
        return declInfo;
    }

    return nullptr;
}

// Returns PointerInfo
std::any BuildAstVisitor::visitPointer(CParser::PointerContext* ctx) {
    PointerInfo pointerInfo;
    if (CParser::PointerContext* pointerCtx = ctx->pointer()) {
        std::any pointerInfoAny = visitPointer(pointerCtx);
        pointerInfo = std::any_cast<PointerInfo>(pointerInfoAny);
    }

    auto* ptrType = m_Program.CreateType<PointerType>(pointerInfo.HeadType);
    QualType ptrQualType{ptrType};
    
    if (CParser::TypeQualifierListContext* qualCtx = ctx->qual) {
        std::any typeQualifiersAny = visitTypeQualifierList(qualCtx);
        auto typeQualifiers = std::any_cast<TypeQualifiers>(typeQualifiersAny);
        if (typeQualifiers.Const) {
            ptrQualType.AddConst();
        }
        if (typeQualifiers.Restrict) {
            ptrQualType.AddRestrict();
        }
        if (typeQualifiers.Volatile) {
            ptrQualType.AddVolatile();
        }
    }

    pointerInfo.HeadType = ptrQualType;
    if (!pointerInfo.TailType.HasSubType()) {
        pointerInfo.TailType = ptrQualType;
    }

    return pointerInfo;
}

// Returns TypeQualifiers
std::any BuildAstVisitor::visitTypeQualifierList(CParser::TypeQualifierListContext* ctx) {
    TypeQualifiers typeQualifiers;

    std::vector<CParser::TypeQualifierContext*> qualifiersCtx = ctx->qualifiers;
    for (CParser::TypeQualifierContext* qualifierCtx : qualifiersCtx) {
        std::any qualifierAny = visitTypeQualifier(qualifierCtx);
        auto qualifier = std::any_cast<Qualifier>(qualifierAny);
        if (qualifier == Qualifier::kConst) {
            if (typeQualifiers.Const) {
                printSemanticWarning("duplicate 'const' declaration specifier",
                                     m_LocationBuilder.CreateASTLocation(qualifierCtx));
            }
            typeQualifiers.Const = true;
        } else if (qualifier == Qualifier::kRestrict) {
            if (typeQualifiers.Restrict) {
                printSemanticWarning("duplicate 'restrict' declaration specifier",
                                     m_LocationBuilder.CreateASTLocation(qualifierCtx));
            }
            typeQualifiers.Restrict = true;
        } else if (qualifier == Qualifier::kVolatile) {
            if (typeQualifiers.Volatile) {
                printSemanticWarning("duplicate 'volatile' declaration specifier",
                                     m_LocationBuilder.CreateASTLocation(qualifierCtx));
            }
            typeQualifiers.Volatile = true;
        }
    }

    return typeQualifiers;
}

// Returns ParamTypesInfo
std::any BuildAstVisitor::visitParameterTypeList(CParser::ParameterTypeListContext* ctx) {
    ParamTypesInfo paramsInfo;
    std::any paramListAny = visitParameterList(ctx->params);
    paramsInfo.ParamDecls = std::any_cast<std::vector<ParameterDeclaration*>>(paramListAny);

    if (ctx->vararg) {
        paramsInfo.HasVariadic = true;
    }

    return paramsInfo;
}

// Returns std::vector<ParameterDeclaration*>
std::any BuildAstVisitor::visitParameterList(CParser::ParameterListContext* ctx) {
    std::any firstDeclAny = visitParameterDeclaration(ctx->decl);
    auto* firstDecl = std::any_cast<ParameterDeclaration*>(firstDeclAny);
    std::vector<ParameterDeclaration*> paramList = {firstDecl};

    paramList.reserve(ctx->declTail.size() + 1);
    for (CParser::ParameterDeclarationContext* declCtx : ctx->declTail) {
        std::any declAny = visitParameterDeclaration(declCtx);
        auto* decl = std::any_cast<ParameterDeclaration*>(declAny);
        paramList.push_back(decl);
    }

    return paramList;
}

// Returns ParameterDeclaration*
std::any BuildAstVisitor::visitParameterDeclaration(CParser::ParameterDeclarationContext* ctx) {
    std::any declSpecsAny = visitDeclarationSpecifiers(ctx->declspecs);
    auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

    std::vector<DeclaratorInfo> declList;
    if (ctx->decl) {
        std::any declInfoAny = visitDeclarator(ctx->decl);
        auto declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
        declList.push_back(declInfo);
    }

    if (ctx->abstrdecl) {
        std::any abstrDeclInfoAny = visitAbstractDeclarator(ctx->abstrdecl);
        auto abstrDeclInfo = std::any_cast<AbstractDeclaratorInfo>(abstrDeclInfoAny);

        DeclaratorInfo declInfo{
            .Identifier = "",
            .HeadType = abstrDeclInfo.HeadType,
            .TailType = abstrDeclInfo.TailType,
            .FunctionDecl = nullptr,
        };

        declList.push_back(declInfo);
    }

    DeclarationInfo declarationInfo = createDeclaration(declSpecs, declList, ctx, /*isParam=*/true);

    if (declarationInfo.TagPreDecl) {
        printSemanticWarning("tag declaration will not be visible outside of function",
                             m_LocationBuilder.CreateASTLocation(ctx->declspecs));
    }

    Declaration* decl = declarationInfo.Decl;
    if (!decl) {
        auto* paramDecl = m_Program.CreateAstNode<ParameterDeclaration>();
        paramDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        paramDecl->SetType(declSpecs.Type);
        return paramDecl;
    }

    assert(dynamic_cast<ParameterDeclaration*>(decl));

    return static_cast<ParameterDeclaration*>(decl);
}

// Returns QualType
std::any BuildAstVisitor::visitTypeName(CParser::TypeNameContext* ctx) {
    std::any declSpecsAny = visitSpecifierQualifierList(ctx->specqual);
    auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

    QualType& qualType = declSpecs.Type;
    AbstractDeclaratorInfo abstrDeclInfo;
    if (ctx->abstrdecl) {
        auto abstrDeclInfoAny = visitAbstractDeclarator(ctx->abstrdecl);
        abstrDeclInfo = std::any_cast<AbstractDeclaratorInfo>(abstrDeclInfoAny);
    }

    if (abstrDeclInfo.TailType.HasSubType()) {
        auto* tailType = dynamic_cast<INodeType*>(abstrDeclInfo.TailType.GetSubType());
        tailType->SetSubType(qualType);
    } else {  // declspecs only
        abstrDeclInfo.HeadType = qualType;
        abstrDeclInfo.TailType = abstrDeclInfo.HeadType;
    }

    return abstrDeclInfo.HeadType;
}

// Returns AbstractDeclaratorInfo
std::any BuildAstVisitor::visitAbstractDeclarator(CParser::AbstractDeclaratorContext* ctx) {
    PointerInfo ptrInfo;
    if (CParser::PointerContext* pointerCtx = ctx->pointer()) {
        std::any ptrInfoAny = visitPointer(pointerCtx);
        ptrInfo = std::any_cast<PointerInfo>(ptrInfoAny);
    }

    AbstractDeclaratorInfo abstrDeclInfo;
    if (ctx->decl) {
        std::any abstrDeclInfoAny = visitDirectAbstractDeclarator(ctx->decl);
        abstrDeclInfo = std::any_cast<AbstractDeclaratorInfo>(abstrDeclInfoAny);
        auto* tailType = dynamic_cast<INodeType*>(abstrDeclInfo.TailType.GetSubType());
        if (ctx->pointer()) {
            tailType->SetSubType(ptrInfo.HeadType);
            abstrDeclInfo.TailType = ptrInfo.TailType;
        }
    } else {
        abstrDeclInfo.HeadType = ptrInfo.HeadType;
        abstrDeclInfo.TailType = ptrInfo.TailType;
    }

    return abstrDeclInfo;
}

// Returns AbstractDeclaratorInfo
std::any BuildAstVisitor::visitDirectAbstractDeclarator(CParser::DirectAbstractDeclaratorContext* ctx) {
    if (ctx->nested) {
        std::any declInfoAny = visitAbstractDeclarator(ctx->nested);
        return std::any_cast<AbstractDeclaratorInfo>(declInfoAny);
    }

    if (ctx->arrdecl || ctx->leafarr) {
        AbstractDeclaratorInfo declInfo;
        if (ctx->arrdecl) {
            std::any declInfoAny = visitDirectAbstractDeclarator(ctx->arrdecl);
            declInfo = std::any_cast<AbstractDeclaratorInfo>(declInfoAny);
        }

        // NB: It is calculated in Semantic AST Visitor
        ConstExpression* constExpr = nullptr;
        if (ctx->sizeexpr) {
            std::any constExprAny = visitConstantExpression(ctx->sizeexpr);
            constExpr = std::any_cast<ConstExpression*>(constExprAny);
        }

        ArrayType* arrType = m_Program.CreateType<ArrayType>(nullptr, constExpr);
        QualType arrQualType{arrType};

        if (ctx->leafarr) {  // []
            declInfo.HeadType = arrQualType;
            declInfo.TailType = declInfo.HeadType;
            return declInfo;
        }

        auto* tailNodeType = dynamic_cast<INodeType*>(declInfo.TailType.GetSubType());
        if (tailNodeType->IsFunctionType()) {
            printSemanticError("function cannot return array type",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }

        tailNodeType->SetSubType(arrQualType);
        declInfo.TailType = arrQualType;
        return declInfo;
    }

    if (ctx->fundecl || ctx->leaffun) {
        AbstractDeclaratorInfo declInfo;
        if (ctx->fundecl) {
            std::any declInfoAny = visitDirectAbstractDeclarator(ctx->fundecl);
            declInfo = std::any_cast<AbstractDeclaratorInfo>(declInfoAny);
        }

        auto* funType = m_Program.CreateType<FunctionType>(nullptr);
        QualType funQualType{funType};

        ParamTypesInfo paramTypesInfo;
        if (ctx->paramlist) {
            std::any paramTypesAny = visitParameterTypeList(ctx->paramlist);
            paramTypesInfo = std::any_cast<ParamTypesInfo>(paramTypesAny);
        }

        std::vector<QualType> paramTypes;
        for (ParameterDeclaration* paramDecl : paramTypesInfo.ParamDecls) {
            paramTypes.push_back(paramDecl->GetType());
        }
        funType->SetParamTypes(paramTypes);

        if (paramTypesInfo.HasVariadic) {
            funType->SetVariadic();
        }

        if (ctx->leaffun) {  // ()
            declInfo.HeadType = funQualType;
            declInfo.TailType = declInfo.HeadType;
            return declInfo;
        }

        auto* tailNodeType = dynamic_cast<INodeType*>(declInfo.TailType.GetSubType());
        if (tailNodeType->IsFunctionType()) {
            printSemanticError("function cannot return function type",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (tailNodeType->IsArrayType()) {
            printSemanticError("array cannot have elements of function type",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }

        tailNodeType->SetSubType(funQualType);
        declInfo.TailType = funQualType;
        return declInfo;
    }

    return nullptr;
}

// Returns Expression*
std::any BuildAstVisitor::visitInitializer(CParser::InitializerContext* ctx) {
    if (ctx->init) {
        std::any initAny = visitAssignmentExpression(ctx->init);
        return std::any_cast<Expression*>(initAny);
    }

    if (ctx->initlist) {
        std::any initlistAny = visitInitializerList(ctx->initlist);
        auto* initlist = std::any_cast<InitializerList*>(initlistAny);
        return static_cast<Expression*>(initlist);
    }

    return nullptr;
}

// Returns InitializerList*
std::any BuildAstVisitor::visitInitializerList(CParser::InitializerListContext* ctx) {
    std::any firstInitAny = visitInitializer(ctx->init);
    auto* firstInit = std::any_cast<Expression*>(firstInitAny);
    std::vector<Expression*> inits = {firstInit};

    inits.reserve(ctx->initTail.size() + 1);
    for (CParser::InitializerContext* initCtx : ctx->initTail) {
        std::any initAny = visitInitializer(initCtx);
        auto init = std::any_cast<Expression*>(initAny);
        inits.push_back(init);
    }

    auto* initList = m_Program.CreateAstNode<InitializerList>(inits);
    initList->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    return initList;
}

// Returns Statement*
std::any BuildAstVisitor::visitLabeledStatement(CParser::LabeledStatementContext* ctx) {
    if (ctx->Identifier()) {
        auto* labelDecl = m_Program.CreateAstNode<LabelDeclaration>();
        labelDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx->Identifier()));
        std::string name = ctx->Identifier()->getText();
        labelDecl->SetName(name);

        Statement* statement = nullptr;
        if (ctx->statement()) {
            std::any statementAny = ctx->statement();
            statement = std::any_cast<Statement*>(statementAny);
        }
        labelDecl->SetStatement(statement);
        
        auto labelStmt = m_Program.CreateAstNode<LabelStatement>(labelDecl, statement);
        labelStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(labelStmt);
    }

    if (ctx->Case()) {
        std::any constExprAny = visitConstantExpression(ctx->constantExpression());
        auto* constExpr = std::any_cast<ConstExpression*>(constExprAny);

        std::any bodyAny = visitStatement(ctx->statement());
        auto* body = std::any_cast<Statement*>(bodyAny);

        auto* caseStmt = m_Program.CreateAstNode<CaseStatement>(constExpr, body);
        caseStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(caseStmt);
    }
    
    if (ctx->Default()) {
        std::any bodyAny = visitStatement(ctx->statement());
        auto* body = std::any_cast<Statement*>(bodyAny);

        auto* defaultStmt = m_Program.CreateAstNode<DefaultStatement>(body);
        defaultStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(defaultStmt);
    }

    return nullptr;
}

// Returns Statement*
std::any BuildAstVisitor::visitCompoundStatement(CParser::CompoundStatementContext* ctx) {
    if (!ctx->blockItemList()) {
        auto* compoundStmt = m_Program.CreateAstNode<CompoundStatement>();
        compoundStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(compoundStmt);
    }

    std::any compoundStmtAny = visitBlockItemList(ctx->blockItemList());
    auto* compoundStmt = std::any_cast<Statement*>(compoundStmtAny);
    return static_cast<Statement*>(compoundStmt);
}

// Returns Statement*
std::any BuildAstVisitor::visitBlockItemList(CParser::BlockItemListContext* ctx) {
    // TODO: block item list context --> compound context
    auto* compoundStmt = m_Program.CreateAstNode<CompoundStatement>();
    compoundStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
    for (CParser::BlockItemContext* item : ctx->items) {
        std::any statementAny = visitBlockItem(item);
        compoundStmt->AddStatement(std::any_cast<Statement*>(statementAny));
    }
    return static_cast<Statement*>(compoundStmt);
}

// Returns Statement*
std::any BuildAstVisitor::visitBlockItem(CParser::BlockItemContext* ctx) {
    if (ctx->statement()) {
        return visitStatement(ctx->statement());
    }

    if (ctx->declaration()) {
        std::any declarationInfoAny = visitDeclaration(ctx->declaration());
        auto declarationInfo = std::any_cast<DeclarationInfo>(declarationInfoAny);
        std::vector<Declaration*> decls;
        if (declarationInfo.TagPreDecl) {
            decls.push_back(declarationInfo.TagPreDecl);
        }
        decls.push_back(declarationInfo.Decl);
        auto* declStatement = m_Program.CreateAstNode<DeclStatement>(decls);
        declStatement->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(declStatement);
    }

    return nullptr;
}

// Returns Statement*
std::any BuildAstVisitor::visitExpressionStatement(CParser::ExpressionStatementContext* ctx) {
    if (ctx->expression()) {
        std::any expressionAny = visitExpression(ctx->expression());
        auto* expression = std::any_cast<Expression*>(expressionAny);
        return static_cast<Statement*>(expression);
    }
    return nullptr;
}

// Returns Statement*
std::any BuildAstVisitor::visitSelectionStatement(CParser::SelectionStatementContext* ctx) {
    if (ctx->If()) {
        std::any condExpressionAny = visitExpression(ctx->cond);
        auto* condExpression = std::any_cast<Expression*>(condExpressionAny);

        std::any thenStatementAny = visitStatement(ctx->thenstmt);
        auto* thenStatement = std::any_cast<Statement*>(thenStatementAny);

        Statement* elseStatement = nullptr;
        if (ctx->elsestmt) {
            std::any elseStatementAny = visitStatement(ctx->elsestmt);
            elseStatement = std::any_cast<Statement*>(elseStatementAny);
        }

        auto* ifStmt = m_Program.CreateAstNode<IfStatement>(
            condExpression, thenStatement, elseStatement
        );
        ifStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(ifStmt);
    }

    if (ctx->Switch()) {
        std::any expressionAny = visitExpression(ctx->expression());
        auto* expression = std::any_cast<Expression*>(expressionAny);

        std::any statementAny = visitStatement(ctx->switchstmt);
        auto* statement = std::any_cast<Statement*>(statementAny);

        auto* switchStmt = m_Program.CreateAstNode<SwitchStatement>(
            expression, statement
        );
        switchStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(switchStmt);  
    }

    return nullptr;
}

// Returns Statement*
std::any BuildAstVisitor::visitIterationStatement(CParser::IterationStatementContext* ctx) {
    if (ctx->loop->getType() == CParser::While) {
        std::any conditionAny = visitExpression(ctx->cond);
        auto* condition = std::any_cast<Expression*>(conditionAny);

        std::any bodyAny = visitStatement(ctx->stmt);
        auto* body = std::any_cast<Statement*>(bodyAny);

        auto* whileStmt = m_Program.CreateAstNode<WhileStatement>(condition, body);
        whileStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(whileStmt);
    }
    
    if (ctx->loop->getType() == CParser::Do) {
        std::any conditionAny = visitExpression(ctx->cond);
        auto* condition = std::any_cast<Expression*>(conditionAny);

        std::any bodyAny = visitStatement(ctx->stmt);
        auto* body = std::any_cast<Statement*>(bodyAny);

        auto* doStmt = m_Program.CreateAstNode<DoStatement>(condition, body);
        doStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(doStmt);
    }

    if (ctx->loop->getType() == CParser::For) {
        std::any conditionAny = visitForCondition(ctx->forCondition());
        auto condition = std::any_cast<
                std::tuple<Statement*, Expression*, Expression*>>(conditionAny);
        auto [init, cond, step] = condition;

        std::any bodyAny = visitStatement(ctx->stmt);
        auto* body = std::any_cast<Statement*>(bodyAny);

        auto forStmt = m_Program.CreateAstNode<ForStatement>(init, cond, step, body);
        forStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx->statement()));
        return static_cast<Statement*>(forStmt);
    }

    return nullptr;
}

// Returns std::tuple<Statement*, Expression*, Expression*>
std::any BuildAstVisitor::visitForCondition(CParser::ForConditionContext* ctx) {
    Statement* initStatement = nullptr;
    if (ctx->decl) {
        std::any declarationInfoAny = visitForDeclaration(ctx->decl);
        auto declarationInfo = std::any_cast<DeclarationInfo>(declarationInfoAny);
        std::vector<Declaration*> decls;
        if (declarationInfo.TagPreDecl) {
            decls.push_back(declarationInfo.TagPreDecl);
        }
        decls.push_back(declarationInfo.Decl);
        initStatement = m_Program.CreateAstNode<DeclStatement>(decls);
        initStatement->SetLocation(m_LocationBuilder.CreateASTLocation(ctx->decl));
    } else if (ctx->expr) {
        std::any exprAny = visitExpression(ctx->expr);
        initStatement = std::any_cast<Expression*>(exprAny);
    }

    Expression* condExpression = nullptr;
    if (ctx->cond) {
        std::any condAny = visitForExpression(ctx->cond);
        condExpression = std::any_cast<Expression*>(condAny);
    }

    Expression* stepExpression = nullptr;
    if (ctx->step) {
        std::any stepAny = visitForExpression(ctx->step);
        stepExpression = std::any_cast<Expression*>(stepAny);
    }

    return std::tuple{initStatement, condExpression, stepExpression};
}

// Returns DeclarationInfo
std::any BuildAstVisitor::visitForDeclaration(CParser::ForDeclarationContext* ctx) {
    std::any declSpecsAny = visitDeclarationSpecifiers(ctx->declspecs);
    auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

    std::vector<InitDeclaratorInfo> initDeclList;
    if (ctx->initdecl) {
        std::any initDeclListAny = visitInitDeclaratorList(ctx->initdecl);
        initDeclList = std::any_cast<std::vector<InitDeclaratorInfo>>(initDeclListAny);
    }

    DeclarationInfo declarationInfo = createDeclaration(declSpecs, initDeclList, ctx);
    if (!declarationInfo.TagPreDecl && !declarationInfo.Decl) {
        printSemanticWarning("declaration does not declare anything",
                             m_LocationBuilder.CreateASTLocation(ctx));
    }

    return declarationInfo;
}

// Returns Expression*
std::any BuildAstVisitor::visitForExpression(CParser::ForExpressionContext* ctx) {
    auto* exprList = m_Program.CreateAstNode<ExpressionList>();
    exprList->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));

    std::any firstExpressionAny = visitAssignmentExpression(ctx->expr);
    auto* firstExpression = std::any_cast<Expression*>(firstExpressionAny);
    exprList->AddExpression(firstExpression);

    for (CParser::AssignmentExpressionContext* exprCtx : ctx->exprTail) {
        std::any expressionAny = visitAssignmentExpression(exprCtx);
        auto* expression = std::any_cast<Expression*>(expressionAny);
        exprList->AddExpression(expression);
    }

    return static_cast<Expression*>(exprList);
}

// Returns Statement*
std::any BuildAstVisitor::visitJumpStatement(CParser::JumpStatementContext* ctx) {
    if (ctx->Goto()) {
        auto* labelDecl = m_Program.CreateAstNode<LabelDeclaration>();
        std::string name = ctx->Identifier()->getText();
        labelDecl->SetName(name);
        auto* gotoStmt = m_Program.CreateAstNode<GotoStatement>(labelDecl);
        gotoStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(gotoStmt);
    }

    if (ctx->Continue()) {
        auto* continueStmt = m_Program.CreateAstNode<LoopJumpStatement>(LoopJumpStatement::Type::kContinue);
        continueStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(continueStmt);
    }

    if (ctx->Break()) {
        auto* breakStmt = m_Program.CreateAstNode<LoopJumpStatement>(LoopJumpStatement::Type::kBreak);
        breakStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(breakStmt);
    }

    if (ctx->Return()) {
        Expression* expression = nullptr;
        if (ctx->expression()) {
            std::any expressionAny = visitExpression(ctx->expression());
            expression = std::any_cast<Expression*>(expressionAny);
        }
        auto* returnStmt = m_Program.CreateAstNode<ReturnStatement>(expression);
        returnStmt->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));
        return static_cast<Statement*>(returnStmt);
    }

    return nullptr;
}

// Returns void
std::any BuildAstVisitor::visitTranslationUnit(CParser::TranslationUnitContext* ctx) {
    auto* translationUnit = m_Program.CreateAstNode<TranslationUnit>();
    translationUnit->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));

    for (CParser::ExternalDeclarationContext* declCtx : ctx->decls) {
        std::any declarationAny = visitExternalDeclaration(declCtx);
        DeclarationInfo declarationInfo = std::any_cast<DeclarationInfo>(declarationAny);
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

// Returns DeclarationInfo
std::any BuildAstVisitor::visitFunctionDefinition(CParser::FunctionDefinitionContext* ctx) {
    std::any declSpecsAny = visitDeclarationSpecifiers(ctx->declspecs);
    auto declSpecs = std::any_cast<DeclSpecifiers>(declSpecsAny);

    std::vector<DeclaratorInfo> declList;
    if (ctx->decl) {
        std::any declInfoAny = visitDeclarator(ctx->decl);
        DeclaratorInfo declInfo = std::any_cast<DeclaratorInfo>(declInfoAny);
        declList.push_back(declInfo);
    }

    DeclarationInfo declarationInfo = createDeclaration(declSpecs, declList, ctx);

    Declaration* decl = declarationInfo.Decl;
    assert(decl);

    auto* functionDecl = dynamic_cast<FunctionDeclaration*>(decl);
    assert(functionDecl);

    for (ParameterDeclaration* paramDecl : functionDecl->GetParams()) {
        if (!paramDecl->HasName()) {
            printSemanticError("omitting the parameter name in a function definition",
                               paramDecl->GetLocation());
        }
    }

    std::any bodyAny = visitCompoundStatement(ctx->body);
    auto* body = std::any_cast<Statement*>(bodyAny);
    functionDecl->SetBody(body);

    return declarationInfo;
}

BuildAstVisitor::DeclarationInfo BuildAstVisitor::createDeclaration(
            const DeclSpecifiers& declSpecs,
            const std::vector<DeclaratorInfo>& declList,
            antlr4::ParserRuleContext* ctx,
            bool isParam) {
    std::vector<InitDeclaratorInfo> initDeclList;
    initDeclList.reserve(declList.size());

    for (const DeclaratorInfo& decl : declList) {
        initDeclList.push_back(InitDeclaratorInfo{
            .DeclInfo=decl,
            .Init=nullptr,
        });
    }

    return createDeclaration(declSpecs, initDeclList, ctx, isParam);
}

BuildAstVisitor::DeclarationInfo BuildAstVisitor::createDeclaration(
            const DeclSpecifiers& declSpecs,
            const std::vector<InitDeclaratorInfo>& initDeclList,
            antlr4::ParserRuleContext* ctx,
            bool isParam) {
    DeclarationInfo resInfo;
    
    QualType declSpecsQualType = declSpecs.Type;

    Type* declSpecsType = declSpecsQualType.GetSubType();
    if (RecordType* recordType = dynamic_cast<RecordType*>(declSpecsType)) {
        RecordDeclaration* recordDecl = recordType->GetDeclaration();
        resInfo.TagPreDecl = recordDecl;
    }
    if (EnumType* enumType = dynamic_cast<EnumType*>(declSpecsType)) {
        EnumDeclaration* enumDecl = enumType->GetDeclaration();
        resInfo.TagPreDecl = enumDecl;
    }

    if (initDeclList.empty()) {  // Only tag declaration
        return resInfo;
    }

    for (const InitDeclaratorInfo& initDecl : initDeclList) {
        DeclaratorInfo declInfo = initDecl.DeclInfo;

        if (declInfo.TailType.HasSubType()) {
            auto* tailType = dynamic_cast<INodeType*>(declInfo.TailType.GetSubType());
            tailType->SetSubType(declSpecsQualType);
        } else {  // declspecs Ident
            declInfo.HeadType = declSpecsQualType;
            declInfo.TailType = declInfo.HeadType;
        }

        if (declSpecs.Storage == StorageClassInfo::kTypedef) {
            auto* typedefDecl = m_Program.CreateAstNode<TypedefDeclaration>();
            typedefDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));

            if (initDecl.Init) {
                printSemanticError("illegal initializer (only variables can be initialized)",
                                   initDecl.Init->GetLocation());
            }

            typedefDecl->SetName(declInfo.Identifier);
            typedefDecl->SetType(declInfo.HeadType);

            resInfo.Decl = typedefDecl;
        } else if (declInfo.FunctionDecl && !isParam) {
            if (initDecl.Init) {
                printSemanticError("illegal initializer (only variables can be initialized)",
                                   initDecl.Init->GetLocation());
            }
            FunctionDeclaration* funDecl = declInfo.FunctionDecl;
            // funDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));

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
            case StorageClassInfo::kRegister:
                printSemanticError("illegal storage class on function",
                                   m_LocationBuilder.CreateASTLocation(ctx));
                break;
            }

            resInfo.Decl = funDecl;
        } else {  // Variable/Parameter declaration
            if (declSpecs.FunSpec != FunctionSpecifier::kNone) {
                printSemanticError("'inline' can only appear on functions",
                                   m_LocationBuilder.CreateASTLocation(ctx));
            }

            if (isParam) {
                auto* paramDecl = m_Program.CreateAstNode<ParameterDeclaration>();
                paramDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));

                paramDecl->SetName(declInfo.Identifier);

                // void fun(int (int)) -> void fun(int (*)(int));
                if (dynamic_cast<FunctionType*>(declInfo.HeadType.GetSubType())) {
                    auto* pointerType = m_Program.CreateType<PointerType>(declInfo.HeadType);
                    declInfo.HeadType = QualType{pointerType};
                }

                // void fun(int arr[10]) -> void fun(int *)
                if (auto* arrayType = dynamic_cast<ArrayType*>(declInfo.HeadType.GetSubType())) {
                    auto* pointerType = m_Program.CreateType<PointerType>(arrayType->GetSubType());
                    declInfo.HeadType = QualType{pointerType};
                }

                paramDecl->SetType(declInfo.HeadType);
                if (initDecl.Init) {
                    printSemanticError("C does not support default arguments",
                                       initDecl.Init->GetLocation());
                }

                switch (declSpecs.Storage) {
                case StorageClassInfo::kExtern:
                    paramDecl->SetStorageClass(StorageClass::kExtern);
                    break;
                case StorageClassInfo::kStatic:
                    paramDecl->SetStorageClass(StorageClass::kStatic);
                    break;
                case StorageClassInfo::kAuto:
                    printSemanticError("invalid storage class specifier in function declarator",
                                       m_LocationBuilder.CreateASTLocation(ctx));
                    break;
                case StorageClassInfo::kRegister:
                    paramDecl->SetStorageClass(StorageClass::kRegister);
                    break;
                }

                resInfo.Decl = paramDecl;
            } else {
                auto* varDecl = m_Program.CreateAstNode<VariableDeclaration>();
                varDecl->SetLocation(m_LocationBuilder.CreateASTLocation(ctx));

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
    }

    return resInfo;
}

Type* BuildAstVisitor::createBuiltinTypeFromInfo(const BuiltinTypeInfo& info,
                                                 antlr4::ParserRuleContext* ctx) {
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
            printSemanticError("cannot combine 'void' with size specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (info.Sign != BuiltinTypeInfo::SignModifier::kNone) {
            printSemanticError("'void' cannot be signed or unsigned",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kVoid);
    }

    if (info.Type == BuiltinTypeInfo::BasicType::kChar) {
        if (info.Size != BuiltinTypeInfo::SizeModifier::kNone) {
            printSemanticError("cannot combine 'char' with size specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (info.Sign == BuiltinTypeInfo::SignModifier::kUnsigned) {
            return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kUChar);
        }
        return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kChar);
    }

    if (info.Type == BuiltinTypeInfo::BasicType::kFloat) {
        if (info.Size != BuiltinTypeInfo::SizeModifier::kNone) {
            printSemanticError("cannot combine 'float' with size specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (info.Sign != BuiltinTypeInfo::SignModifier::kNone) {
            printSemanticError("'float' cannot be signed or unsigned",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        return m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kFloat);
    }

    if (info.Type == BuiltinTypeInfo::BasicType::kDouble) {
        if (info.Sign != BuiltinTypeInfo::SignModifier::kNone) {
            printSemanticError("'double' cannot be signed or unsigned",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (info.Size == BuiltinTypeInfo::SizeModifier::kShort) {
            printSemanticError("cannot combine 'double' and 'short' specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
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

    // TODO: Make target independent
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

void BuildAstVisitor::updateDeclSpecs(DeclSpecifiers& declSpecs,
                                      const std::any& declSpecAny,
                                      antlr4::ParserRuleContext* ctx) {
    if (auto* storageClassOpt = std::any_cast<StorageClassInfo>(&declSpecAny)) {
        StorageClassInfo storageClass = *storageClassOpt;

        if (declSpecs.Storage == storageClass) {
            printSemanticWarning(std::format("duplicate '{}' declaration specifier",
                                             getStorageClassString(storageClass)),
                                 m_LocationBuilder.CreateASTLocation(ctx));
        } else if (declSpecs.Storage != StorageClassInfo::kNone) {
            printSemanticError(std::format("cannot combine '{}' and '{}' specifiers",
                                           getStorageClassString(storageClass),
                                           getStorageClassString(declSpecs.Storage)),
                               m_LocationBuilder.CreateASTLocation(ctx));
        }

        declSpecs.Storage = storageClass;
        return;
    }

    BuiltinTypeInfo& builtinTypeInfo = declSpecs.BuiltinType;
    auto* namedTypeSpecOpt = std::any_cast<Type*>(&declSpecAny);
    if (namedTypeSpecOpt) {
        if (builtinTypeInfo.Sign != BuiltinTypeInfo::SignModifier::kNone) {
            printSemanticError("cannot combine 'struct', 'union' and 'enum' with sign specifier",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (builtinTypeInfo.Size != BuiltinTypeInfo::SizeModifier::kNone) {
            printSemanticError("cannot combine 'struct', 'union' and 'enum' with size specifier",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        if (builtinTypeInfo.Type != BuiltinTypeInfo::BasicType::kNone) {
            printSemanticError("cannot combine 'struct', 'union' and 'enum' with basic type specifier",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }

        QualType& qualType = declSpecs.Type;
        if (qualType.HasSubType()) {
            printSemanticError("cannot combine 'struct', 'union' and 'enum' specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }

        Type* namedType = *namedTypeSpecOpt;
        qualType.SetSubType(namedType);
        return;
    }

    if (auto* qualifierOpt = std::any_cast<Qualifier>(&declSpecAny)) {
        Qualifier qualifier = *qualifierOpt;
        QualType& declSpecsQualType = declSpecs.Type;

        switch (qualifier) {
        case Qualifier::kConst:
            if (declSpecsQualType.IsConst()) {
                printSemanticWarning("duplicate 'const' declaration specifier",
                                     m_LocationBuilder.CreateASTLocation(ctx));
            }
            declSpecsQualType.AddConst();
            break;
        case Qualifier::kRestrict:
            if (declSpecsQualType.IsRestrict()) {
                printSemanticWarning("duplicate 'restrict' declaration specifier",
                                     m_LocationBuilder.CreateASTLocation(ctx));
            }
            declSpecsQualType.AddRestrict();
            break;
        case Qualifier::kVolatile:
            if (declSpecsQualType.IsVolatile()) {
                printSemanticWarning("duplicate 'volatile' declaration specifier",
                                     m_LocationBuilder.CreateASTLocation(ctx));
            }
            declSpecsQualType.AddVolatile();
            break;
        default:
            assert(false);
            break;
        };
        return;
    }

    if (auto* functionSpecOpt = std::any_cast<FunctionSpecifier>(&declSpecAny)) {
        FunctionSpecifier functionSpec = *functionSpecOpt;

        if (declSpecs.FunSpec == functionSpec) {
            printSemanticWarning(std::format("duplicate '{}' declaration specifier",
                                             getFunctionSpecifierString(functionSpec)),
                                 m_LocationBuilder.CreateASTLocation(ctx));
        } else if (declSpecs.FunSpec != FunctionSpecifier::kNone) {
            printSemanticError("cannot combine function specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }

        declSpecs.FunSpec = functionSpec;
        return;
    }

    auto* builtinTypeSpecOpt = std::any_cast<BuiltinTypeSpecifier>(&declSpecAny);
    assert(builtinTypeSpecOpt);

    BuiltinTypeSpecifier builtinTypeSpec = *builtinTypeSpecOpt;
    if (builtinTypeSpec == BuiltinTypeSpecifier::kSigned) {
        if (builtinTypeInfo.Sign == BuiltinTypeInfo::SignModifier::kSigned) {
            printSemanticWarning("duplicate 'signed' declaration specifier",
                                 m_LocationBuilder.CreateASTLocation(ctx));
        } else if (builtinTypeInfo.Sign != BuiltinTypeInfo::SignModifier::kNone) {
            printSemanticError("cannot combine sign specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        builtinTypeInfo.Sign = BuiltinTypeInfo::SignModifier::kSigned;
    } else if (builtinTypeSpec == BuiltinTypeSpecifier::kUnsigned) {
        if (builtinTypeInfo.Sign == BuiltinTypeInfo::SignModifier::kUnsigned) {
            printSemanticWarning("duplicate 'unsigned' declaration specifier",
                                 m_LocationBuilder.CreateASTLocation(ctx));
        } else if (builtinTypeInfo.Sign != BuiltinTypeInfo::SignModifier::kNone) {
            printSemanticError("cannot combine sign specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        builtinTypeInfo.Sign = BuiltinTypeInfo::SignModifier::kUnsigned;
    } else if (builtinTypeSpec == BuiltinTypeSpecifier::kShort) {
        if (builtinTypeInfo.Size == BuiltinTypeInfo::SizeModifier::kShort) {
            printSemanticWarning("duplicate 'short' declaration specifier",
                                 m_LocationBuilder.CreateASTLocation(ctx));
        } else if (builtinTypeInfo.Size != BuiltinTypeInfo::SizeModifier::kNone) {
            printSemanticError("cannot combine size specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
        }
        builtinTypeInfo.Size = BuiltinTypeInfo::SizeModifier::kShort;
    } else if (builtinTypeSpec == BuiltinTypeSpecifier::kLong) {
        if (builtinTypeInfo.Size == BuiltinTypeInfo::SizeModifier::kLong) {
            builtinTypeInfo.Size = BuiltinTypeInfo::SizeModifier::kLongLong; 
        } else if (builtinTypeInfo.Size != BuiltinTypeInfo::SizeModifier::kNone) {
            printSemanticError("cannot combine size specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
        } else {
            builtinTypeInfo.Size = BuiltinTypeInfo::SizeModifier::kLong; 
        }
    } else {  // basic type
        if (builtinTypeInfo.Type != BuiltinTypeInfo::BasicType::kNone) {
            printSemanticError("cannot combine basic type specifiers",
                               m_LocationBuilder.CreateASTLocation(ctx));
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
            assert(false);
            break;
        }
    }
}

std::string BuildAstVisitor::getFunctionSpecifierString(FunctionSpecifier info) {
    switch (info) {
    case FunctionSpecifier::kInline:
        return "inline";
    }
    return "";
}

std::string BuildAstVisitor::getStorageClassString(StorageClassInfo info) {
    switch (info) {
    case StorageClassInfo::kTypedef:
        return "typedef";
    case StorageClassInfo::kExtern:
        return "extern";
    case StorageClassInfo::kStatic:
        return "static";
    case StorageClassInfo::kAuto:
        return "auto";
    case StorageClassInfo::kRegister:
        return "register";
    }
    return "";
}

}  // namespace anclgrammar
