#include <Ancl/Visitor/SemanticAstVisitor.hpp>

#include <ranges>

#include <Ancl/Visitor/IntConstExprAstVisitor.hpp>


namespace ast {

bool areEqualTypes(const Type* leftType, const Type* rightType);

bool areEqualQualTypes(const QualType& leftQualType, const QualType& rightQualType) {
    if (leftQualType.IsConst() != rightQualType.IsConst()) {
        return false;
    }
    if (leftQualType.IsVolatile() != rightQualType.IsVolatile()) {
        return false;
    }
    if (leftQualType.IsRestrict() != rightQualType.IsRestrict()) {
        return false;
    }
    return areEqualTypes(leftQualType.GetSubType(), rightQualType.GetSubType());
}

bool areEqualTypes(const Type* leftType, const Type* rightType) {
    const auto* builtinLeftType = dynamic_cast<const BuiltinType*>(leftType);
    const auto* builtinRightType = dynamic_cast<const BuiltinType*>(rightType);
    if (builtinLeftType && builtinRightType) {
        return builtinLeftType->GetKind() == builtinRightType->GetKind();
    }

    const auto* ptrLeftType = dynamic_cast<const PointerType*>(leftType);
    const auto* ptrRightType = dynamic_cast<const PointerType*>(rightType);
    if (ptrLeftType && ptrRightType) {
        return areEqualQualTypes(ptrLeftType->GetSubType(), ptrRightType->GetSubType());
    }

    const auto* arrayLeftType = dynamic_cast<const ArrayType*>(leftType);
    const auto* arrayRightType = dynamic_cast<const ArrayType*>(rightType);
    if (arrayLeftType && arrayRightType) {
        if (arrayLeftType->GetSize() != arrayRightType->GetSize()) {
            return false;
        }
        return areEqualQualTypes(arrayLeftType->GetSubType(), arrayRightType->GetSubType());
    }

    const auto* funLeftType = dynamic_cast<const FunctionType*>(leftType);
    const auto* funRightType = dynamic_cast<const FunctionType*>(rightType);
    if (funLeftType && funRightType) {
        if (funLeftType->IsVariadic() != funRightType->IsVariadic()) {
            return false;
        }
        
        std::vector<QualType> leftParamTypes = funLeftType->GetParamTypes();
        std::vector<QualType> rightParamTypes = funRightType->GetParamTypes();
        if (leftParamTypes.size() != rightParamTypes.size()) {
            return false;
        }

        for (size_t i = 0; i < leftParamTypes.size(); ++i) {
            if (!areEqualQualTypes(leftParamTypes[i], rightParamTypes[i])) {
                return false;
            }
        }

        return areEqualQualTypes(funLeftType->GetSubType(), funRightType->GetSubType());
    }

    const auto* tagLeftType = dynamic_cast<const TagType*>(leftType);
    const auto* tagRightType = dynamic_cast<const TagType*>(rightType);
    if (tagLeftType && tagRightType) {
        if (tagLeftType->IsRecord() && tagRightType->IsRecord()) {
            const auto* recordLeftType = static_cast<const RecordType*>(tagLeftType);
            const auto* recordRightType = static_cast<const RecordType*>(tagRightType);
            return recordLeftType->GetDeclaration() == recordRightType->GetDeclaration();
        }

        if (tagLeftType->IsEnum() && tagRightType->IsEnum()) {
            const auto* enumLeftType = static_cast<const EnumType*>(tagLeftType);
            const auto* enumRightType = static_cast<const EnumType*>(tagRightType);
            return enumLeftType->GetDeclaration() == enumRightType->GetDeclaration();
        }

        return false;
    }

    const auto* typedefLeftType = dynamic_cast<const TypedefType*>(leftType);
    const auto* typedefRightType = dynamic_cast<const TypedefType*>(rightType);
    if (typedefLeftType && typedefRightType) {
        return typedefLeftType->GetDeclaration() == typedefRightType->GetDeclaration();
    }

    return false;
}


/*
=================================================================
                            Declaration
=================================================================
*/

void SemanticAstVisitor::Visit(EnumConstDeclaration& enumConstDecl) {
    std::string enumConstName = enumConstDecl.GetName();
    if (m_CurrentScope->HasSymbol(Scope::NamespaceType::Ident, enumConstName)) {
        printSemanticError(std::format("redefinition of enumerator '{}'", enumConstName),
                            enumConstDecl.GetLocation());
    } else {
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, enumConstName, &enumConstDecl);
    }

    ConstExpression* initExpr = enumConstDecl.GetInit();
    initExpr->Accept(*this);
}

void SemanticAstVisitor::Visit(EnumDeclaration& enumDecl) {
    std::string enumName = enumDecl.GetName();

    if (m_CurrentScope->HasSymbol(Scope::NamespaceType::Tag, enumName)) {
        Declaration* decl = m_CurrentScope->GetSymbol(Scope::NamespaceType::Tag, enumName);
        auto* scopeEnumDecl = dynamic_cast<EnumDeclaration*>(decl);
        if (!scopeEnumDecl) {
            printSemanticError(std::format("use of '{}' with tag type that does not "
                                            "match previous declaration", enumName),
                                enumDecl.GetLocation());
        } else {
            handleTagDeclaration(&enumDecl, enumName, scopeEnumDecl);
        }
    } else {
        auto decl = m_CurrentScope->FindSymbol(Scope::NamespaceType::Tag, enumName);
        if (decl) {
            m_CurrentScope->AddSymbol(Scope::NamespaceType::Tag, enumName, *decl);
        } else {
            m_CurrentScope->AddSymbol(Scope::NamespaceType::Tag, enumName, &enumDecl);
        }
    }

    for (EnumConstDeclaration* enumConstDecl : enumDecl.GetEnumerators()) {
        enumConstDecl->Accept(*this);
    }
}

void SemanticAstVisitor::Visit(FieldDeclaration& fieldDecl) {
    QualType fieldDeclQualType = AcceptQualType(fieldDecl.GetType());
    fieldDecl.SetType(fieldDeclQualType);
    Type* fieldDeclType = fieldDeclQualType.GetSubType();

    std::string fieldName = fieldDecl.GetName();
    if (isIncompleteType(fieldDeclQualType)) {
        printSemanticError(std::format("incomplete member '{}' type", fieldName),
                            fieldDecl.GetLocation());  
    }

    if (m_CurrentScope->HasSymbol(Scope::NamespaceType::Ident, fieldName)) {
        printSemanticError(std::format("duplicate member '{}'", fieldName),
                            fieldDecl.GetLocation());
    } else {
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, fieldName, &fieldDecl);
    }
}

void SemanticAstVisitor::Visit(FunctionDeclaration& funcDecl) {
    QualType funcDeclQualType = AcceptQualType(funcDecl.GetType());
    funcDecl.SetType(funcDeclQualType);
    Type* funcDeclType = funcDeclQualType.GetSubType();

    std::string funcName = funcDecl.GetName();

    Scope* globalScope = m_SymbolTable.GetGlobalScope();
    if (globalScope->HasSymbol(Scope::NamespaceType::Ident, funcName)) {
        Declaration* decl = globalScope->GetSymbol(Scope::NamespaceType::Ident, funcName);
        auto* scopeFuncDecl = dynamic_cast<FunctionDeclaration*>(decl);
        if (!scopeFuncDecl) {
            printSemanticError(std::format("redefinition of '{}' as different kind of symbol",
                                            funcName),
                                funcDecl.GetLocation());
        } else {
            if (funcDecl.HasBody()) {  // Function definition
                if (scopeFuncDecl->HasBody()) {  
                    printSemanticError(std::format("redefinition of '{}'",funcName),
                                        funcDecl.GetLocation());
                } else {
                    // TODO: Compare types
                    // printSemanticError(std::format("conflicting types for '{}'", funcName),
                    //                    funcDecl.GetLocation());

                    // TODO: If types are equal, handle static/extern
                }
            } else {  // Function declaration
                // TODO: Compare types
                // printSemanticError(std::format("conflicting types for '{}'", funcName),
                //                    funcDecl.GetLocation());

                // TODO: If types are equal, handle static/extern
            }
        }
    } else {
        auto decl = m_CurrentScope->FindSymbol(Scope::NamespaceType::Ident, funcName);
        if (decl) {
            m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, funcName, *decl);
        } else {
            m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, funcName, &funcDecl);
        }
    }

    m_CurrentScope = m_SymbolTable.CreateScope(std::format("{} [function]", funcName),
                                                m_CurrentScope);
    m_FunctionScope = m_CurrentScope;  

    for (ParameterDeclaration* paramDecl : funcDecl.GetParams()) {
        paramDecl->Accept(*this);
    }

    Statement* body = funcDecl.GetBody();
    if (body) {
        // TODO: Place inside classes
        m_UnlabeledGotos.clear();
        m_CurrentLabelDecl = nullptr;
        m_HasReturn = false;
        m_IgnoreCompoundScope = true;
        m_CurrentFunctionDecl = &funcDecl;

        body->Accept(*this);
    
        auto* funcType = static_cast<FunctionType*>(funcDeclType);
        if (!isVoidType(funcType->GetSubType()) && !m_HasReturn) {
            printSemanticError("non-void function does not return a value", funcDecl.GetLocation());
        }

        for (GotoStatement* gotoStmt : std::views::values(m_UnlabeledGotos)) {
            printSemanticError(std::format("use of undeclared label '{}'", gotoStmt->GetLabel()->GetName()), 
                               gotoStmt->GetLocation());
        }
    }

    m_FunctionScope = nullptr;
    m_CurrentScope = m_CurrentScope->GetParentScope(); 
}

void SemanticAstVisitor::Visit(LabelDeclaration& labelDecl) {
    std::string labelName = labelDecl.GetName();
    assert(m_FunctionScope);
    if (m_FunctionScope->HasSymbol(Scope::NamespaceType::Label, labelName)) {
        printSemanticError(std::format("redefinition of label '{}'", labelName),
                            labelDecl.GetLocation());
    } else {
        m_FunctionScope->AddSymbol(Scope::NamespaceType::Label, labelName, &labelDecl);
    }

    if (m_CurrentLabelDecl) {
        labelDecl.SetPreviousLabelDeclaration(m_CurrentLabelDecl);
    }
    m_CurrentLabelDecl = &labelDecl;

    if (m_UnlabeledGotos.contains(labelName)) {
        m_UnlabeledGotos[labelName]->SetLabel(&labelDecl);
        m_UnlabeledGotos.erase(labelName);
    }
}

void SemanticAstVisitor::Visit(ParameterDeclaration& paramDecl) {
    QualType paramDeclQualType = AcceptQualType(paramDecl.GetType());
    paramDecl.SetType(paramDeclQualType);
    Type* paramDeclType = paramDeclQualType.GetSubType();

    std::string paramName = paramDecl.GetName();
    assert(m_FunctionScope);
    if (m_FunctionScope->HasSymbol(Scope::NamespaceType::Ident, paramName)) {
        printSemanticError(std::format("redefinition of parameter '{}'", paramName),
                            paramDecl.GetLocation());
    } else {
        m_FunctionScope->AddSymbol(Scope::NamespaceType::Ident, paramName, &paramDecl);
    }
}

void SemanticAstVisitor::Visit(RecordDeclaration& recordDecl) {
    std::string recordName = recordDecl.GetName();

    if (m_CurrentScope->HasSymbol(Scope::NamespaceType::Tag, recordName)) {
        Declaration* decl = m_CurrentScope->GetSymbol(Scope::NamespaceType::Tag, recordName);
        auto* scopeRecordDecl = dynamic_cast<RecordDeclaration*>(decl);
        if (!scopeRecordDecl || recordDecl.IsStruct() != scopeRecordDecl->IsStruct()) {
            printSemanticError(std::format("use of '{}' with tag type that does not "
                                            "match previous declaration", recordName),
                                recordDecl.GetLocation());
        } else {
            handleTagDeclaration(&recordDecl, recordName, scopeRecordDecl);
        }
    } else {
        auto decl = m_CurrentScope->FindSymbol(Scope::NamespaceType::Tag, recordName);
        if (decl) {
            m_CurrentScope->AddSymbol(Scope::NamespaceType::Tag, recordName, *decl);
        } else {
            m_CurrentScope->AddSymbol(Scope::NamespaceType::Tag, recordName, &recordDecl);
        }
    }

    Scope* recordScope = m_SymbolTable.CreateScope(std::format("{} [record]", recordName),
                                                    m_CurrentScope);
    for (Declaration* decl : recordDecl.GetInternalDecls()) {
        if (auto* fieldDecl = dynamic_cast<FieldDeclaration*>(decl)) {
            m_CurrentScope = recordScope;
            fieldDecl->Accept(*this);
            m_CurrentScope = m_CurrentScope->GetParentScope();
        } else {
            // internal Tag declaration
            decl->Accept(*this);
        }
    }
}

void SemanticAstVisitor::Visit(TranslationUnit& unit) {
    for (Declaration* decl : unit.GetDeclarations()) {
        decl->Accept(*this);
    }
}

void SemanticAstVisitor::Visit(TypedefDeclaration& typedefDecl) {
    QualType typedefQualType = AcceptQualType(typedefDecl.GetType());
    typedefDecl.SetType(typedefQualType);
    Type* typedefType = typedefQualType.GetSubType();

    std::string typedefName = typedefDecl.GetName();
    if (m_CurrentScope->HasSymbol(Scope::NamespaceType::Ident, typedefName)) {
        Declaration* decl = m_CurrentScope->GetSymbol(Scope::NamespaceType::Ident, typedefName);
        auto* scopeTypedefDecl = dynamic_cast<TypedefDeclaration*>(decl);
        if (!scopeTypedefDecl) {
            printSemanticError(std::format("redefinition of '{}' as different kind of symbol",
                                            typedefName),
                                typedefDecl.GetLocation());       
        } else if (!areEqualQualTypes(scopeTypedefDecl->GetType(), typedefDecl.GetType())) {
            printSemanticError(std::format("redefinition of typedef '{}'", typedefName),
                                typedefDecl.GetLocation());
        }
    } else {
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, typedefName, &typedefDecl);
    }
}

void SemanticAstVisitor::Visit(VariableDeclaration& varDecl) {
    QualType varDeclQualType = AcceptQualType(varDecl.GetType());
    varDecl.SetType(varDeclQualType);
    Type* varDeclType = varDeclQualType.GetSubType();

    if (isIncompleteType(varDeclQualType)) {
        printSemanticError("variable has incomplete type",
                           varDecl.GetLocation());    
    }

    std::string varName = varDecl.GetName();
    if (m_CurrentScope->IsGlobalScope()) {  // Global variable
        varDecl.SetGlobal();

        if (varDecl.GetStorageClass() == StorageClass::kRegister) {
            printSemanticError("illegal storage class on file-scoped variable",
                                varDecl.GetLocation());
        }

        if (m_CurrentScope->HasSymbol(Scope::NamespaceType::Ident, varName)) {
            Declaration* decl = m_CurrentScope->GetSymbol(Scope::NamespaceType::Ident, varName);
            auto* scopeVarDecl = dynamic_cast<VariableDeclaration*>(decl);
            if (!scopeVarDecl) {
                printSemanticError(std::format("redefinition of '{}' as different kind of symbol",
                                                varName),
                                    varDecl.GetLocation());
            } else {
                if (varDecl.HasInit()) {  // Global variable definition
                    if (scopeVarDecl->HasInit()) {
                        printSemanticError(std::format("redefinition of '{}'",varName),
                                            varDecl.GetLocation());
                    } else {  // Tentative definition
                        // TODO: Compare types
                        // printSemanticError(std::format("redefinition of '{}' with a different type",
                        //                    varName),
                        //                    varDecl.GetLocation());

                        // TODO: If types are equal, handle static/extern
                        // m_CurrentScope->UpdateSymbol(Scope::NamespaceType::Ident, varName, &varDecl);
                    }
                } else {  // Global variable tentative definition
                    // TODO: Compare types
                    // printSemanticError(std::format("redefinition of '{}' with a different type",
                    //                    varName),
                    //                    varDecl.GetLocation());

                    // TODO: If types are equal, handle static/extern
                }
            }
        } else {
            m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, varName, &varDecl);
        }
    } else {  // Local variable
        if (m_CurrentScope->HasSymbol(Scope::NamespaceType::Ident, varName)) {
            printSemanticError(std::format("redefinition of local variable '{}'", varName),
                                varDecl.GetLocation());
        } else {
            m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, varName, &varDecl);
        }
    }

    if (!varDecl.HasInit()) {
        return;
    }

    Expression* initExpr = varDecl.GetInit();
    initExpr->Accept(*this);

    // TODO: Evaluate global variable constant initializer
    // TODO: Handle initializer lists

    varDeclQualType = varDecl.GetType();
    QualType exprQualType = initExpr->GetType();

    if (checkArrayInitialization(varDeclQualType, initExpr)) {
        return;
    }

    bool isInitNullPtr = isNullPointerConstant(initExpr);

    if (initExpr->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(varDecl.GetInit(),
                                                             CastExpression::Kind::kLValueToRValue);
        varDecl.SetInit(cast);
        exprQualType = cast->GetType();
    }

    QualType resultUnqualType = varDeclQualType; 
    resultUnqualType.RemoveQualifiers();

    bool ptrNull = isPointerType(varDeclQualType) && isInitNullPtr;

    bool bothReal = isRealType(varDeclQualType) && isRealType(exprQualType);
    bool bothRecord = isRecordType(varDeclQualType) && isRecordType(exprQualType);
    bool bothPtr = isPointerType(varDeclQualType) && isPointerType(exprQualType);
    if (!bothReal && !bothRecord && !bothPtr && !ptrNull) {
        printSemanticError("initializing with an expression of incompatible type",
                            initExpr->GetLocation());
    }

    if (bothRecord) {
        if (!areCompatibleTypes(varDeclQualType, exprQualType, /*isPointer=*/false)) {
            printSemanticError("initializing with an expression of incompatible type",
                                initExpr->GetLocation());     
        }
    } else if (bothPtr && !isPointerToVoidType(varDeclQualType) && !isPointerToVoidType(exprQualType)) {
        auto* varDeclPtrType = dynamic_cast<PointerType*>(varDeclQualType.GetSubType());
        QualType varDeclSubType = varDeclPtrType->GetSubType();
        auto* exprPtrType = dynamic_cast<PointerType*>(exprQualType.GetSubType());
        QualType exprSubType = exprPtrType->GetSubType();

        if (!areEqualTypes(varDeclSubType.GetSubType(), exprSubType.GetSubType())) {
            printSemanticError("pointers to compatible types are required",
                                initExpr->GetLocation());  
        }

        if (!varDeclSubType.IsConst() && exprSubType.IsConst()) {
            printSemanticError("initializing discards qualifiers",
                                initExpr->GetLocation());  
        }
    }

    // TODO: Handle null pointer
    if (!areEqualQualTypes(resultUnqualType, exprQualType) && !ptrNull) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(varDecl.GetInit(),
                                                                resultUnqualType);
        varDecl.SetInit(cast);
    }
}


/*
=================================================================
                            Statement
=================================================================
*/


void SemanticAstVisitor::Visit(CaseStatement& caseStmt) {
    if (!m_InsideSwitch) {
        printSemanticError("'case' statement not in switch statement",
                            caseStmt.GetLocation());  
    }

    ConstExpression* constExpr = caseStmt.GetExpression();
    constExpr->Accept(*this);

    Statement* body = caseStmt.GetBody();
    body->Accept(*this);
}

void SemanticAstVisitor::Visit(CompoundStatement& compoundStmt) {
    Scope* oldScope = m_CurrentScope;
    if (!m_IgnoreCompoundScope) {
        m_CurrentScope = m_SymbolTable.CreateScope(/*name=*/"[compound stmt]", m_CurrentScope);
    }
    m_IgnoreCompoundScope = false;

    for (Statement* stmt : compoundStmt.GetBody()) {
        stmt->Accept(*this);
    }

    m_CurrentScope = oldScope; 
}

void SemanticAstVisitor::Visit(DeclStatement& declStmt) {
    for (Declaration* decl : declStmt.GetDeclarations()) {
        decl->Accept(*this);
    }
}

void SemanticAstVisitor::Visit(DefaultStatement& defaultStmt) {
    if (!m_InsideSwitch) {
        printSemanticError("'default' statement not in switch statement",
                            defaultStmt.GetLocation());  
    }

    Statement* body = defaultStmt.GetBody();
    body->Accept(*this);
}

void SemanticAstVisitor::Visit(DoStatement& doStmt) {
    Expression* cond = doStmt.GetCondition();
    cond->Accept(*this);

    Statement* body = doStmt.GetBody();

    bool wasInsideLoop = m_InsideLoop;
    m_InsideLoop = true;
    body->Accept(*this);
    m_InsideLoop = wasInsideLoop;    
}

void SemanticAstVisitor::Visit(ForStatement& forStmt) {
    m_CurrentScope = m_SymbolTable.CreateScope("[for stmt]", m_CurrentScope);

    if (forStmt.HasInit()) {
        forStmt.GetInit()->Accept(*this);
    }

    if (forStmt.HasCondition()) {
        forStmt.GetCondition()->Accept(*this);
    }

    if (forStmt.HasStep()) {
        forStmt.GetStep()->Accept(*this);
    }

    Statement* body = forStmt.GetBody();

    bool wasInsideLoop = m_InsideLoop;
    m_InsideLoop = true;
    m_IgnoreCompoundScope = true;
    body->Accept(*this);
    m_InsideLoop = wasInsideLoop;

    m_CurrentScope = m_CurrentScope->GetParentScope(); 
}

void SemanticAstVisitor::Visit(GotoStatement& gotoStmt) {
    LabelDeclaration* labelOldDecl = gotoStmt.GetLabel();
    std::string declName = labelOldDecl->GetName();

    if (!m_FunctionScope->HasSymbol(Scope::NamespaceType::Label, declName)) {
        m_UnlabeledGotos[declName] = &gotoStmt;
        return;
    }

    Declaration* decl = m_FunctionScope->GetSymbol(Scope::NamespaceType::Label, declName);
    auto* labelDecl = dynamic_cast<LabelDeclaration*>(decl);
    assert(labelDecl);

    gotoStmt.SetLabel(labelDecl);
}

void SemanticAstVisitor::Visit(IfStatement& ifStmt) {
    Expression* cond = ifStmt.GetCondition();
    cond->Accept(*this);

    Statement* thenStmt = ifStmt.GetThen();
    thenStmt->Accept(*this);

    Statement* elseStmt = ifStmt.GetElse();
    if (elseStmt) {
        elseStmt->Accept(*this);
    }
}

void SemanticAstVisitor::Visit(LabelStatement& labelStmt) {
    LabelDeclaration* labelDecl = labelStmt.GetLabel();
    labelDecl->Accept(*this);

    Statement* body = labelStmt.GetBody();
    body->Accept(*this);
}

void SemanticAstVisitor::Visit(LoopJumpStatement& loopJmpStmt) {
    if (loopJmpStmt.GetType() == LoopJumpStatement::Type::kBreak) {
        if (!m_InsideLoop && !m_InsideSwitch) {
            printSemanticError("'break' statement not in loop or switch statement",
                                loopJmpStmt.GetLocation());  
        }
    } else if (!m_InsideLoop) {
        printSemanticError("'continue' statement not in loop statement",
                            loopJmpStmt.GetLocation());  
    }
}

void SemanticAstVisitor::Visit(ReturnStatement& returnStmt) {
    QualType funcQualType = m_CurrentFunctionDecl->GetType();
    auto* funcType = dynamic_cast<FunctionType*>(funcQualType.GetSubType());
    
    QualType returnQualType = funcType->GetSubType();
    if (returnStmt.HasReturnExpression() && isVoidType(returnQualType)) {
        printSemanticError("void function should not return a value",
                            returnStmt.GetLocation());
    }

    if (!returnStmt.HasReturnExpression()) {
        if (!isVoidType(returnQualType)) {
            printSemanticError("non-void function should return a value",
                                returnStmt.GetLocation());      
        }
        m_HasReturn = false;
        return;
    }
    m_HasReturn = true;

    Expression* retExpr = returnStmt.GetReturnExpression();
    retExpr->Accept(*this);

    returnQualType.RemoveQualifiers();

    QualType exprQualType = retExpr->GetType();
    if (retExpr->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(retExpr,
                                                             CastExpression::Kind::kLValueToRValue);
        returnStmt.SetReturnExpression(cast);
        exprQualType = cast->GetType();
    }

    bool bothReal = isRealType(returnQualType) && isRealType(exprQualType);
    bool bothRecord = isRecordType(returnQualType) && isRecordType(exprQualType);
    bool bothPtr = isPointerType(returnQualType) && isPointerType(exprQualType);
    bool ptrNull = isPointerType(returnQualType) && isNullPointerConstant(retExpr);
    if (!bothReal && !bothRecord && !bothPtr && !ptrNull) {
        printSemanticError("returning from a function with incompatible result type",
                            retExpr->GetLocation());
    }

    if (bothRecord) {
        if (!areCompatibleTypes(returnQualType, exprQualType, /*isPointer=*/false)) {
            printSemanticError("returning record from a function with incompatible result type",
                                retExpr->GetLocation());     
        }
    } else if (bothPtr && !isPointerToVoidType(returnQualType) && !isPointerToVoidType(exprQualType)) {
        if (!areCompatibleTypes(returnQualType, exprQualType, /*isPointer=*/true)) {
            printSemanticError("incompatible pointer types returning from a function",
                                retExpr->GetLocation());  
        }
    }

    // TODO: Handle null pointer
    if (!areEqualQualTypes(returnQualType, exprQualType) && !ptrNull) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(returnStmt.GetReturnExpression(),
                                                                returnQualType);
        returnStmt.SetReturnExpression(cast);
    }
}

void SemanticAstVisitor::Visit(SwitchStatement& switchStmt) {
    Expression* expr = switchStmt.GetExpression();
    expr->Accept(*this);

    Statement* body = switchStmt.GetBody();

    bool wasInsideSwitch = m_InsideSwitch;
    m_InsideSwitch = true;
    body->Accept(*this);
    m_InsideSwitch = wasInsideSwitch;
}

void SemanticAstVisitor::Visit(WhileStatement& whileStmt) {
    Expression* cond = whileStmt.GetCondition();
    cond->Accept(*this);

    Statement* body = whileStmt.GetBody();

    bool wasInsideLoop = m_InsideLoop;
    m_InsideLoop = true;
    body->Accept(*this);
    m_InsideLoop = wasInsideLoop;
}


/*
=================================================================
                            Expression
=================================================================
*/

void SemanticAstVisitor::VisitAssignmentExpression(BinaryExpression& assignExpr) {
    Expression* leftOperand = assignExpr.GetLeftOperand();
    leftOperand->Accept(*this);        

    Expression* rightOperand = assignExpr.GetRightOperand();
    rightOperand->Accept(*this);

    QualType leftQualType = leftOperand->GetType();
    QualType rightQualType = rightOperand->GetType();

    if (!isModifiableLValue(*leftOperand)) {
        printSemanticError("modifiable lvalue as left operand is required",
                            assignExpr.GetLocation());  
    }

    bool isRightNullPtr = isNullPointerConstant(rightOperand);

    assignExpr.SetRValue();
    if (rightOperand->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(assignExpr.GetRightOperand(),
                                                             CastExpression::Kind::kLValueToRValue);
        assignExpr.SetRightOperand(cast);
        rightQualType = cast->GetType();
    }

    QualType resultUnqualType = leftQualType; 
    resultUnqualType.RemoveQualifiers();
    assignExpr.SetType(resultUnqualType);

    bool ptrNull = isPointerType(leftQualType) && isRightNullPtr;

    BinaryExpression::OpType exprType = assignExpr.GetOpType();
    if (exprType == BinaryExpression::OpType::kAssign) {
        bool bothReal = isRealType(leftQualType) && isRealType(rightQualType);
        bool bothRecord = isRecordType(leftQualType) && isRecordType(rightQualType);
        bool bothPtr = isPointerType(leftQualType) && isPointerType(rightQualType);
        if (!bothReal && !bothRecord && !bothPtr && !ptrNull) {
            printSemanticError("invalid operands to assign expression",
                                assignExpr.GetLocation());
        }

        if (bothRecord) {
            if (!areCompatibleTypes(leftQualType, rightQualType, /*isPointer=*/false)) {
                printSemanticError("assigning from incompatible tag type",
                                    assignExpr.GetLocation());     
            }
        } else if (bothPtr && !isPointerToVoidType(leftQualType) && !isPointerToVoidType(rightQualType)) {
            auto* leftPtrType = static_cast<PointerType*>(leftQualType.GetSubType());
            QualType leftSubType = leftPtrType->GetSubType();
            auto* rightPtrType = static_cast<PointerType*>(rightQualType.GetSubType());
            QualType rightSubType = rightPtrType->GetSubType();

            if (!areEqualTypes(leftSubType.GetSubType(), rightSubType.GetSubType())) {
                printSemanticError("pointers to compatible types are required",
                                    assignExpr.GetLocation());  
            }

            if (!leftSubType.IsConst() && rightSubType.IsConst()) {
                printSemanticError("assigning discards qualifiers",
                                    assignExpr.GetLocation());  
            }
        }

        // TODO: Handle null pointer
        if (!areEqualQualTypes(resultUnqualType, rightQualType) && !ptrNull) {
            auto* cast = m_Program.CreateAstNode<CastExpression>(assignExpr.GetRightOperand(),
                                                                    resultUnqualType);
            assignExpr.SetRightOperand(cast);     
        }

        return;
    }


    QualType resultType = rightQualType;
    if (isRealType(leftQualType) && isRealType(rightQualType)) {
        auto* leftRealType = static_cast<BuiltinType*>(leftQualType.GetSubType());
        auto* rightRealType = static_cast<BuiltinType*>(rightQualType.GetSubType());
        if (rightRealType->GetRank() < leftRealType->GetRank()) {
            resultType = leftQualType;
        }
    }
    if (isIntegerType(resultType)) {
        resultType = promoteIntegerType(resultType);
    }
    if (!areEqualQualTypes(resultType, rightQualType)) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(assignExpr.GetRightOperand(),
                                                                resultType);
        assignExpr.SetRightOperand(cast);
    }

    if (exprType == BinaryExpression::OpType::kMulAssign ||
            exprType == BinaryExpression::OpType::kDivAssign) {
        if (!isRealType(leftQualType) || !isRealType(rightQualType)) {
            printSemanticError("arithmetic type is required",
                                assignExpr.GetLocation());  
        }
    } else if (exprType == BinaryExpression::OpType::kRemAssign || assignExpr.IsBitwiseAssign()) {
        if (!isIntegerType(leftQualType) || !isIntegerType(rightQualType)) {
            printSemanticError("integer type is required",
                                assignExpr.GetLocation());  
        }   
    } else if (exprType == BinaryExpression::OpType::kAddAssign ||
                exprType == BinaryExpression::OpType::kSubAssign) {
        bool bothReal = isRealType(leftQualType) && isRealType(rightQualType);
        bool ptrInt = isPointerType(leftQualType) && isIntegerType(rightQualType);
        if (!bothReal && !ptrInt) {
            printSemanticError(std::format("invalid operands to compound assign '{}'",
                                            assignExpr.GetOpTypeStr()),
                                assignExpr.GetLocation());       
        }

        if (ptrInt && isPointerToIncompleteType(leftQualType)) {
            printSemanticError("arithmetic on a pointer to an incomplete type",
                                assignExpr.GetLocation());     
        }    
    }
}

void SemanticAstVisitor::VisitArrSubscriptExpression(BinaryExpression& arrExpr) {
    Expression* leftOperand = arrExpr.GetLeftOperand();
    leftOperand->Accept(*this);        

    Expression* rightOperand = arrExpr.GetRightOperand();
    rightOperand->Accept(*this);

    QualType leftQualType = leftOperand->GetType();
    QualType rightQualType = rightOperand->GetType();

    if (isIntegerType(leftQualType)) {
        arrExpr.SetLeftOperand(rightOperand);
        arrExpr.SetRightOperand(leftOperand);
        std::swap(leftOperand, rightOperand);
        std::swap(leftQualType, rightQualType);
    }

    if (!isIntegerType(rightQualType)) {
        printSemanticError("array subscript is not an integer", arrExpr.GetLocation());
    }

    if (auto* arrayType = dynamic_cast<ArrayType*>(leftQualType.GetSubType())) {
        leftQualType = *decayType(leftQualType.GetSubType());
    }

    if (!isPointerType(leftQualType)) {
        printSemanticError("subscripted value is not an array or pointer", arrExpr.GetLocation());
    }

    arrExpr.SetLValue();
    if (rightOperand->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(arrExpr.GetRightOperand(),
                                                             CastExpression::Kind::kLValueToRValue);
        arrExpr.SetRightOperand(cast);
    }

    auto* ptrType = static_cast<PointerType*>(leftQualType.GetSubType());
    auto decayedQualTypeOpt = decayType(ptrType->GetSubType());
    if (decayedQualTypeOpt) {
        arrExpr.SetType(*decayedQualTypeOpt);
    } else {
        arrExpr.SetType(ptrType->GetSubType());
    }
}

void SemanticAstVisitor::VisitMemberExpression(BinaryExpression& memberExpr) {
    Expression* leftOperand = memberExpr.GetLeftOperand();
    leftOperand->Accept(*this);        

    Expression* rightOperand = memberExpr.GetRightOperand();
    rightOperand->Accept(*this);

    QualType leftQualType = leftOperand->GetType();
    QualType rightQualType = rightOperand->GetType();

    memberExpr.SetLValue();

    RecordType* recordType = nullptr;
    if (memberExpr.GetOpType() == BinaryExpression::OpType::kDirectMember) {
        if (!isRecordType(leftQualType)) {
            printSemanticError("member reference base type is not a structure or union",
                                memberExpr.GetLocation());
        }
        recordType = static_cast<RecordType*>(leftQualType.GetSubType());
    } else {
        if (!isPointerType(leftQualType)) {
            printSemanticError("member reference type is not a pointer",
                                memberExpr.GetLocation());
        }

        auto* ptrType = static_cast<PointerType*>(leftQualType.GetSubType());
        QualType ptrSubQualType = ptrType->GetSubType();
        if (!isRecordType(ptrSubQualType)) {
            printSemanticError("member reference base type is not a structure or union",
                                memberExpr.GetLocation());
        }
        recordType = static_cast<RecordType*>(ptrSubQualType.GetSubType());

        auto* cast = m_Program.CreateAstNode<CastExpression>(memberExpr.GetLeftOperand(),
                                                             CastExpression::Kind::kLValueToRValue);
        memberExpr.SetLeftOperand(cast);
    }

    auto* declRefExpr = static_cast<DeclRefExpression*>(rightOperand);
    auto* fieldDecl = static_cast<FieldDeclaration*>(declRefExpr->GetDeclaration());
    std::string fieldName = fieldDecl->GetName();

    RecordDeclaration* recordDecl = recordType->GetDeclaration();
    FieldDeclaration* actualFieldDecl = recordDecl->GetField(fieldName);
    if (!actualFieldDecl) {
        printSemanticError(std::format("no member named '{}' in record", fieldName),
                            memberExpr.GetLocation());
    }

    rightOperand->SetLValue();
    rightOperand->SetType(actualFieldDecl->GetType());

    memberExpr.SetType(actualFieldDecl->GetType());

    declRefExpr->SetDeclaration(actualFieldDecl);
}

void SemanticAstVisitor::Visit(BinaryExpression& binaryExpr) {
    if (binaryExpr.IsAssignment()) {
        VisitAssignmentExpression(binaryExpr);
        return;
    }

    if (binaryExpr.GetOpType() == BinaryExpression::OpType::kArrSubscript) {
        VisitArrSubscriptExpression(binaryExpr);
        return;
    }

    if (binaryExpr.GetOpType() == BinaryExpression::OpType::kDirectMember ||
            binaryExpr.GetOpType() == BinaryExpression::OpType::kArrowMember) {
        VisitMemberExpression(binaryExpr);
        return;
    }

    assert(binaryExpr.IsArithmetic() || binaryExpr.IsRelational() || binaryExpr.IsEquality());

    Expression* leftOperand = binaryExpr.GetLeftOperand();
    leftOperand->Accept(*this);        

    Expression* rightOperand = binaryExpr.GetRightOperand();
    rightOperand->Accept(*this);

    QualType leftQualType = leftOperand->GetType();
    QualType rightQualType = rightOperand->GetType();

    bool isLeftNullPtr = isNullPointerConstant(leftOperand);
    bool isRightNullPtr = isNullPointerConstant(rightOperand);

    binaryExpr.SetRValue();
    if (leftOperand->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(binaryExpr.GetLeftOperand(),
                                                             CastExpression::Kind::kLValueToRValue);
        binaryExpr.SetLeftOperand(cast);
        leftQualType = cast->GetType();
    }
    if (rightOperand->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(binaryExpr.GetRightOperand(),
                                                             CastExpression::Kind::kLValueToRValue);
        binaryExpr.SetRightOperand(cast);
        rightQualType = cast->GetType();
    }

    QualType leftResultType = leftQualType;
    QualType rightResultType = rightQualType;

    QualType leftPromotedType = leftQualType;
    if (isIntegerType(leftQualType)) {
        leftPromotedType = promoteIntegerType(leftQualType);
        leftResultType = leftPromotedType;
    }
    QualType rightPromotedType = rightQualType;
    if (isIntegerType(rightQualType)) {
        rightPromotedType = promoteIntegerType(rightQualType);
        rightResultType = rightPromotedType;
    }

    BinaryExpression::OpType exprType = binaryExpr.GetOpType();
    if (exprType == BinaryExpression::OpType::kMul ||
            exprType == BinaryExpression::OpType::kDiv) {
        if (!isRealType(leftQualType) || !isRealType(rightQualType)) {
            printSemanticError("arithmetic type is required",
                                binaryExpr.GetLocation());  
        }
        leftResultType = getCommonRealType(leftPromotedType, rightPromotedType);
        rightResultType = leftResultType;
        binaryExpr.SetType(leftResultType);
    } else if (exprType == BinaryExpression::OpType::kRem || binaryExpr.IsBitwise()) {
        if (!isIntegerType(leftQualType) || !isIntegerType(rightQualType)) {
            printSemanticError("integer type is required",
                                binaryExpr.GetLocation());  
        }
        leftResultType = getCommonRealType(leftPromotedType, rightPromotedType);
        rightResultType = leftResultType;
        binaryExpr.SetType(leftResultType);
    } else if (exprType == BinaryExpression::OpType::kAdd) {
        bool bothReal = isRealType(leftQualType) && isRealType(rightQualType);
        bool ptrInt = isPointerType(leftQualType) && isIntegerType(rightQualType);
        bool intPtr = isIntegerType(leftQualType) && isPointerType(rightQualType);
        if (!bothReal && !ptrInt && !intPtr) {
            printSemanticError("invalid operands to binary '+'",
                                binaryExpr.GetLocation());       
        }
    
        if (ptrInt && isPointerToIncompleteType(leftQualType) ||
                intPtr && isPointerToIncompleteType(rightQualType)) {
            printSemanticError("arithmetic on a pointer to an incomplete type",
                                binaryExpr.GetLocation());       
        }

        if (bothReal) {
            leftResultType = getCommonRealType(leftPromotedType, rightPromotedType);
            rightResultType = leftResultType;
            binaryExpr.SetType(leftResultType);
        } else if (ptrInt) {
            binaryExpr.SetType(leftQualType);
        } else {
            binaryExpr.SetType(rightQualType);
        }
    } else if (exprType == BinaryExpression::OpType::kSub) {
        bool bothReal = isRealType(leftQualType) && isRealType(rightQualType);
        bool bothPtr = isPointerType(leftQualType) && isPointerType(rightQualType);
        bool ptrInt = isPointerType(leftQualType) && isIntegerType(rightQualType);
        if (!bothReal && !bothPtr && !ptrInt) {
            printSemanticError("invalid operands to binary '-'",
                                binaryExpr.GetLocation());       
        }
        if (bothReal) {
            leftResultType = getCommonRealType(leftPromotedType, rightPromotedType);
            rightResultType = leftResultType;
            binaryExpr.SetType(leftResultType);
        } else if (bothPtr) {
            if (!areCompatibleTypes(leftQualType, rightQualType, /*isPointer=*/true)) {
                printSemanticError("pointers to compatible types are required",
                                    binaryExpr.GetLocation());  
            }
            if (isPointerToIncompleteType(leftQualType) || isPointerToIncompleteType(rightQualType)) {
                printSemanticError("arithmetic on a pointer to an incomplete type",
                                    binaryExpr.GetLocation());     
            }
            binaryExpr.SetType(QualType{m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kLong)});
        } else {
            if (isPointerToIncompleteType(leftQualType)) {
                printSemanticError("arithmetic on a pointer to an incomplete type",
                                    binaryExpr.GetLocation());     
            }
            binaryExpr.SetType(leftQualType);
        }
    } else if (exprType == BinaryExpression::OpType::kLogAnd ||
                exprType == BinaryExpression::OpType::kLogOr) {
        if (!isScalarType(leftQualType) || !isScalarType(rightQualType)) {
            printSemanticError("scalar type is required",
                                binaryExpr.GetLocation());  
        }
        binaryExpr.SetType(QualType{m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt)});
    } else if (binaryExpr.IsRelational()) {
        bool bothReal = isRealType(leftQualType) && isRealType(rightQualType);
        bool bothPtr = isPointerType(leftQualType) && isPointerType(rightQualType);
        if (!bothReal && !bothPtr) {
            printSemanticError(std::format("invalid operands to binary '{}'",
                                            binaryExpr.GetOpTypeStr()),
                                binaryExpr.GetLocation());    
        }  
        if (bothReal) {
            leftResultType = getCommonRealType(leftPromotedType, rightPromotedType);
            rightResultType = leftResultType;
        } else if (bothPtr) {
            // TODO: Handle void pointer
            if (!areCompatibleTypes(leftQualType, rightQualType, /*isPointer=*/true)) {
                printSemanticError("pointers to compatible types are required",
                                    binaryExpr.GetLocation());  
            }
        }
        binaryExpr.SetType(QualType{m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt)});
    } else if (binaryExpr.IsEquality()) {
        bool bothReal = isRealType(leftQualType) && isRealType(rightQualType);
        bool bothPtr = isPointerType(leftQualType) && isPointerType(rightQualType);
        bool ptrNull = isPointerType(leftQualType) && isRightNullPtr;
        bool nullPtr = isLeftNullPtr && isPointerType(rightQualType);
        if (!bothReal && !bothPtr && !ptrNull && !nullPtr) {
            printSemanticError(std::format("invalid operands to binary '{}'",
                                            binaryExpr.GetOpTypeStr()),
                                binaryExpr.GetLocation());    
        }

        bool ptrVoid = isPointerType(leftQualType) && isPointerToVoidType(rightQualType);
        bool voidPtr = isPointerToVoidType(leftQualType) && isPointerType(rightQualType);
        if (bothReal) {
            leftResultType = getCommonRealType(leftPromotedType, rightPromotedType);
            rightResultType = leftResultType;
        } else if (ptrVoid) {
            rightResultType = leftResultType;
        } else if (voidPtr) {
            leftResultType = rightResultType;
        } else if (bothPtr) {
            if (!isLeftNullPtr && !isRightNullPtr &&
                    !areCompatibleTypes(leftQualType, rightQualType, /*isPointer=*/true)) {
                printSemanticError("pointers to compatible types are required",
                                    binaryExpr.GetLocation()); 
            }
        }

        // TODO: Handle null pointer

        binaryExpr.SetType(QualType{m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt)});
    }

    if (!areEqualQualTypes(leftQualType, leftResultType)) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(binaryExpr.GetLeftOperand(),
                                                                leftResultType);
        binaryExpr.SetLeftOperand(cast);     
    }
    if (!areEqualQualTypes(rightQualType, rightResultType)) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(binaryExpr.GetRightOperand(),
                                                                rightResultType);
        binaryExpr.SetRightOperand(cast);     
    }
}

void SemanticAstVisitor::Visit(CallExpression& callExpr) {
    Expression* calleeExpr = callExpr.GetCallee();
    calleeExpr->Accept(*this);

    QualType calleeQualType = calleeExpr->GetType();

    std::vector<Expression*> argumentExprs = callExpr.GetArguments();
    std::vector<QualType> argQualTypes;
    argQualTypes.reserve(argumentExprs.size());
    for (Expression* argExpr : argumentExprs) {
        argExpr->Accept(*this);
        argQualTypes.push_back(argExpr->GetType());
    }

    if (!isPointerToFunctionType(calleeQualType)) {
        printSemanticError("called object type is not a function or function pointer",
                            callExpr.GetLocation());  
    }

    auto* pointerType = static_cast<PointerType*>(calleeQualType.GetSubType());
    QualType functionSubType = pointerType->GetSubType();
    auto* functionType = static_cast<FunctionType*>(functionSubType.GetSubType());

    callExpr.SetRValue();
    callExpr.SetType(functionType->GetSubType());

    std::vector<QualType> paramQualTypes = functionType->GetParamTypes();
    if (argQualTypes.size() < paramQualTypes.size()) {
        printSemanticError("too few arguments to function call",
                            callExpr.GetLocation());  
    } else if (argQualTypes.size() > paramQualTypes.size() && !functionType->IsVariadic()) {
        printSemanticError("too many arguments to function call",
                            callExpr.GetLocation());  
    }

    for (size_t i = 0; i < argQualTypes.size(); ++i) {
        QualType argQualType = argQualTypes[i];

        Expression* argExpr = argumentExprs[i];
        if (argExpr->IsLValue()) {
            auto* cast = m_Program.CreateAstNode<CastExpression>(argExpr,
                                                                 CastExpression::Kind::kLValueToRValue);
            callExpr.SetArgument(i, cast);
            argQualType = cast->GetType();
        }

        if (i >= paramQualTypes.size()) {
            auto* builtinType = dynamic_cast<BuiltinType*>(argQualType.GetSubType());
            if (builtinType) {
                if (builtinType->IsSinglePrecision()) {
                    auto* doubleType = m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kDouble);
                    auto* cast = m_Program.CreateAstNode<CastExpression>(
                                                callExpr.GetArgument(i), QualType(doubleType));
                    callExpr.SetArgument(i, cast);
                } else {
                    QualType promotedType = promoteIntegerType(argQualType);
                    if (!areEqualQualTypes(argQualType, promotedType)) {
                        auto* cast = m_Program.CreateAstNode<CastExpression>(
                                                    callExpr.GetArgument(i), promotedType);
                        callExpr.SetArgument(i, cast);  
                    }
                }
            }

            continue;
        }

        QualType paramQualType = paramQualTypes[i];
        paramQualType.RemoveQualifiers();

        bool bothReal = isRealType(paramQualType) && isRealType(argQualType);
        bool bothRecord = isRecordType(paramQualType) && isRecordType(argQualType);
        bool bothPtr = isPointerType(paramQualType) && isPointerType(argQualType);
        bool ptrNull = isPointerType(paramQualType) && isNullPointerConstant(argExpr);
        if (!bothReal && !bothRecord && !bothPtr && !ptrNull) {
            printSemanticError("incompatible argument and parameter types",
                                argExpr->GetLocation());
        }

        if (bothRecord) {
            if (!areCompatibleTypes(paramQualType, argQualType, /*isPointer=*/false)) {
                printSemanticError("incompatible argument and parameter types",
                                    argExpr->GetLocation());     
            }
        } else if (bothPtr && !isPointerToVoidType(paramQualType) && !isPointerToVoidType(argQualType)) {
            if (!areCompatibleTypes(paramQualType, argQualType, /*isPointer=*/true)) {
                printSemanticError("pointers to compatible types are required",
                                    argExpr->GetLocation());  
            }
        }

        // TODO: Handle null pointer
        if (!areEqualQualTypes(paramQualType, argQualType) && !ptrNull) {
            auto* cast = m_Program.CreateAstNode<CastExpression>(callExpr.GetArgument(i),
                                                                 paramQualType);
            callExpr.SetArgument(i, cast);  
        }
    }
}

void SemanticAstVisitor::Visit(CastExpression& castExpr) {
    Expression* subExpr = castExpr.GetSubExpression();
    subExpr->Accept(*this);

    QualType subExprQualType = subExpr->GetType();

    QualType toQualType = castExpr.GetToType();
    // Type* toType = toQualType.GetSubType();
    // toType->Accept(*this);

    // TODO: function and array pointers decay
    if (!isVoidType(toQualType) && (!isScalarType(subExprQualType) ||
                                    !isScalarType(toQualType))) {
        printSemanticError("arithmetic or pointer type is required",
                            castExpr.GetLocation());  
    }

    if (isFloatType(subExprQualType) && isPointerType(toQualType)) {
        printSemanticError("operand of float type cannot be cast to a pointer type",
                            castExpr.GetLocation()); 
    }

    if (isPointerType(subExprQualType) && isFloatType(toQualType)) {
        printSemanticError("pointer cannot be cast to float type",
                            castExpr.GetLocation()); 
    }

    castExpr.SetType(toQualType);
    castExpr.SetRValue();

    if (subExpr->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(castExpr.GetSubExpression(),
                                                             CastExpression::Kind::kLValueToRValue);
        castExpr.SetSubExpression(cast);
    }
}

void SemanticAstVisitor::Visit(CharExpression& charExpr) {
    charExpr.SetType(QualType{
        m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kChar)});
    charExpr.SetRValue();
}

void SemanticAstVisitor::Visit(ConditionalExpression& condExpr) {
    Expression* condition = condExpr.GetCondition();
    condition->Accept(*this);

    Expression* trueExpr = condExpr.GetTrueExpression();
    trueExpr->Accept(*this);

    Expression* falseExpr = condExpr.GetFalseExpression();
    falseExpr->Accept(*this);

    QualType condQualType = condition->GetType();
    QualType trueQualType = trueExpr->GetType();
    QualType falseQualType = falseExpr->GetType();

    if (!isScalarType(condQualType)) {
        printSemanticError("arithmetic or pointer type is required for condition",
                            condExpr.GetLocation());     
    }

    bool isTrueNullPtr = isNullPointerConstant(trueExpr);
    bool isFalseNullPtr = isNullPointerConstant(falseExpr);

    condExpr.SetRValue();
    if (condition->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(condExpr.GetCondition(),
                                                             CastExpression::Kind::kLValueToRValue);
        condExpr.SetCondition(cast);
        condQualType = cast->GetType();
    }
    if (trueExpr->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(condExpr.GetTrueExpression(),
                                                             CastExpression::Kind::kLValueToRValue);
        condExpr.SetTrueExpression(cast);
        trueQualType = cast->GetType();
    }
    if (falseExpr->IsLValue()) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(condExpr.GetFalseExpression(),
                                                             CastExpression::Kind::kLValueToRValue);
        condExpr.SetFalseExpression(cast);
        falseQualType = cast->GetType();
    }

    bool bothReal = isRealType(trueQualType) && isRealType(falseQualType);
    bool bothRecord = isRecordType(trueQualType) && isRecordType(falseQualType);
    bool bothVoid = isVoidType(trueQualType) && isVoidType(falseQualType);
    bool bothPtr = isPointerType(trueQualType) && isPointerType(falseQualType);
    bool ptrNull = isPointerType(trueQualType) && isFalseNullPtr;
    bool nullPtr = isTrueNullPtr && isPointerType(falseQualType);
    if (!bothReal && !bothRecord && !bothVoid && !bothPtr && !ptrNull && !nullPtr) {
        printSemanticError("invalid true and false operands to conditional expression",
                            condExpr.GetLocation());  
    }

    bool ptrVoid = isPointerType(trueQualType) && isPointerToVoidType(falseQualType);
    bool voidPtr = isPointerToVoidType(trueQualType) && isPointerType(falseQualType);

    QualType trueResultType = trueQualType;
    QualType falseResultType = falseQualType;

    if (bothReal) {
        trueResultType = promoteIntegerType(trueQualType);
        falseResultType = promoteIntegerType(falseQualType);

        trueResultType = getCommonRealType(trueResultType, falseResultType);
        falseResultType = trueResultType;
    } else if (bothRecord) {
        if (!areCompatibleTypes(trueQualType, falseQualType, /*isPointer=*/false)) {
            printSemanticError("incompatible operand record types",
                                condExpr.GetLocation());     
        }
        condExpr.SetType(trueQualType);
    } else if (bothVoid) {  // TODO: Make "appropriately qualified version"
        condExpr.SetType(trueQualType);
    } else if (ptrVoid) {
        trueResultType = falseQualType;
        condExpr.SetType(falseQualType);
    } else if (voidPtr) {
        falseQualType = trueResultType;
        condExpr.SetType(trueQualType);
    } else if (bothPtr) {
        if (!isTrueNullPtr && !isFalseNullPtr &&
                !areCompatibleTypes(trueQualType, falseQualType, /*isPointer=*/true)) {
            printSemanticError("pointers to compatible types are required",
                                condExpr.GetLocation()); 
        }
    } else if (ptrNull) {  // TODO: Cast null pointer constant?
        condExpr.SetType(trueQualType);
    } else if (nullPtr) {
        condExpr.SetType(falseQualType);
    }

    if (!areEqualQualTypes(trueResultType, trueQualType)) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(condExpr.GetTrueExpression(),
                                                                trueResultType);
        condExpr.SetTrueExpression(cast);
    }
    if (!areEqualQualTypes(falseResultType, falseQualType)) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(condExpr.GetFalseExpression(),
                                                                falseResultType);
        condExpr.SetFalseExpression(cast);
    }
}

void SemanticAstVisitor::Visit(ConstExpression& constExpr) {
    Expression* expr = constExpr.GetExpression();
    expr->Accept(*this);

    IntConstExprAstVisitor constExprVisitor;
    IntConstExprAstVisitor::Status status = constExprVisitor.Evaluate(constExpr);
    if (status == IntConstExprAstVisitor::Status::kError) {
        printSemanticError("expression is not an integer constant expression",
                            constExpr.GetLocation()); 
    }
}

void SemanticAstVisitor::Visit(DeclRefExpression& declrefExpr) {
    ValueDeclaration* oldDecl = declrefExpr.GetDeclaration();
    std::string declName = oldDecl->GetName();

    if (dynamic_cast<FieldDeclaration*>(oldDecl)) {
        return;
    }

    if (auto declOpt = m_CurrentScope->FindSymbol(Scope::NamespaceType::Ident, declName)) {
        Declaration* decl = *declOpt;
        auto* valDecl = dynamic_cast<ValueDeclaration*>(decl);
        assert(valDecl);
        declrefExpr.SetDeclaration(valDecl);
    } else {
        printSemanticError(std::format("use of undeclared identifier '{}'", declName),
                            declrefExpr.GetLocation());
    }

    ValueDeclaration* decl = declrefExpr.GetDeclaration();
    QualType declQualType = decl->GetType();
    auto decayedQualTypeOpt = decayType(declQualType);
    if (decayedQualTypeOpt) {
        declrefExpr.SetType(*decayedQualTypeOpt);
        declrefExpr.SetRValue();
    } else {
        declrefExpr.SetType(declQualType);
        declrefExpr.SetLValue();
    }
}

void SemanticAstVisitor::Visit(ExpressionList& exprList) {
    std::vector<Expression*> expressions = exprList.GetExpressions();
    for (size_t i = 0; i < expressions.size(); ++i) {
        Expression* expr = expressions[i];
        expr->Accept(*this);

        if (expr->IsLValue()) {
            auto* cast = m_Program.CreateAstNode<CastExpression>(
                            expr, CastExpression::Kind::kLValueToRValue);
            exprList.SetExpression(cast, i);
        }
    }

    exprList.SetRValue();

    Expression* lastExpr = exprList.GetLastExpression();
    exprList.SetType(lastExpr->GetType());
}

void SemanticAstVisitor::Visit(FloatExpression& floatExpr) {
    floatExpr.SetType(QualType{
        m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kDouble)});
    floatExpr.SetRValue();
}

void SemanticAstVisitor::Visit(InitializerList& initList) {
    for (Expression* init : initList.GetInits()) {
        init->Accept(*this);
    }

    // TODO: ...
    ANCL_CRITICAL("Initializer lists are not supported yet :(");
    throw std::runtime_error("Not implemented error");
    // exit(EXIT_FAILURE);
}

void SemanticAstVisitor::Visit(IntExpression& intExpr) {
    intExpr.SetType(QualType{
        m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt)});
    intExpr.SetRValue();
}

void SemanticAstVisitor::Visit(SizeofTypeExpression& sizeofTypeExpr) {
    QualType qualType = sizeofTypeExpr.GetSubType();
    qualType = AcceptQualType(qualType);

    if (isIncompleteType(qualType)) {
        printSemanticError("invalid application of 'sizeof' to an incomplete type",
                            sizeofTypeExpr.GetLocation());   
    }

    Type* sizeType = m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kULong);
    sizeofTypeExpr.SetType(QualType{sizeType});
    sizeofTypeExpr.SetRValue();
}

void SemanticAstVisitor::Visit(StringExpression& stringExpr) {
    std::string value = stringExpr.GetStringValue();

    auto* charType = m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kChar);
    // auto* constExpr = m_Program.CreateAstNode<ConstExpression>(Value(value.size(), false));
    // auto* arrayType = m_Program.CreateType<ArrayType>(QualType{charType}, constExpr);
    auto* pointerType = m_Program.CreateType<PointerType>(QualType{charType});
    stringExpr.SetType(QualType{pointerType});
    stringExpr.SetRValue();
}

void SemanticAstVisitor::Visit(UnaryExpression& unaryExpr) {
    Expression* operandExpr = unaryExpr.GetOperand();
    operandExpr->Accept(*this);

    QualType qualType = operandExpr->GetType();
    QualType resultType;

    bool isIncDec = false;

    UnaryExpression::OpType exprType = unaryExpr.GetOpType();
    if (exprType == UnaryExpression::OpType::kPreInc ||
            exprType == UnaryExpression::OpType::kPreDec ||
                exprType == UnaryExpression::OpType::kPostInc || 
                    exprType == UnaryExpression::OpType::kPostDec) {
        isIncDec = true;

        if (!isModifiableLValue(*operandExpr) || !isScalarType(qualType)) {
            printSemanticError("modifiable lvalue with scalar type is required "
                                "as increment or decrement operand",
                                unaryExpr.GetLocation());
        }

        // TODO: Handle pointer type

        resultType = qualType;
        unaryExpr.SetRValue();
    } else if (exprType == UnaryExpression::OpType::kAddrOf) {
        if (operandExpr->IsRValue()) {
            printSemanticError("cannot take the address of an rvalue",
                                unaryExpr.GetLocation());  
        }

        if (auto* declRefExpr = dynamic_cast<DeclRefExpression*>(operandExpr)) {
            ValueDeclaration* valueDecl = declRefExpr->GetDeclaration();
            bool isRegister = false;
            if (auto* varDecl = dynamic_cast<VariableDeclaration*>(valueDecl)) {
                if (varDecl->GetStorageClass() == StorageClass::kRegister) {
                    isRegister = true;
                }
            } else if (auto* paramDecl = dynamic_cast<ParameterDeclaration*>(valueDecl)) {
                if (paramDecl->GetStorageClass() == StorageClass::kRegister) {
                    isRegister = true;
                }
            }
            if (isRegister) {
                printSemanticError("address of register variable requested",
                                    unaryExpr.GetLocation()); 
            }
        }

        auto* ptrType = m_Program.CreateType<PointerType>(qualType);
        resultType = ptrType;
        qualType = resultType;
        unaryExpr.SetRValue();
    } else if (exprType == UnaryExpression::OpType::kDeref) {
        if (!isPointerType(qualType)) {
            printSemanticError("indirection requires pointer operand",
                                unaryExpr.GetLocation());  
        }

        auto* ptrType = static_cast<PointerType*>(qualType.GetSubType());
        resultType = ptrType->GetSubType();
        qualType = resultType;
        unaryExpr.SetLValue();
    } else if (exprType == UnaryExpression::OpType::kPlus || 
                exprType == UnaryExpression::OpType::kMinus) {
        if (!isRealType(qualType)) {
            printSemanticError(std::format("invalid type argument to unary '{}'",
                                            unaryExpr.GetOpTypeStr()),
                                unaryExpr.GetLocation());     
        }

        resultType = qualType;
        if (isIntegerType(qualType)) {
            resultType = promoteIntegerType(qualType);
        }

        unaryExpr.SetRValue();
    } else if (exprType == UnaryExpression::OpType::kNot) {
        if (!isIntegerType(qualType)) {
            printSemanticError("invalid type argument to unary '~'",
                                unaryExpr.GetLocation());
        }

        resultType = promoteIntegerType(qualType);
        unaryExpr.SetRValue();
    } else if (exprType == UnaryExpression::OpType::kLogNot) {
        if (!isScalarType(qualType)) {
            printSemanticError("invalid type argument to unary '!'",
                                unaryExpr.GetLocation());
        }

        Type* intType = m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kInt);
        resultType = QualType{intType};
        qualType = resultType;
        unaryExpr.SetRValue();
    } else if (exprType == UnaryExpression::OpType::kSizeof) {            
        if (isIncompleteType(qualType)) {
            printSemanticError("invalid application of 'sizeof' to an incomplete type",
                                unaryExpr.GetLocation());   
        }

        Type* sizeType = m_Program.CreateType<BuiltinType>(BuiltinType::Kind::kULong);
        resultType = QualType{sizeType};
        qualType = resultType;
        unaryExpr.SetRValue();
    }

    if ((operandExpr->IsLValue() && exprType != UnaryExpression::OpType::kAddrOf && !isIncDec) ||
                exprType == UnaryExpression::OpType::kDeref) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(unaryExpr.GetOperand(),
                                                             CastExpression::Kind::kLValueToRValue);
        unaryExpr.SetOperand(cast);
    }

    if (!areEqualQualTypes(resultType, qualType)) {
        auto* cast = m_Program.CreateAstNode<CastExpression>(unaryExpr.GetOperand(), resultType);
        unaryExpr.SetOperand(cast);
    }

    unaryExpr.SetType(resultType);
}


/*
=================================================================
                            Type
=================================================================
*/


void SemanticAstVisitor::Visit(ArrayType& arrayType) {
    ConstExpression* sizeExpr = arrayType.GetSizeExpression();
    if (sizeExpr) {
        sizeExpr->Accept(*this);
    }

    QualType subQualType = arrayType.GetSubType();
    Type* subType = subQualType.GetSubType();
    subType->Accept(*this);
}

void SemanticAstVisitor::Visit(EnumType& enumType) {
    EnumDeclaration* oldDecl = enumType.GetDeclaration();
    if (oldDecl->IsDefinition()) {
        return;
    }

    std::string enumName = oldDecl->GetName();

    auto declOpt = m_CurrentScope->FindSymbol(Scope::NamespaceType::Tag, enumName);
    if (!declOpt) {
        return;
    }

    Declaration* decl = *declOpt;
    auto* enumDecl = dynamic_cast<EnumDeclaration*>(decl);
    if (!enumDecl) {
        printSemanticError(std::format("use of '{}' with tag type that does "
                                        "not match previous declaration",
                                        enumName),
                            oldDecl->GetLocation());
        return;
    }

    enumType.SetDeclaration(enumDecl);
}

void SemanticAstVisitor::Visit(FunctionType& funcType) {
    QualType retQualType = funcType.GetSubType();
    Type* retType = retQualType.GetSubType();
    retType->Accept(*this);

    for (QualType& paramQualType : funcType.GetParamTypes()) {
        Type* paramType = paramQualType.GetSubType();
        paramType->Accept(*this);
    }
}

void SemanticAstVisitor::Visit(PointerType& ptrType) {
    QualType subQualType = ptrType.GetSubType();
    Type* subType = subQualType.GetSubType();
    subType->Accept(*this);
}

void SemanticAstVisitor::Visit(RecordType& recordType) {
    RecordDeclaration* oldDecl = recordType.GetDeclaration();
    if (oldDecl->IsDefinition()) {
        return;
    }

    std::string recordName = oldDecl->GetName();

    auto declOpt = m_CurrentScope->FindSymbol(Scope::NamespaceType::Tag, recordName);
    if (!declOpt) {
        return;
    }

    Declaration* decl = *declOpt;
    auto* recordDecl = dynamic_cast<RecordDeclaration*>(decl);
    if (!recordDecl || oldDecl->IsStruct() != recordDecl->IsStruct()) {
        printSemanticError(std::format("use of '{}' with tag type that does "
                                        "not match previous declaration",
                                        recordName),
                            oldDecl->GetLocation());
        return;
    }

    recordType.SetDeclaration(recordDecl);
}

void SemanticAstVisitor::Visit(TypedefType& typedefType) {
    TypedefDeclaration* oldDecl = typedefType.GetDeclaration();
    std::string typedefName = oldDecl->GetName();

    auto declOpt = m_CurrentScope->FindSymbol(Scope::NamespaceType::Ident, typedefName);
    if (!declOpt) {
        printSemanticError(std::format("use of undeclared identifier '{}'", typedefName),
                            oldDecl->GetLocation());
        return;
    }

    Declaration* decl = *declOpt;
    auto* typedefDecl = dynamic_cast<TypedefDeclaration*>(decl);
    if (!typedefDecl) {
        printSemanticError(std::format("typedef name '{}' is expected", typedefName),
                            oldDecl->GetLocation());
        return;
    }

    typedefType.SetDeclaration(typedefDecl);
}

bool SemanticAstVisitor::checkArrayInitialization(QualType arrayQualType, Expression* initExpr) {
    auto* arrayType = dynamic_cast<ArrayType*>(arrayQualType.GetSubType());
    if (!arrayType) {
        return false;
    }

    bool hasSize = arrayType->HasSize();
    IntValue sizeValue = arrayType->GetSize();
    uint64_t arraySize = sizeValue.GetUnsignedValue();

    if (auto* stringExpr = dynamic_cast<StringExpression*>(initExpr)) {
        std::string strValue = stringExpr->GetStringValue();
        if (!hasSize) {
            arrayType->SetSize(strValue.size() + 1);
        } else if (arraySize < strValue.size()) {
            printSemanticError("initializer-string for char array is too long",
                               initExpr->GetLocation());
        }

        return true;
    }

    // TODO: ...
    ANCL_CRITICAL("Initializer lists are not supported yet :(");
    throw std::runtime_error("Not implemented error");
    // exit(EXIT_FAILURE);

    // printSemanticError("array initializer must be an initializer list or string literal",
    //                    initExpr->GetLocation());

    return true;
}

std::optional<QualType> SemanticAstVisitor::decayType(QualType qualType) {
    Type* type = qualType.GetSubType();
    if (auto* arrayType = dynamic_cast<ArrayType*>(type)) {
        QualType memberQualType = arrayType->GetSubType();
        return m_Program.CreateType<PointerType>(memberQualType.GetSubType());
    }
    if (auto* funcType = dynamic_cast<FunctionType*>(type)) {
        return m_Program.CreateType<PointerType>(funcType);
    }
    return std::nullopt;
}

bool SemanticAstVisitor::areCompatibleTypes(QualType leftQualType, QualType rightQualType,
                                            bool isPointer) {
    if (isPointer) {
        auto* leftPtrType = static_cast<PointerType*>(leftQualType.GetSubType());
        leftQualType = leftPtrType->GetSubType();
        auto* rightPtrType = static_cast<PointerType*>(rightQualType.GetSubType());
        rightQualType = rightPtrType->GetSubType();
    }

    return areEqualTypes(leftQualType.GetSubType(), rightQualType.GetSubType());
}

QualType SemanticAstVisitor::promoteIntegerType(QualType qualType) {
    Type* type = qualType.GetSubType();
    auto* builtinType = dynamic_cast<BuiltinType*>(type);
    if (!builtinType) {
        return QualType{};
    }

    BuiltinType::Kind kind = builtinType->GetKind();
    BuiltinType::Kind promotedKind = promoteIntegerKind(kind);
    if (kind == promotedKind) {
        return qualType;
    }

    Type* promotedType = m_Program.CreateType<BuiltinType>(promotedKind);
    return QualType{promotedType};
}

BuiltinType::Kind SemanticAstVisitor::promoteIntegerKind(BuiltinType::Kind kind) {
    switch (kind) {
        case BuiltinType::Kind::kChar:
        case BuiltinType::Kind::kUChar:
        case BuiltinType::Kind::kShort:
        case BuiltinType::Kind::kUShort:
        case BuiltinType::Kind::kInt:
            return BuiltinType::Kind::kInt;
        case BuiltinType::Kind::kUInt:
        case BuiltinType::Kind::kLong:
        case BuiltinType::Kind::kULong:
            return kind;
        default:
            return BuiltinType::Kind::kNone;
    }
}

QualType SemanticAstVisitor::getCommonRealType(QualType leftQualType,
                            QualType rightQualType) {
    Type* leftType = leftQualType.GetSubType();
    auto* leftBuiltinType = dynamic_cast<BuiltinType*>(leftType);
    if (!leftBuiltinType) {
        return QualType{};
    }

    Type* rightType = rightQualType.GetSubType();
    auto* rightBuiltinType = dynamic_cast<BuiltinType*>(rightType);
    if (!rightBuiltinType) {
        return QualType{};
    }

    BuiltinType::Kind leftKind = leftBuiltinType->GetKind();
    BuiltinType::Kind rightKind = rightBuiltinType->GetKind();
    BuiltinType::Kind promotedKind = getCommonRealTypeKind(leftKind, rightKind);
    if (leftKind == promotedKind) {
        return leftQualType;
    }
    if (rightKind == promotedKind) {
        return rightQualType;
    }

    Type* promotedType = m_Program.CreateType<BuiltinType>(promotedKind);
    return QualType{promotedType};
}

BuiltinType::Kind SemanticAstVisitor::getCommonRealTypeKind(BuiltinType::Kind leftKind,
                                                            BuiltinType::Kind rightKind) {
    if (leftKind == BuiltinType::Kind::kLongDouble ||
            rightKind == BuiltinType::Kind::kLongDouble) {
        return BuiltinType::Kind::kLongDouble;
    }

    if (leftKind == BuiltinType::Kind::kDouble ||
            rightKind == BuiltinType::Kind::kDouble) {
        return BuiltinType::Kind::kDouble;
    }

    if (leftKind == BuiltinType::Kind::kFloat ||
            rightKind == BuiltinType::Kind::kFloat) {
        return BuiltinType::Kind::kFloat;
    }

    // NB: Promoted types are expected
    if (leftKind == rightKind) {
        return leftKind;
    }

    // TODO: Explicitly set the ranks
    if (leftKind == BuiltinType::Kind::kLong &&
            (rightKind == BuiltinType::Kind::kInt || 
                rightKind == BuiltinType::Kind::kUInt)) {
        return BuiltinType::Kind::kLong;
    }

    if (leftKind == BuiltinType::Kind::kULong &&
            (rightKind == BuiltinType::Kind::kInt ||
                rightKind == BuiltinType::Kind::kUInt)) {
        return BuiltinType::Kind::kULong;
    }

    return getCommonRealTypeKind(rightKind, leftKind);
}

bool SemanticAstVisitor::isIncompleteType(QualType qualType) {
    if (isVoidType(qualType)) {
        return true;
    }

    Type* type = qualType.GetSubType();

    if (auto* arrayType = dynamic_cast<ArrayType*>(type)) {
        if (arrayType->HasSize()) {
            return false;
        }
        return true;
    }

    if (auto* tagType = dynamic_cast<TagType*>(type)) {
        TagDeclaration* actualDecl = nullptr;
        if (auto* recordType = dynamic_cast<RecordType*>(tagType)) {
            actualDecl = recordType->GetDeclaration();
        } else {
            auto* enumType = dynamic_cast<EnumType*>(tagType);
            actualDecl = enumType->GetDeclaration();
        }

        if (actualDecl->IsDefinition()) {
            return false;
        }
        return true;
    }

    return false;
}

bool SemanticAstVisitor::isPointerType(QualType qualType) {
    Type* type = qualType.GetSubType();
    return dynamic_cast<PointerType*>(type);
}

bool SemanticAstVisitor::isPointerToFunctionType(QualType qualType) {
    if (!isPointerType(qualType)) {
        return false;
    }
    auto* ptrType = static_cast<PointerType*>(qualType.GetSubType());
    QualType subQualType = ptrType->GetSubType();
    return dynamic_cast<FunctionType*>(subQualType.GetSubType());
}

bool SemanticAstVisitor::isPointerToIncompleteType(QualType qualType) {
    if (!isPointerType(qualType)) {
        return false;
    }
    auto* ptrType = static_cast<PointerType*>(qualType.GetSubType());
    return isIncompleteType(ptrType->GetSubType());
}

bool SemanticAstVisitor::isVoidType(QualType qualType) {
    Type* type = qualType.GetSubType();
    if (auto* builtinType = dynamic_cast<BuiltinType*>(type)) {
        return builtinType->IsVoid();
    }
    return false;
}

bool SemanticAstVisitor::isPointerToVoidType(QualType qualType) {
    if (!isPointerType(qualType)) {
        return false;
    }
    auto* ptrType = static_cast<PointerType*>(qualType.GetSubType());
    return isVoidType(ptrType->GetSubType());
}

bool SemanticAstVisitor::isNullPointerConstant(Expression* expr) {
    if (auto* intExpr = dynamic_cast<IntExpression*>(expr)) {
        IntValue value = intExpr->GetIntValue();
        if (value.GetSignedValue() == 0) {
            return true;
        }
        return false;
    }

    // TODO: Handle cast to void
    return false;
}

bool SemanticAstVisitor::isIntegerType(QualType qualType) {
    Type* type = qualType.GetSubType();
    if (auto* builtinType = dynamic_cast<BuiltinType*>(type)) {
        return builtinType->IsInteger();
    }
    return false;
}

bool SemanticAstVisitor::isFloatType(QualType qualType) {
    Type* type = qualType.GetSubType();
    if (auto* builtinType = dynamic_cast<BuiltinType*>(type)) {
        return builtinType->IsFloat();
    }
    return false;
}

bool SemanticAstVisitor::isRecordType(QualType qualType) {
    Type* type = qualType.GetSubType();
    return dynamic_cast<RecordType*>(type);
}

bool SemanticAstVisitor::isRealType(QualType qualType) {
    Type* type = qualType.GetSubType();
    if (auto* builtinType = dynamic_cast<BuiltinType*>(type)) {
        return !builtinType->IsVoid();
    }
    return false;
}

bool SemanticAstVisitor::isScalarType(QualType qualType) {
    return isRealType(qualType) || isPointerType(qualType);
}

bool SemanticAstVisitor::isModifiableLValue(Expression& expr) {
    if (expr.IsRValue()) {
        return false;
    }

    QualType exprQualType = expr.GetType();
    if (exprQualType.IsConst()) {
        return false;
    }

    Type* exprType = exprQualType.GetSubType();
    if (dynamic_cast<ArrayType*>(exprType)) {
        return false;
    }

    if (auto* recordType = dynamic_cast<RecordType*>(exprType)) {
        // TODO: Check const members recursively
    }

    return true;
}

void SemanticAstVisitor::handleTagDeclaration(TagDeclaration* decl, const std::string& name,
                                              TagDeclaration* scopeDecl) {
    if (decl->IsDefinition()) {
        if (scopeDecl->IsDefinition()) {
            printSemanticError(std::format("redefinition of '{}'", name),
                                decl->GetLocation());
        } else {
            TagDeclaration* currentDecl = decl;
            while (!currentDecl->IsFirstDeclaration()) {
                TagDeclaration* prevDecl = currentDecl->GetPreviousDeclaration();
                QualType declQualType = prevDecl->GetType();
                Type* declType = declQualType.GetSubType();

                if (decl->IsEnum()) {
                    auto* declEnumType = dynamic_cast<EnumType*>(declType);
                    declEnumType->SetDeclaration(static_cast<EnumDeclaration*>(decl));
                } else {
                    auto* declRecordType = dynamic_cast<RecordType*>(declType);
                    declRecordType->SetDeclaration(static_cast<RecordDeclaration*>(decl));
                }

                currentDecl = prevDecl;
            }
            m_CurrentScope->UpdateSymbol(Scope::NamespaceType::Tag, name, decl);
        }
    } else {
        if (scopeDecl->IsDefinition()) {
            QualType declQualType = decl->GetType();
            Type* declType = declQualType.GetSubType();

            if (decl->IsEnum()) {
                auto* declEnumType = dynamic_cast<EnumType*>(declType);
                declEnumType->SetDeclaration(static_cast<EnumDeclaration*>(scopeDecl));
            } else {
                auto* declRecordType = dynamic_cast<RecordType*>(declType);
                declRecordType->SetDeclaration(static_cast<RecordDeclaration*>(scopeDecl));
            }
        } else {
            decl->SetPreviousDeclaration(scopeDecl);
            m_CurrentScope->UpdateSymbol(Scope::NamespaceType::Tag, name, decl);
        }
    }
}

}  // namespace ast
