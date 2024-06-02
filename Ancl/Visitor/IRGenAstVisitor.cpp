#include <Ancl/Visitor/IRGenAstVisitor.hpp>

#include <Ancl/DataLayout/Alignment.hpp>
#include <Ancl/Logger/Logger.hpp>


namespace ast {

IRGenAstVisitor::IRGenAstVisitor(ir::IRProgram& irProgram)
    : m_IRProgram(irProgram),
      m_Constexpr(irProgram) {}

void IRGenAstVisitor::Run(const ASTProgram& astProgram) {
    Visit(*astProgram.GetTranslationUnit());
}


/*
=================================================================
                            Declaration
=================================================================
*/

void IRGenAstVisitor::Visit(FunctionDeclaration& funcDecl) {
    ir::Type* irType = VisitQualType(funcDecl.GetType());
    auto* funcIRType = dynamic_cast<ir::FunctionType*>(irType);
    assert(funcIRType);

    // TODO: Handle small struct decomposition
    // TODO: Handle big struct as return value (add ptr parameter + void return)

    StorageClass storageClass = funcDecl.GetStorageClass();
    auto linkage = ir::GlobalValue::LinkageType::kExtern;
    if (storageClass == StorageClass::kStatic) {
        linkage = ir::GlobalValue::LinkageType::kStatic;
    }

    auto* functionValue = m_IRProgram.CreateValue<ir::Function>(
        funcIRType, linkage, funcDecl.GetName()
    );
    m_CurrentFunction = functionValue;

    if (!funcDecl.IsDefinition()) {
        functionValue->SetDeclaration();
    } else {
        m_CurrentBB = createBasicBlock(funcDecl.GetName());
    }
    m_IRProgram.AddFunction(functionValue);

    for (ParameterDeclaration* param : funcDecl.GetParams()) {
        param->Accept(*this);
    }

    if (funcDecl.IsVariadic()) {
        funcIRType->SetVariadic();
    }

    Statement* body = funcDecl.GetBody();
    if (body) {
        body->Accept(*this);

        generateAllocas();

        if (m_ReturnBlocks.empty()) {
            ir::BasicBlock* lastBlock = m_CurrentFunction->GetLastBlock();
            auto* returnInstr = m_IRProgram.CreateValue<ir::ReturnInstruction>(lastBlock);
            lastBlock->AddInstruction(returnInstr);
        } else if (m_ReturnBlocks.size() == 1) {
            ir::BasicBlock* retBlock = m_ReturnBlocks[0];

            size_t retBlockIdx = 0;
            for (ir::BasicBlock* block : m_CurrentFunction->GetBasicBlocks()) {
                if (block == retBlock) {
                    break;
                }
                ++retBlockIdx;
            }

            m_CurrentFunction->SetLastBlock(retBlockIdx);
        } else {
            ir::AllocaInstruction* retValueAlloca = nullptr;
            if (!dynamic_cast<ir::VoidType*>(funcIRType->GetReturnType())) {
                ir::BasicBlock* entryBlock = m_CurrentFunction->GetEntryBlock();
                retValueAlloca = m_IRProgram.CreateValue<ir::AllocaInstruction>(
                                        funcIRType->GetReturnType(), "retval", entryBlock);
                entryBlock->AddInstructionToBegin(retValueAlloca);
            }

            ir::BasicBlock* newRetBlock = createBasicBlock("return");
            if (retValueAlloca) {
                auto* allocaPtrType = dynamic_cast<ir::PointerType*>(retValueAlloca->GetType());
                auto* load = m_IRProgram.CreateValue<ir::LoadInstruction>(
                                    retValueAlloca, allocaPtrType->GetSubType(), "", newRetBlock);
                auto* retInstr = m_IRProgram.CreateValue<ir::ReturnInstruction>(load, newRetBlock);

                newRetBlock->AddInstruction(load);
                newRetBlock->AddInstruction(retInstr);
            } else {
                auto* retInstr = m_IRProgram.CreateValue<ir::ReturnInstruction>(newRetBlock);
                newRetBlock->AddInstruction(retInstr);
            }

            for (ir::BasicBlock* retBlock : m_ReturnBlocks) {
                auto* retInstr = dynamic_cast<ir::ReturnInstruction*>(retBlock->GetTerminator());
                if (retValueAlloca) {
                    auto* store = m_IRProgram.CreateValue<ir::StoreInstruction>(
                                        retInstr->GetReturnValue(), retValueAlloca, "", retBlock);
                    retBlock->InsertInstructionBeforeTerminator(store);
                }

                auto* branch = m_IRProgram.CreateValue<ir::BranchInstruction>(newRetBlock, retBlock);
                retBlock->ReplaceTerminator(branch);
            }
        }

        ir::BasicBlock* retBlock = m_CurrentFunction->GetLastBlock();
        for (ir::BasicBlock* block : m_CurrentFunction->GetBasicBlocks()) {
            if (!block->IsTerminated()) {
                auto* branch = m_IRProgram.CreateValue<ir::BranchInstruction>(retBlock, block);
                block->AddInstruction(branch);
            }
        }

        resetFunctionData();
    }
}

void IRGenAstVisitor::Visit(LabelDeclaration& labelDecl) {
    std::string labelName = labelDecl.GetName();

    ir::BasicBlock* nextBB = nullptr;
    if (m_FunBBMap.contains(labelName)) {
        nextBB = m_FunBBMap[labelName];
    } else {
        nextBB = createBasicBlock(labelName);
    }

    if (!m_CurrentBB->IsTerminated()) {
        auto* branch = m_IRProgram.CreateValue<ir::BranchInstruction>(nextBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(branch);
    }

    m_CurrentBB = nextBB;
    Statement* labelStatement = labelDecl.GetStatement();
    labelStatement->Accept(*this);
}

void IRGenAstVisitor::Visit(ParameterDeclaration& paramDecl) {
    // TODO: Handle implicit parameter (struct pointer)
    // TODO: Handle struct parameter decomposition

    ir::Type* paramIRType = VisitQualType(paramDecl.GetType());
    std::string name = paramDecl.GetName();

    bool isVolatileParam = m_IsVolatileType;

    auto* paramValue = m_IRProgram.CreateValue<ir::Parameter>(
            name, paramIRType, m_CurrentFunction);

    m_CurrentFunction->AddParameter(paramValue);

    if (m_CurrentFunction->IsDeclaration()) {
        return;
    }

    std::string mangledName = std::format("{}.addr", name);

    auto* alloca = m_IRProgram.CreateValue<ir::AllocaInstruction>(
            paramIRType, mangledName, m_CurrentBB);
    m_AllocasMap[&paramDecl] = alloca;
    m_AllocaBuffer.push(alloca);

    createStoreInstruction(paramValue, alloca, isVolatileParam);
}

void IRGenAstVisitor::Visit(TranslationUnit& unit) {
    for (Declaration* decl : unit.GetDeclarations()) {
        decl->Accept(*this);
    }
}

void IRGenAstVisitor::Visit(VariableDeclaration& varDecl) {
    ir::Type* varIRType = VisitQualType(varDecl.GetType());  // -> m_IsConstVar

    std::string name = varDecl.GetName();
    StorageClass storageClass = varDecl.GetStorageClass();

    bool isVolatileVar = m_IsVolatileType;

    Expression* init = varDecl.GetInit();

    ir::GlobalVariable* globalVar = nullptr;
    if (varDecl.IsGlobal()) {
        auto linkage = ir::GlobalValue::LinkageType::kExtern;
        if (storageClass == StorageClass::kStatic) {
            linkage = ir::GlobalValue::LinkageType::kStatic;
        }
        auto* ptrType = ir::PointerType::Create(varIRType);
        globalVar = m_IRProgram.CreateValue<ir::GlobalVariable>(
            ptrType, linkage, name
        );
        globalVar->SetConst(m_IsConstVar);
        m_IRProgram.AddGlobalVar(globalVar);
    } else if (storageClass == StorageClass::kStatic) {
        std::string mangledName = std::format("{}.{}", m_CurrentFunction->GetName(), name);
        auto* ptrType = ir::PointerType::Create(varIRType);
        globalVar = m_IRProgram.CreateValue<ir::GlobalVariable>(
            ptrType, ir::GlobalValue::LinkageType::kStatic, mangledName
        );
        globalVar->SetConst(m_IsConstVar);
        m_IRProgram.AddGlobalVar(globalVar);     
    } else {  // Local variable
        auto* alloca = m_IRProgram.CreateValue<ir::AllocaInstruction>(
            varIRType, name, m_CurrentBB
        );
        m_AllocasMap[&varDecl] = alloca;
        m_AllocaBuffer.push(alloca);

        if (!init) {
            return;
        }

        if (auto* structType = dynamic_cast<ir::StructType*>(varIRType)) {
            // TODO: memcpy for init list and init struct (memset for zero init)
            ANCL_CRITICAL("Struct/Union initialization is not implemented :(");
            throw std::runtime_error("Not implemented error");
            // exit(EXIT_FAILURE);
        } else if (auto* arrayType = dynamic_cast<ir::ArrayType*>(varIRType)) {
            if (auto* stringExpr = dynamic_cast<StringExpression*>(init)) {
                std::string labelName = std::format(".L__const.{}.{}",
                                                    m_CurrentFunction->GetName(), name);

                auto* ptrType = ir::PointerType::Create(varIRType);
                globalVar = m_IRProgram.CreateValue<ir::GlobalVariable>(
                                        ptrType, ir::GlobalValue::LinkageType::kStatic, labelName);

                std::string initString = stringExpr->GetStringValue();
                globalVar->SetInitString(initString);
                globalVar->SetConst(true);

                m_IRProgram.AddGlobalVar(globalVar);

                createMemoryCopyInstruction(alloca, globalVar, initString.size() + 1);
            } else {
                // TODO: memcpy for init list
                ANCL_CRITICAL("Initializer lists are not implemented :(");
                exit(EXIT_FAILURE);
            }
        } else {
            ir::Value* initValue = Accept(*init);
            createStoreInstruction(initValue, alloca, isVolatileVar);
        }

        // if (auto* stringExpr = dynamic_cast<StringExpression*>(init)) {
        //     // TODO: memcpy for array
        //     // TODO: member expr + store for pointer
        // } else if (auto* initList = dynamic_cast<InitializerList*>(init)) {
        //     std::vector<ir::Value*> initValues = AcceptInitList(*initList);
        //     // TODO: memcpy
        // } else {
        //     ir::Value* initValue = Accept(*init);
        //     ir::Type* initType = initValue->GetType();
        //     if (auto* pointerType = dynamic_cast<ir::PointerType*>(initType)) {
        //         ir::Type* subType = pointerType->GetSubType();
        //         if (auto* arrayType = dynamic_cast<ir::ArrayType*>(subType)) {
        //             // TODO: memcpy
        //             return;
        //         } else if (auto* structType = dynamic_cast<ir::StructType*>(subType)) {
        //             // TODO: memcpy
        //             return;
        //         }
        //     }

        //     createStoreInstruction(initValue, alloca, isVolatileVar);
        // }

        return;
    }


    // Global variable init

    if (!init) {
        return;
    }

    if (auto* stringExpr = dynamic_cast<StringExpression*>(init)) {
        if (dynamic_cast<ir::ArrayType*>(varIRType)) {
            globalVar->SetInitString(stringExpr->GetStringValue());
        } else {  // Pointer type
            auto* stringLabel = dynamic_cast<ir::GlobalVariable*>(Accept(*init));
            globalVar->SetInitVariable(stringLabel);
        }
    } else if (auto* initList = dynamic_cast<InitializerList*>(init)) {
        std::vector<ir::Constant*> initValues = AcceptConst(*initList);
        globalVar->SetInitList(initValues);
    } else {
        ir::Value* initValue = Accept(*init);
        auto* initConstant = dynamic_cast<ir::Constant*>(initValue);
        assert(initConstant);
        globalVar->SetInit(initConstant);
    }
}


/*
=================================================================
                            Statement
=================================================================
*/

void IRGenAstVisitor::Visit(CaseStatement& caseStmt) {
    // TODO: ...
    ANCL_CRITICAL("IRGen: Switch case is not implemented :(");
}

void IRGenAstVisitor::Visit(CompoundStatement& compoundStmt) {
    for (Statement* stmt : compoundStmt.GetBody()) {
        stmt->Accept(*this);
    }
}

void IRGenAstVisitor::Visit(DeclStatement& declStmt) {
    for (Declaration* decl : declStmt.GetDeclarations()) {
        decl->Accept(*this);
    } 
}

void IRGenAstVisitor::Visit(DefaultStatement& defaultStmt) {
    // TODO: ...
    ANCL_CRITICAL("IRGen: Switch case is not implemented :(");
}

void IRGenAstVisitor::Visit(DoStatement& doStmt) {
    ir::BasicBlock* bodyBB = createBasicBlock("do.body");
    ir::BasicBlock* condBB = createBasicBlock("do.cond");
    ir::BasicBlock* endBB = createBasicBlock("do.end");

    // Entry
    auto* bodyBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(bodyBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(bodyBranch);

    // Body
    m_CurrentBB = bodyBB;
    m_ContinueBBStack.push(condBB);
    m_BreakBBStack.push(endBB);
    Statement* body = doStmt.GetBody();
    body->Accept(*this);
    m_BreakBBStack.pop();
    m_ContinueBBStack.pop();

    auto* loopBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(loopBranch);

    // Condition
    // TODO: handle const condition
    m_CurrentBB = condBB;

    Expression* condExpr = doStmt.GetCondition();
    ir::Value* condValue = Accept(*condExpr);
    auto* compareInstr = dynamic_cast<ir::CompareInstruction*>(condValue);
    if (!compareInstr) {
        QualType exprQualType = condExpr->GetType();
        Type* exprType = exprQualType.GetSubType();
        condValue = generateCompareZeroInstruction(ast::BinaryExpression::OpType::kNEqual,
                                                    condValue, exprType);
    }

    auto* condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, bodyBB, endBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(condBranch);

    m_CurrentBB = endBB;
}

void IRGenAstVisitor::Visit(ForStatement& forStmt) {
    ir::BasicBlock* condBB = createBasicBlock("for.cond");

    Expression* forCondition = forStmt.GetCondition();
    ir::BasicBlock* bodyBB = nullptr;
    if (forCondition) {
        bodyBB = createBasicBlock("for.body");
    }

    Expression* stepExpr = forStmt.GetStep();
    ir::BasicBlock* stepBB = nullptr;
    if (stepExpr) {
        stepBB = createBasicBlock("for.step");
    }
    
    ir::BasicBlock* endBB = createBasicBlock("for.end");

    // Init
    if (forStmt.HasInit()) {
        Statement* initStmt = forStmt.GetInit();
        initStmt->Accept(*this);
    }

    auto* entryBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(entryBranch);

    // Condition
    m_CurrentBB = condBB;
    if (forCondition) {
        ir::Value* condValue = Accept(*forCondition);

        auto* compareInstr = dynamic_cast<ir::CompareInstruction*>(condValue);
        if (!compareInstr) {
            QualType exprQualType = forCondition->GetType();
            Type* exprType = exprQualType.GetSubType();
            condValue = generateCompareZeroInstruction(ast::BinaryExpression::OpType::kNEqual,
                                                        condValue, exprType);
        }

        auto* condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(
                    condValue, bodyBB, endBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(condBranch);
        m_CurrentBB = bodyBB;
    }

    // Body
    if (stepBB) {
        m_ContinueBBStack.push(stepBB);
    } else {
        m_ContinueBBStack.push(condBB);
    }
    m_BreakBBStack.push(endBB);
    Statement* bodyStmt = forStmt.GetBody();
    bodyStmt->Accept(*this);
    m_BreakBBStack.pop();
    m_ContinueBBStack.pop();

    if (stepBB) {
        auto* stepBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(stepBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(stepBranch);

        // Step
        m_CurrentBB = stepBB;
        stepExpr->Accept(*this);  // Ignore result value
    }
    auto* condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(condBranch);

    m_CurrentBB = endBB;
}

void IRGenAstVisitor::Visit(GotoStatement& gotoStmt) {
    LabelDeclaration* labelDecl = gotoStmt.GetLabel();
    std::string labelName = labelDecl->GetName();

    ir::BasicBlock* labelBB = nullptr;
    if (m_FunBBMap.contains(labelName)) {
        labelBB = m_FunBBMap[labelName];
    } else {
        labelBB = createBasicBlock(labelName);
    }

    auto* branch = m_IRProgram.CreateValue<ir::BranchInstruction>(labelBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(branch);
}

void IRGenAstVisitor::Visit(IfStatement& ifStmt) {
    ir::BasicBlock* thenBB = createBasicBlock("if.then");

    Statement* elseStmt = ifStmt.GetElse();
    ir::BasicBlock* elseBB = nullptr;
    if (elseStmt) {
        elseBB = createBasicBlock("if.else");
    }

    ir::BasicBlock* endBB = createBasicBlock("if.end");

    // Condition
    // TODO: handle const condition
    Expression* condExpr = ifStmt.GetCondition();
    ir::Value* condValue = Accept(*condExpr);
    auto* compareInstr = dynamic_cast<ir::CompareInstruction*>(condValue);
    if (!compareInstr) {
        QualType exprQualType = condExpr->GetType();
        Type* exprType = exprQualType.GetSubType();
        condValue = generateCompareZeroInstruction(BinaryExpression::OpType::kNEqual,
                                                    condValue, exprType);
    }

    ir::BranchInstruction* condBranch = nullptr;
    if (elseBB) {
        condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, thenBB, elseBB, m_CurrentBB);
    } else {
        condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, thenBB, endBB, m_CurrentBB);
    }
    m_CurrentBB->AddInstruction(condBranch);

    // Then
    m_CurrentBB = thenBB;

    Statement* thenStmt = ifStmt.GetThen();
    thenStmt->Accept(*this);

    auto* endBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(endBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(endBranch);

    // Else
    if (elseBB) {
        m_CurrentBB = elseBB;

        elseStmt->Accept(*this);

        auto* endBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(endBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(endBranch);
    }

    m_CurrentBB = endBB;
}

void IRGenAstVisitor::Visit(LabelStatement& labelStmt) {
    LabelDeclaration* labelDecl = labelStmt.GetLabel();
    labelDecl->Accept(*this);
}

void IRGenAstVisitor::Visit(LoopJumpStatement& loopJmpStmt) {
    ir::BasicBlock* toBasicBlock = nullptr;

    LoopJumpStatement::Type jmpType = loopJmpStmt.GetType();
    if (jmpType == LoopJumpStatement::Type::kContinue) {
        toBasicBlock = m_ContinueBBStack.top();
    } else {  // Break
        toBasicBlock = m_BreakBBStack.top();
    }

    auto* branchInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
        toBasicBlock, m_CurrentBB
    );
    m_CurrentBB->AddInstruction(branchInstr);
}

void IRGenAstVisitor::Visit(ReturnStatement& returnStmt) {
    ir::Value* returnValue = nullptr;
    if (returnStmt.HasReturnExpression()) {
        returnValue = Accept(*returnStmt.GetReturnExpression());
    }

    // TODO: handle struct value

    m_ReturnBlocks.push_back(m_CurrentBB);

    auto* returnInstr = m_IRProgram.CreateValue<ir::ReturnInstruction>(
        returnValue, m_CurrentBB
    );
    m_CurrentBB->AddInstruction(returnInstr);
}

void IRGenAstVisitor::Visit(SwitchStatement& switchStmt) {
    // TODO: ...
}

void IRGenAstVisitor::Visit(WhileStatement& whileStmt) {
    ir::BasicBlock* condBB = createBasicBlock("while.cond");
    ir::BasicBlock* bodyBB = createBasicBlock("while.body");
    ir::BasicBlock* endBB = createBasicBlock("while.end");

    // Entry
    auto* entryBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(entryBranch);

    // Condition
    // TODO: handle const condition (+ endless loop)
    m_CurrentBB = condBB;

    Expression* condExpr = whileStmt.GetCondition();
    ir::Value* condValue = Accept(*condExpr);
    auto* compareInstr = dynamic_cast<ir::CompareInstruction*>(condValue);
    if (!compareInstr) {
        QualType exprQualType = condExpr->GetType();
        Type* exprType = exprQualType.GetSubType();
        condValue = generateCompareZeroInstruction(ast::BinaryExpression::OpType::kNEqual,
                                                    condValue, exprType);
    }

    auto* condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, bodyBB, endBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(condBranch);

    // Body
    m_CurrentBB = bodyBB;
    m_ContinueBBStack.push(condBB);
    m_BreakBBStack.push(endBB);
    Statement* body = whileStmt.GetBody();
    body->Accept(*this);
    m_BreakBBStack.pop();
    m_ContinueBBStack.pop();

    auto* loopBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(loopBranch);

    m_CurrentBB = endBB;
}


/*
=================================================================
                            Expression
=================================================================
*/

void IRGenAstVisitor::Visit(BinaryExpression& binaryExpr) {
    BinaryExpression::OpType opType = binaryExpr.GetOpType();

    Expression* leftOperand = binaryExpr.GetLeftOperand();
    QualType leftQualType = leftOperand->GetType();
    Type* leftType = leftQualType.GetSubType();

    Expression* rightOperand = binaryExpr.GetRightOperand();
    QualType rightQualType = rightOperand->GetType();
    Type* rightType = rightQualType.GetSubType();

    QualType resultQualType = binaryExpr.GetType();
    Type* resultType = resultQualType.GetSubType();

    if (opType == BinaryExpression::OpType::kLogAnd ||
            opType == BinaryExpression::OpType::kLogOr) {
        m_IRValue = generateLogInstruction(opType, leftOperand, rightOperand,
                                            leftType, rightType);
        return;
    }

    switch (opType) {
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
            m_IRValue = generateCompoundAssignment(opType, leftOperand, rightOperand, resultType);
            return;
        default:
            break;
    }

    ir::Value* leftValue = Accept(*leftOperand);

    if (opType == BinaryExpression::OpType::kDirectMember ||
            opType == BinaryExpression::OpType::kArrowMember) {
        auto* declRefExpr = dynamic_cast<ast::DeclRefExpression*>(rightOperand);
        assert(declRefExpr && "Must be DeclRefExpr");
        m_IRValue = generateStructMemberExpression(opType, leftValue, declRefExpr, leftQualType.GetSubType());
        return;
    }

    ir::Value* rightValue = Accept(*rightOperand);

    switch (opType) {
        case BinaryExpression::OpType::kMul:
            m_IRValue = createMulInstruction(leftValue, rightValue, resultType);
            return;
        case BinaryExpression::OpType::kDiv:
            m_IRValue = createDivInstruction(leftValue, rightValue, resultType);
            return;
        case BinaryExpression::OpType::kRem:
            m_IRValue = createRemInstruction(leftValue, rightValue, resultType);
            return;   
        case BinaryExpression::OpType::kAdd:
        case BinaryExpression::OpType::kSub:
            m_IRValue = generateAdditiveExpression(opType, leftValue, rightValue,
                                                    leftType, rightType, resultType);
            return;
        case BinaryExpression::OpType::kShiftL:
            m_IRValue = createShiftLInstruction(leftValue, rightValue);
            return;
        case BinaryExpression::OpType::kShiftR:
            m_IRValue = createShiftRInstruction(leftValue, rightValue, resultType);
            return;

        case BinaryExpression::OpType::kAnd:
            m_IRValue = createAndInstruction(leftValue, rightValue);
            return;
        case BinaryExpression::OpType::kXor:
            m_IRValue = createXorInstruction(leftValue, rightValue);
            return;
        case BinaryExpression::OpType::kOr:
            m_IRValue = createOrInstruction(leftValue, rightValue);
            return;

        case BinaryExpression::OpType::kLess:
        case BinaryExpression::OpType::kGreater:
        case BinaryExpression::OpType::kLessEq:
        case BinaryExpression::OpType::kGreaterEq:
        case BinaryExpression::OpType::kEqual:
        case BinaryExpression::OpType::kNEqual:
            m_IRValue = createCompareInstruction(opType, leftValue, rightValue, leftType);
            return;

        case BinaryExpression::OpType::kAssign:
            m_IRValue = createStoreInstruction(rightValue, leftValue, leftQualType.IsVolatile());
            return;

        case BinaryExpression::OpType::kArrSubscript:
            m_IRValue = generateArrMemberExpression(leftValue, rightValue, rightType);
            return;
    }

    return;
}

void IRGenAstVisitor::Visit(CallExpression& callExpr) {
    Expression* calleeExpr = callExpr.GetCallee();
    ir::Value* calleeValue = Accept(*calleeExpr);
    auto* functionValue = dynamic_cast<ir::Function*>(calleeValue);
    assert(functionValue);

    std::vector<Expression*> argExprs = callExpr.GetArguments();
    std::vector<ir::Value*> argValues;
    argValues.reserve(argExprs.size());
    for (Expression* argExpr : argExprs) {
        argValues.push_back(Accept(*argExpr));
    }

    // TODO: handle struct arguments and struct return
    auto* callInstr = m_IRProgram.CreateValue<ir::CallInstruction>(
                                functionValue, argValues, "call", m_CurrentBB);
    m_CurrentBB->AddInstruction(callInstr);
    m_IRValue = callInstr;
}

void IRGenAstVisitor::Visit(CastExpression& castExpr) {
    Expression* subExpr = castExpr.GetSubExpression();
    ir::Value* fromValue = Accept(*subExpr);

    if (castExpr.IsLValueToRValue()) {
        auto* valuePtrType = dynamic_cast<ir::PointerType*>(fromValue->GetType());
        assert(valuePtrType);

        QualType qualType = castExpr.GetType();
        ir::LoadInstruction* loadInstr = createLoadInstruction(fromValue, valuePtrType->GetSubType(),
                                                                qualType.IsVolatile());
        m_IRValue = loadInstr; 
        return;
    }

    QualType toQualType = castExpr.GetToType();
    auto* toBuiltinType = dynamic_cast<BuiltinType*>(toQualType.GetSubType());

    ir::Type* fromType = fromValue->GetType();
    ir::Type* toType = VisitQualType(toQualType);

    if (auto* constValue = getNumberIRConstant(fromValue)) {
        m_IRValue = m_Constexpr.EvaluateCastConstExpr(constValue, toType);
        return;
    }

    bool fromPointer = dynamic_cast<ir::PointerType*>(fromType);
    bool toPointer = dynamic_cast<ir::PointerType*>(toType);

    if (fromPointer && toPointer) {
        m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kBitcast, fromValue, toType);
        return;
    }

    bool fromInt = dynamic_cast<ir::IntType*>(fromType);
    bool toInt = dynamic_cast<ir::IntType*>(toType);

    // TODO: Prohibit this casting between types of different sizes
    if (fromPointer && toInt) {
        m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kPtrToI, fromValue, toType);
        return;
    }
    if (fromInt && toPointer) {
        m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kIToPtr, fromValue, toType);
        return;
    } 

    QualType fromQualType = subExpr->GetType();
    auto* fromBuiltinType = dynamic_cast<BuiltinType*>(fromQualType.GetSubType());

    uint64_t fromSize = ir::Alignment::GetTypeSize(fromType);
    uint64_t toSize = ir::Alignment::GetTypeSize(toType);

    if (fromInt && toInt) {
        if (fromSize < toSize) {
            if (fromBuiltinType->IsSignedInteger()) {
                m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kSExt, fromValue, toType);
            } else {
                m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kZExt, fromValue, toType);
            }
        } else if (fromSize > toSize) {
            m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kITrunc, fromValue, toType);
        }
        return;
    }

    bool fromFloat = dynamic_cast<ir::FloatType*>(fromType);
    bool toFloat = dynamic_cast<ir::FloatType*>(toType);

    if (fromFloat && toFloat) {
        if (fromSize < toSize) {
            m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kFExt, fromValue, toType);
        } else if (fromSize > toSize) {
            m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kFTrunc, fromValue, toType);
        }
        return;
    }   

    if (fromInt && toFloat) {
        if (fromBuiltinType->IsSignedInteger()) {
            m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kSIToF, fromValue, toType);
        } else {
            m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kUIToF, fromValue, toType);
        }
        return;
    } 

    if (fromFloat && toInt) {
        if (toBuiltinType->IsSignedInteger()) {
            m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kFToSI, fromValue, toType);
        } else {
            m_IRValue = createCastInstruction(ir::CastInstruction::OpType::kFToUI, fromValue, toType);
        }
        return;
    }
}

void IRGenAstVisitor::Visit(CharExpression& charExpr) {
    QualType qualType = charExpr.GetType();
    auto* intType = dynamic_cast<ir::IntType*>(VisitQualType(qualType));
    assert(intType);

    IntValue intValue(charExpr.GetCharValue(), /*isSigned=*/true);
    m_IRValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, intValue);
}

void IRGenAstVisitor::Visit(ConditionalExpression& condExpr) {
    // TODO: Handle const

    ir::BasicBlock* trueBB = createBasicBlock("cond.true");
    ir::BasicBlock* falseBB = createBasicBlock("cond.false");
    ir::BasicBlock* endBB = createBasicBlock("cond.end");

    Expression* condition = condExpr.GetCondition();
    QualType conditionQualType = condition->GetType();
    Type* conditionType = conditionQualType.GetSubType();
    ir::Value* condValue = Accept(*condition);

    ir::Instruction* compareInstr = nullptr;
    if (auto* cmpInstr = dynamic_cast<ir::CompareInstruction*>(condValue)) {
        compareInstr = cmpInstr;
    } else {
        compareInstr = generateCompareZeroInstruction(
                            ast::BinaryExpression::OpType::kNEqual,
                            condValue, conditionType);
    }

    auto* branchInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
        compareInstr, trueBB, falseBB, m_CurrentBB
    );

    m_CurrentBB->AddInstruction(branchInstr);

    // True
    m_CurrentBB = trueBB;

    Expression* trueExpr = condExpr.GetTrueExpression();
    ir::Value* trueValue = Accept(*trueExpr);

    auto* result = m_IRProgram.CreateValue<ir::AllocaInstruction>(
                                trueValue->GetType(), "tmp", m_CurrentBB);
    m_AllocaBuffer.push(result);

    ir::StoreInstruction* trueStoreInstr = createStoreInstruction(trueValue, result);

    auto* trueBranchEndInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
                                endBB, m_CurrentBB);

    m_CurrentBB->AddInstruction(trueBranchEndInstr);

    // False
    m_CurrentBB = falseBB;

    Expression* falseExpr = condExpr.GetFalseExpression();
    ir::Value* falseValue = Accept(*falseExpr);

    ir::StoreInstruction* falseStoreInstr = createStoreInstruction(falseValue, result);

    auto* falseBranchEndInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
                                    endBB, m_CurrentBB);

    m_CurrentBB->AddInstruction(falseBranchEndInstr);

    // End
    m_CurrentBB = endBB;

    ir::LoadInstruction* loadInstr = createLoadInstruction(result, trueValue->GetType());

    m_IRValue = loadInstr; 
}

void IRGenAstVisitor::Visit(ConstExpression& constExpr) {
    QualType qualType = constExpr.GetType();

    auto constValue = constExpr.GetValue();
    if (constValue.IsInteger()) {
        auto* intType = dynamic_cast<ir::IntType*>(VisitQualType(qualType));
        assert(intType);

        IntValue intValue = constValue.GetIntValue();
        m_IRValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, intValue);
    } else {
        auto* floatType = dynamic_cast<ir::FloatType*>(VisitQualType(qualType));
        assert(floatType);

        FloatValue floatValue = constValue.GetFloatValue();
        m_IRValue = m_IRProgram.CreateValue<ir::FloatConstant>(floatType, floatValue);
    }
}

void IRGenAstVisitor::Visit(DeclRefExpression& declrefExpr) {
    ValueDeclaration* declaration = declrefExpr.GetDeclaration();
    if (m_AllocasMap.contains(declaration)) {
        m_IRValue = m_AllocasMap.at(declaration);
        return;
    }

    if (auto* enumConstDecl = dynamic_cast<EnumConstDeclaration*>(declaration)) {
        ConstExpression* init = enumConstDecl->GetInit();
        m_IRValue = Accept(*init);
        return;
    }

    std::string name = declaration->GetName();

    if (m_IRProgram.HasFunction(name)) {
        m_IRValue = m_IRProgram.GetFunction(name);
        return;
    }

    if (m_IRProgram.HasGlobalVar(name)) {
        m_IRValue = m_IRProgram.GetGlobalVar(name);
        return;       
    }

    // Static local variable
    std::string mangledName = std::format("{}.{}", m_CurrentFunction->GetName(), name);
    if (m_IRProgram.HasGlobalVar(mangledName)) {
        m_IRValue = m_IRProgram.GetGlobalVar(mangledName);
        return;       
    }

    ANCL_CRITICAL("Unknown name (DeclRefExpr)");
    return;
}

void IRGenAstVisitor::Visit(ExpressionList& exprList) {
    for (Expression* expr : exprList.GetExpressions()) {
        m_IRValue = Accept(*expr);
    }
}

void IRGenAstVisitor::Visit(FloatExpression& floatExpr) {
    QualType qualType = floatExpr.GetType();
    auto* floatType = dynamic_cast<ir::FloatType*>(VisitQualType(qualType));
    assert(floatType);

    FloatValue floatValue = floatExpr.GetFloatValue();
    m_IRValue = m_IRProgram.CreateValue<ir::FloatConstant>(floatType, floatValue);
}

void IRGenAstVisitor::Visit(InitializerList& initList) {
    // TODO: memcpy or loop
    // TODO: -> m_ValueList
}

void IRGenAstVisitor::Visit(IntExpression& intExpr) {
    QualType qualType = intExpr.GetType();
    auto* intType = dynamic_cast<ir::IntType*>(VisitQualType(qualType));
    assert(intType);

    IntValue intValue = intExpr.GetIntValue();
    m_IRValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, intValue);
}

void IRGenAstVisitor::Visit(SizeofTypeExpression& sizeofTypeExpr) {
    ir::Type* type = VisitQualType(sizeofTypeExpr.GetSubType());
    uint64_t size = ir::Alignment::GetTypeSize(type);

    auto* intType = ir::IntType::Create(m_IRProgram, size);
    IntValue intValue(size, /*isSigned=*/false);
    m_IRValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, intValue);
}

void IRGenAstVisitor::Visit(StringExpression& stringExpr) {
    static uint64_t counter = 0;

    std::string stringValue = stringExpr.GetStringValue();
    if (m_StringLabelsMap.contains(stringValue)) {
        m_IRValue = m_StringLabelsMap[stringValue];
        return;
    }

    std::string label = ".L.str";
    if (counter > 0) {
        label = std::format(".L.str.{}", counter);
    }
    ++counter;

    auto* arrayType = ir::ArrayType::Create(
                        ir::IntType::Create(m_IRProgram, ir::Alignment::GetPointerTypeSize()),
                        stringValue.size() + 1);

    auto* strValue = m_IRProgram.CreateValue<ir::GlobalVariable>(
            ir::PointerType::Create(arrayType), ir::GlobalValue::LinkageType::kStatic, label);

    strValue->SetInitString(stringValue);
    strValue->SetConst(true);

    m_IRProgram.AddGlobalVar(strValue);
    m_StringLabelsMap[stringValue] = strValue;
    m_IRValue = strValue;
}

void IRGenAstVisitor::Visit(UnaryExpression& unaryExpr) {
    Expression* operandExpr = unaryExpr.GetOperand();
    ir::Value* operandValue = Accept(*operandExpr);

    QualType exprQualType = unaryExpr.GetType();
    Type* exprType = exprQualType.GetSubType();

    UnaryExpression::OpType opType = unaryExpr.GetOpType();
    switch (opType) {
        case UnaryExpression::OpType::kPreInc:
        case UnaryExpression::OpType::kPreDec:
        case UnaryExpression::OpType::kPostInc:
        case UnaryExpression::OpType::kPostDec:
            m_IRValue = generateIncDecExpression(opType, operandValue, exprQualType);
            return;

        case UnaryExpression::OpType::kAddrOf:
            m_IRValue = operandValue;
            return;
        case UnaryExpression::OpType::kDeref:
            m_IRValue = generateDerefExpression(operandValue);
            return;

        // Ignore Plus
        case UnaryExpression::OpType::kMinus:
            m_IRValue = generateNegExpression(operandValue);
            return;
        case UnaryExpression::OpType::kNot:
            m_IRValue = generateNotExpression(operandValue);
            return;
        case UnaryExpression::OpType::kLogNot:
            m_IRValue = generateLogNotExpression(operandValue, exprType);
            return;

        case UnaryExpression::OpType::kSizeof:
            m_IRValue = generateSizeofExpression(operandValue);
            return;
    }
}


/*
=================================================================
                            Type
=================================================================
*/

void IRGenAstVisitor::Visit(ArrayType& arrayType) {
    ir::Type* elementType = VisitQualType(arrayType.GetSubType());
    IntValue sizeValue = arrayType.GetSize();  // Unsigned
    m_IRType = ir::ArrayType::Create(elementType, sizeValue.GetUnsignedValue());
}

// TODO: Use Frontend Target Machine
void IRGenAstVisitor::Visit(BuiltinType& builtinType) {
    switch (builtinType.GetKind()) {
        case BuiltinType::Kind::kVoid:
            m_IRType = ir::VoidType::Create(m_IRProgram);
            break; 

        case BuiltinType::Kind::kChar:
        case BuiltinType::Kind::kUChar:
            m_IRType = ir::IntType::Create(m_IRProgram, 1);
            break;

        case BuiltinType::Kind::kShort:
        case BuiltinType::Kind::kUShort:
            m_IRType = ir::IntType::Create(m_IRProgram, 2);
            break;

        case BuiltinType::Kind::kInt:
        case BuiltinType::Kind::kUInt:
            m_IRType = ir::IntType::Create(m_IRProgram, 4);
            break;

        case BuiltinType::Kind::kLong:
        case BuiltinType::Kind::kULong:
            m_IRType = ir::IntType::Create(m_IRProgram, 8);
            break;

        case BuiltinType::Kind::kFloat:
            m_IRType = ir::FloatType::Create(m_IRProgram, ir::FloatType::Kind::kFloat);
            break;

        case BuiltinType::Kind::kDouble:  
            m_IRType = ir::FloatType::Create(m_IRProgram, ir::FloatType::Kind::kDouble);
            break;

        case BuiltinType::Kind::kLongDouble:  
            m_IRType = ir::FloatType::Create(m_IRProgram, ir::FloatType::Kind::kLongDouble);
            break;

        default:
            ANCL_CRITICAL("Invalid builtin type");
            break;
    }
}

void IRGenAstVisitor::Visit(EnumType& enumType) {
    m_IRType = ir::IntType::Create(m_IRProgram, 4);
}

void IRGenAstVisitor::Visit(FunctionType& funcType) {
    ir::Type* returnType = VisitQualType(funcType.GetSubType());

    std::vector<ir::Type*> paramIRTypes;
    std::vector<QualType> paramAstTypes = funcType.GetParamTypes();
    paramIRTypes.reserve(paramAstTypes.size());
    for (const QualType& paramAstType : paramAstTypes) {
        paramIRTypes.push_back(VisitQualType(paramAstType));
    }

    m_IRType = ir::FunctionType::Create(returnType, paramIRTypes, funcType.IsVariadic());
}

void IRGenAstVisitor::Visit(PointerType& ptrType) {
    ir::Type* elementType = VisitQualType(ptrType.GetSubType());
    m_IRType = ir::PointerType::Create(elementType);
}

ir::Type* IRGenAstVisitor::VisitQualType(QualType qualType) {
    Type* astType = qualType.GetSubType();
    m_IRType = Accept(*astType);

    m_IsConstVar = qualType.IsConst();
    m_IsVolatileType = qualType.IsVolatile();
    m_IsRestrictType = qualType.IsRestrict();

    return m_IRType;
}

void IRGenAstVisitor::Visit(RecordType& recordType) {
    std::vector<ir::Type*> elementTypes;

    RecordDeclaration* decl = recordType.GetDeclaration();
    if (m_StructTypesMap.contains(decl)) {
        m_IRType = m_StructTypesMap[decl];
        return;
    }

    auto* structIRType = m_IRProgram.CreateType<ir::StructType>(m_IRProgram, elementTypes);
    m_StructTypesMap[decl] = structIRType;

    static uint64_t counter = 0;
    std::string structIRName = decl->GetName();
    if (counter > 0) {
        structIRName += "." + std::to_string(counter);
    }
    ++counter;
    structIRType->SetName(structIRName);

    std::vector<FieldDeclaration*> fields = decl->GetFields();

    elementTypes.reserve(fields.size());
    for (FieldDeclaration* fieldDecl : fields) {
        ir::Type* fieldIRType = VisitQualType(fieldDecl->GetType());
        elementTypes.push_back(fieldIRType);
    }

    if (decl->IsStruct()) {
        structIRType->SetElementTypes(elementTypes);
        m_IRType = structIRType;
        return;
    }

    // Union type
    // TODO: Take it out somewhere

    uint64_t maxSize = 0;
    uint64_t maxAlignment = 0;
    uint64_t maxAlignTypeSize = 0;

    ir::Type* maxAlignType = nullptr;

    for (ir::Type* elemType : elementTypes) {
        uint64_t elemSize = ir::Alignment::GetTypeSize(elemType);
        maxSize = std::max(maxSize, elemSize);

        uint64_t elemAlignment = ir::Alignment::GetTypeAlignment(elemType, /*isStackAlignment=*/false);
        if (elemAlignment > maxAlignment) {
            maxAlignment = elemAlignment;
            maxAlignType = elemType;
            maxAlignTypeSize = elemSize;
        }
    }

    std::vector<ir::Type*> unionElemTypes = {maxAlignType};

    uint64_t unionSize = ir::Alignment::Align(maxSize, maxAlignment);
    if (unionSize > maxAlignTypeSize) {
        ir::IntType* byteType = ir::IntType::Create(m_IRProgram, 1);
        ir::ArrayType* restBytesType = ir::ArrayType::Create(byteType, unionSize - maxAlignTypeSize);
        unionElemTypes.push_back(restBytesType);
    }

    structIRType->SetElementTypes(unionElemTypes);
    m_IRType = structIRType;
}

void IRGenAstVisitor::Visit(TypedefType& typedefType) {
    TypeDeclaration* decl = typedefType.GetDeclaration();
    m_IRType = VisitQualType(decl->GetType());
}

ir::Value* IRGenAstVisitor::createAddInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                                 ast::Type* type) {
    auto* builtinType = dynamic_cast<BuiltinType*>(type);

    auto opType = ir::BinaryInstruction::OpType::kAdd;
    if (builtinType->IsFloat()) {
        opType = ir::BinaryInstruction::OpType::kFAdd;
    }

    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst =  getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst, opType);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                opType, "add", leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::Value* IRGenAstVisitor::createSubInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                                 ast::Type* type) {
    auto* builtinType = dynamic_cast<BuiltinType*>(type);

    auto opType = ir::BinaryInstruction::OpType::kSub;
    if (builtinType->IsFloat()) {
        opType = ir::BinaryInstruction::OpType::kFSub;
    }

    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst =  getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst, opType);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                    opType, "sub", leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

// TODO:!!! dynamic_cast to ptr<ir::type>
ir::Value* IRGenAstVisitor::generateIncDecExpression(UnaryExpression::OpType opType, ir::Value* value,
                                                     QualType qualType) {
    bool isInc = true;
    if (opType == UnaryExpression::OpType::kPreDec ||
            opType == UnaryExpression::OpType::kPostDec) {
        isInc = false;
    }

    auto* valuePtrType = dynamic_cast<ir::PointerType*>(value->GetType());
    assert(valuePtrType);
    ir::Type* valueSubType = valuePtrType->GetSubType();

    bool isPointer = false;
    ir::Value* constValue = nullptr;
    if (auto* intType = dynamic_cast<ir::IntType*>(valueSubType)) {
        int64_t inc = isInc ? 1 : -1;
        constValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(inc));
    } else if (auto* floatType = dynamic_cast<ir::FloatType*>(valueSubType)) {
        double inc = isInc ? 1. : -1.;
        constValue = m_IRProgram.CreateValue<ir::FloatConstant>(floatType, FloatValue(inc));
    } else if (auto* pointerType = dynamic_cast<ir::PointerType*>(valueSubType)) {
        isPointer = true;
        int64_t inc = isInc ? 1 : -1;
        auto* intType = ir::IntType::Create(m_IRProgram, ir::Alignment::GetPointerTypeSize());
        constValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(inc));
    }

    ir::LoadInstruction* loadInstr = createLoadInstruction(value, valueSubType, qualType.IsVolatile());

    ir::Value* resultValue = nullptr;
    if (isPointer) {
        resultValue = generatePtrAddExpression(/*isAdd=*/true, loadInstr, constValue);
    } else {
        resultValue = createAddInstruction(loadInstr, constValue, qualType.GetSubType());
    }

    ir::StoreInstruction* storeInstr = createStoreInstruction(resultValue, value, qualType.IsVolatile());

    if (opType == UnaryExpression::OpType::kPreDec ||
            opType == UnaryExpression::OpType::kPreInc) {
        return resultValue;
    }

    return value;
}

ir::BinaryInstruction* IRGenAstVisitor::generatePtrSubExpression(ir::Value* leftValue, ir::Value* rightValue) {
    auto* ptrType = dynamic_cast<ir::PointerType*>(leftValue->GetType());
    auto* bytesType = ir::IntType::Create(m_IRProgram, ir::Alignment::GetPointerTypeSize());

    auto* leftPTIInstr = m_IRProgram.CreateValue<ir::CastInstruction>(
        ir::CastInstruction::OpType::kPtrToI, "sub.ptr.lhs.cast",
        leftValue, bytesType, m_CurrentBB
    );
    auto* rightPTIInstr = m_IRProgram.CreateValue<ir::CastInstruction>(
        ir::CastInstruction::OpType::kPtrToI, "sub.ptr.rhs.cast",
        rightValue, bytesType, m_CurrentBB
    );
    m_CurrentBB->AddInstruction(leftPTIInstr);
    m_CurrentBB->AddInstruction(rightPTIInstr);

    auto* subInstr = m_IRProgram.CreateValue<ir::BinaryInstruction>(
        ir::BinaryInstruction::OpType::kSub, "sub.ptr.sub",
        leftPTIInstr, rightPTIInstr, m_CurrentBB
    );
    m_CurrentBB->AddInstruction(subInstr);

    size_t subTypeSize = ir::Alignment::GetTypeSize(ptrType->GetSubType());
    auto* sizeIntConstant = m_IRProgram.CreateValue<ir::IntConstant>(
                                bytesType, IntValue(subTypeSize, false));

    auto* divInstr = m_IRProgram.CreateValue<ir::BinaryInstruction>(
        ir::BinaryInstruction::OpType::kSDiv, "sub.ptr.div",
        subInstr, sizeIntConstant, m_CurrentBB
    );
    m_CurrentBB->AddInstruction(divInstr);

    return divInstr;
}

ir::CastInstruction* IRGenAstVisitor::generateExtCast(ir::Value* fromValue,
                                                      ast::Type* fromASTType,
                                                      ir::Type* toIRType) {
    auto* builtinType = static_cast<ast::BuiltinType*>(fromASTType);

    auto opType = ir::CastInstruction::OpType::kNone;
    if (builtinType->IsSignedInteger()) {
        opType = ir::CastInstruction::OpType::kSExt;
    } else if (builtinType->IsUnsignedInteger()) {
        opType = ir::CastInstruction::OpType::kZExt;
    } else {
        opType = ir::CastInstruction::OpType::kFExt;
    }

    auto* extInstr = m_IRProgram.CreateValue<ir::CastInstruction>(
                                opType, "ext", fromValue, toIRType, m_CurrentBB);
    m_CurrentBB->AddInstruction(extInstr);

    return extInstr;
}

ir::Value* IRGenAstVisitor::generateNotExpression(ir::Value* value) {
    auto* intType = ir::IntType::Create(m_IRProgram, 4);

    if (auto* intConstant = dynamic_cast<ir::IntConstant*>(value)) {
        IntValue intValue = intConstant->GetValue();
        IntValue notValue(~intValue.GetSignedValue(), intValue.IsSigned());
        return m_IRProgram.CreateValue<ir::IntConstant>(intType, notValue);
    }

    auto* allOnesValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(-1, false));
    return createXorInstruction(value, allOnesValue);
}

ir::Value* IRGenAstVisitor::generateLogNotExpression(ir::Value* operandValue, ast::Type* exprType) {
    ir::Type* operandType = operandValue->GetType();
    uint64_t typeSize = ir::Alignment::GetTypeSize(operandType);

    if (auto* intConstant = dynamic_cast<ir::IntConstant*>(operandValue)) {
        IntValue intValue = intConstant->GetValue();
        IntValue notValue(!intValue.GetSignedValue(), intValue.IsSigned());
        return m_IRProgram.CreateValue<ir::IntConstant>(ir::IntType::Create(m_IRProgram, typeSize), notValue);
    }
    if (auto* floatConstant = dynamic_cast<ir::FloatConstant*>(operandValue)) {
        FloatValue floatValue = floatConstant->GetValue();
        IntValue notValue(!floatValue.GetValue());
        return m_IRProgram.CreateValue<ir::IntConstant>(ir::IntType::Create(m_IRProgram, typeSize), notValue);
    }

    return generateCompareZeroInstruction(ast::BinaryExpression::OpType::kEqual,
                                          operandValue, exprType);
}

ir::Value* IRGenAstVisitor::generateSizeofExpression(ir::Value* operandValue) {
    uint64_t size = ir::Alignment::GetTypeSize(operandValue->GetType());
    auto* intType = ir::IntType::Create(m_IRProgram, size);
    IntValue intValue(size, /*isSigned=*/false);

    return m_IRProgram.CreateValue<ir::IntConstant>(intType, intValue);
}

ir::Value* IRGenAstVisitor::generateNegExpression(ir::Value* value) {
    ir::Type* valueType = value->GetType();

    if (auto* intConstant = dynamic_cast<ir::IntConstant*>(value)) {
        IntValue intValue = intConstant->GetValue();
        IntValue negValue(-intValue.GetSignedValue(), intValue.IsSigned());
        auto* intType = static_cast<ir::IntType*>(intConstant->GetType());
        return m_IRProgram.CreateValue<ir::IntConstant>(intType, negValue);
    }
    if (auto* floatConstant = dynamic_cast<ir::FloatConstant*>(value)) {
        FloatValue floatValue = floatConstant->GetValue();
        FloatValue negValue(-floatValue.GetValue());
        auto* floatType = static_cast<ir::FloatType*>(floatConstant->GetType());
        return m_IRProgram.CreateValue<ir::FloatConstant>(floatType, negValue);
    }

    auto opType = ir::BinaryInstruction::OpType::kNone;
    ir::Value* zeroValue = nullptr;
    if (auto* intType = dynamic_cast<ir::IntType*>(valueType)) {
        opType = ir::BinaryInstruction::OpType::kSub;
        zeroValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(0, false));
    } else if (auto* floatType = dynamic_cast<ir::FloatType*>(valueType)) {
        opType = ir::BinaryInstruction::OpType::kFSub;
        zeroValue = m_IRProgram.CreateValue<ir::FloatConstant>(floatType, FloatValue(0.));
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                    opType, "neg", zeroValue, value, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::Instruction* IRGenAstVisitor::generateStructMemberExpression(BinaryExpression::OpType opType,
                                                                 ir::Value* structValue,
                                                                 ast::DeclRefExpression* memberExpr,
                                                                 ast::Type* astType) {
    if (opType == BinaryExpression::OpType::kArrowMember) {
        auto* astPtrType = dynamic_cast<ast::PointerType*>(astType);
        QualType recordQualType = astPtrType->GetSubType();
        astType = recordQualType.GetSubType();
    }

    auto* recordType = dynamic_cast<ast::RecordType*>(astType);
    assert(recordType);

    ir::Type* memberIRType = VisitQualType(memberExpr->GetType());
    auto* memberPtrType = ir::PointerType::Create(memberIRType);

    ast::RecordDeclaration* recordDecl = recordType->GetDeclaration();
    if (recordDecl->IsUnion()) {
        return createCastInstruction(ir::CastInstruction::OpType::kBitcast, structValue, memberPtrType);
    }

    ValueDeclaration* memberDecl = memberExpr->GetDeclaration();
    auto* fieldDecl = dynamic_cast<ast::FieldDeclaration*>(memberDecl);
    assert(fieldDecl && "Must be field declaration");

    size_t fieldIndex = fieldDecl->GetPosition();
    auto* intType = ir::IntType::Create(m_IRProgram, 4);
    auto* idxValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(fieldIndex));

    auto* memberInstr = m_IRProgram.CreateValue<ir::MemberInstruction>(
                                structValue, idxValue, "member", memberPtrType, m_CurrentBB);
    memberInstr->SetDeref(true);
    m_CurrentBB->AddInstruction(memberInstr);
    return memberInstr;
}

ir::Value* IRGenAstVisitor::generateCompoundAssignment(BinaryExpression::OpType opType,
                                                       Expression* leftOperand, Expression* rightOperand,
                                                       ast::Type* resultType) {
    ir::Value* rightValue = Accept(*rightOperand);
    ir::Value* leftValue = Accept(*leftOperand);

    auto* valuePtrType = dynamic_cast<ir::PointerType*>(leftValue->GetType());
    assert(valuePtrType);
    ir::Type* valueSubType = valuePtrType->GetSubType();

    QualType leftQualType = leftOperand->GetType();
    ast::Type* leftASTType = leftQualType.GetSubType();

    QualType rightQualType = rightOperand->GetType();
    ast::Type* rightASTType = rightQualType.GetSubType();

    ir::Value* leftLoadValue = createLoadInstruction(leftValue, valueSubType, leftQualType.IsVolatile());

    ir::Type* leftLoadIRType = leftLoadValue->GetType();
    ir::Type* rightIRType = rightValue->GetType();

    // TODO: Encode this cast during semantics pass
    uint64_t leftSize = ir::Alignment::GetTypeSize(leftLoadIRType);
    uint64_t rightSize = ir::Alignment::GetTypeSize(rightIRType);
    if (leftSize < rightSize) {
        auto* builtinLeftType = dynamic_cast<BuiltinType*>(leftASTType);
        if (builtinLeftType->IsSignedInteger()) {
            leftLoadValue = createCastInstruction(ir::CastInstruction::OpType::kSExt, leftLoadValue, rightIRType);
        } else {
            leftLoadValue = createCastInstruction(ir::CastInstruction::OpType::kZExt, leftLoadValue, rightIRType);
        }
    }

    auto additiveOpType = BinaryExpression::OpType::kAdd;
    if (opType == BinaryExpression::OpType::kSubAssign) {
        additiveOpType = BinaryExpression::OpType::kSub;
    }

    ir::Value* result = nullptr;
    switch (opType) {
        case BinaryExpression::OpType::kMulAssign:
            result = createMulInstruction(leftLoadValue, rightValue, resultType);
            break;
        case BinaryExpression::OpType::kDivAssign:
            result = createDivInstruction(leftLoadValue, rightValue, resultType);
            break;
        case BinaryExpression::OpType::kRemAssign:
            result = createRemInstruction(leftLoadValue, rightValue, resultType);
            break;   
        case BinaryExpression::OpType::kAddAssign:
        case BinaryExpression::OpType::kSubAssign:
            result = generateAdditiveExpression(additiveOpType, leftLoadValue, rightValue,
                                                leftASTType, rightASTType, resultType);
            break;
        case BinaryExpression::OpType::kShiftLAssign:
            result = createShiftLInstruction(leftLoadValue, rightValue);
            break;
        case BinaryExpression::OpType::kShiftRAssign:
            result = createShiftRInstruction(leftLoadValue, rightValue, resultType);
            break;

        case BinaryExpression::OpType::kAndAssign:
            result = createAndInstruction(leftLoadValue, rightValue);
            break;
        case BinaryExpression::OpType::kXorAssign:
            result = createXorInstruction(leftLoadValue, rightValue);
            break;
        case BinaryExpression::OpType::kOrAssign:
            result = createOrInstruction(leftLoadValue, rightValue);
            break;
    }

    // TODO: Encode this cast during semantics pass
    if (leftSize < rightSize) {
        result = createCastInstruction(ir::CastInstruction::OpType::kITrunc, result, leftLoadIRType);
    }

    createStoreInstruction(result, leftValue, leftQualType.IsVolatile());

    return result;
}

ir::Value* IRGenAstVisitor::generateDerefExpression(ir::Value* operandValue) {
    auto* arrType = dynamic_cast<ir::ArrayType*>(operandValue->GetType());
    if (!arrType) {
        return operandValue;
    }

    auto* intType = ir::IntType::Create(m_IRProgram, ir::Alignment::GetPointerTypeSize());
    auto* zeroValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(0, /*isSigned=*/false));

    auto* ptrType = ir::PointerType::Create(arrType->GetSubType());
    auto* memberInstr = m_IRProgram.CreateValue<ir::MemberInstruction>(
                                operandValue, zeroValue, "member", ptrType, m_CurrentBB);
    memberInstr->SetDeref(true); 

    m_CurrentBB->AddInstruction(memberInstr);
    return memberInstr;
}

ir::Instruction* IRGenAstVisitor::generateArrToPointerDecay(ir::Value* ptrValue) {
    auto* intType = ir::IntType::Create(m_IRProgram, ir::Alignment::GetPointerTypeSize());
    ir::Value* idxValue = m_IRProgram.CreateValue<ir::IntConstant>(
                                    intType, IntValue(0, /*isSigned=*/false));

    auto* ptrType = static_cast<ir::PointerType*>(ptrValue->GetType());
    auto* arrType = static_cast<ir::ArrayType*>(ptrType->GetSubType());

    auto* newPtrType = ir::PointerType::Create(arrType->GetSubType());

    auto* memberInstr = m_IRProgram.CreateValue<ir::MemberInstruction>(
                                    ptrValue, idxValue, "member", newPtrType, m_CurrentBB);
    memberInstr->SetDeref(true);

    m_CurrentBB->AddInstruction(memberInstr);
    return memberInstr;
}

ir::Instruction* IRGenAstVisitor::generateArrMemberExpression(ir::Value* ptrValue, ir::Value* intValue,
                                                              ast::Type* intType) {
    uint64_t intSize = ir::Alignment::GetTypeSize(intValue->GetType());
    uint64_t ptrSize = ir::Alignment::GetPointerTypeSize();

    ir::Value* idxValue = intValue;
    bool isConstant = getNumberIRConstant(idxValue);
    if (intSize < ptrSize && !isConstant) {
        auto* bytesType = ir::IntType::Create(m_IRProgram, ptrSize);
        idxValue = generateExtCast(idxValue, intType, bytesType);
    }

    bool isDeref = false;
    auto* ptrType = static_cast<ir::PointerType*>(ptrValue->GetType());
    if (auto* arrType = dynamic_cast<ir::ArrayType*>(ptrType->GetSubType())) {
        ptrType = ir::PointerType::Create(arrType->GetSubType());
        isDeref = true;
    }

    auto* memberInstr = m_IRProgram.CreateValue<ir::MemberInstruction>(
                                    ptrValue, idxValue, "member", ptrType, m_CurrentBB);
    memberInstr->SetDeref(isDeref);

    m_CurrentBB->AddInstruction(memberInstr);
    return memberInstr;
}

ir::Instruction* IRGenAstVisitor::generatePtrAddExpression(bool isAdd, ir::Value* ptrValue, ir::Value* intValue,
                                                           ast::Type* intType) {
    uint64_t intSize = ir::Alignment::GetTypeSize(intValue->GetType());
    uint64_t ptrSize = ir::Alignment::GetPointerTypeSize();

    ir::Value* idxValue = intValue;
    bool isConstant = getNumberIRConstant(idxValue);
    if (intType && intSize < ptrSize && !isConstant) {
        auto* bytesType = ir::IntType::Create(m_IRProgram, ptrSize);
        idxValue = generateExtCast(intValue, intType, bytesType);
    }

    if (!isAdd) {
        idxValue = generateNegExpression(idxValue);
    }

    bool isDeref = false;
    auto* ptrType = static_cast<ir::PointerType*>(ptrValue->GetType());
    if (auto* arrType = dynamic_cast<ir::ArrayType*>(ptrType->GetSubType())) {
        ptrType = ir::PointerType::Create(arrType->GetSubType());
        isDeref = true;
    }

    auto* memberInstr = m_IRProgram.CreateValue<ir::MemberInstruction>(
                            ptrValue, idxValue, "add.ptr", ptrValue->GetType(), m_CurrentBB);
    memberInstr->SetDeref(isDeref);

    m_CurrentBB->AddInstruction(memberInstr);
    return memberInstr;
}

ir::MemoryCopyInstruction* IRGenAstVisitor::createMemoryCopyInstruction(ir::Value* destination, ir::Value* source,
                                                                        size_t size) {
    auto* intType = ir::IntType::Create(m_IRProgram, ir::Alignment::GetPointerTypeSize());
    auto* sizeConstant = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(size));

    auto* memCopyInstr = m_IRProgram.CreateValue<ir::MemoryCopyInstruction>(
                                destination, source, sizeConstant, m_CurrentBB);

    m_CurrentBB->AddInstruction(memCopyInstr);
    return memCopyInstr;
}

ir::StoreInstruction* IRGenAstVisitor::createStoreInstruction(ir::Value* value, ir::Value* address,
                                                              bool isVolatile) {
    // TODO:...
    auto* ptrType = static_cast<ir::PointerType*>(address->GetType());
    if (auto* ptrSubType = dynamic_cast<ir::PointerType*>(ptrType->GetSubType())) {
        if (auto* intValue = dynamic_cast<ir::IntConstant*>(value)) {
            if (ir::Alignment::GetTypeSize(intValue->GetType()) < ir::Alignment::GetPointerTypeSize()) {
                auto* intType = ir::IntType::Create(m_IRProgram, ir::Alignment::GetPointerTypeSize());
                value = m_IRProgram.CreateValue<ir::IntConstant>(intType, intValue->GetValue());
            }
        }
    }

    auto* storeInstr = m_IRProgram.CreateValue<ir::StoreInstruction>(
        value, address, "", m_CurrentBB
    );
    if (isVolatile) {
        storeInstr->SetVolatile();
    }

    m_CurrentBB->AddInstruction(storeInstr);
    return storeInstr;
}

ir::LoadInstruction* IRGenAstVisitor::createLoadInstruction(ir::Value* fromPointer, ir::Type* toType,
                                                            bool isVolatile) {
    auto* loadInstr = m_IRProgram.CreateValue<ir::LoadInstruction>(
        fromPointer, toType, "", m_CurrentBB
    );
    if (isVolatile) {
        loadInstr->SetVolatile();
    }
    m_CurrentBB->AddInstruction(loadInstr);
    return loadInstr;
}

ir::CastInstruction* IRGenAstVisitor::createCastInstruction(ir::CastInstruction::OpType opType,
                                                            ir::Value* fromValue, ir::Type* toType) {
    auto* castInstr = m_IRProgram.CreateValue<ir::CastInstruction>(
        opType, "", fromValue, toType, m_CurrentBB
    );
    m_CurrentBB->AddInstruction(castInstr);
    return castInstr;
}

ir::Instruction* IRGenAstVisitor::generateCompareZeroInstruction(BinaryExpression::OpType opType,
                                                                 ir::Value* value, ast::Type* astType) {
    // TODO: Handle float
    auto* intType = ir::IntType::Create(m_IRProgram, 4);
    auto* zeroValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(0));
    return createCompareInstruction(opType, value, zeroValue, astType);
}

ir::Value* IRGenAstVisitor::generateLogInstruction(BinaryExpression::OpType opType,
                                                   ast::Expression* leftExpr,
                                                   ast::Expression* rightExpr,
                                                   ast::Type* leftType, ast::Type* rightType) {
    // TODO: Handle constants

    bool isAnd = (opType == BinaryExpression::OpType::kLogAnd);

    ir::BasicBlock* rightBB = createBasicBlock("land.rhs");
    ir::BasicBlock* endBB = createBasicBlock("land.end");

    auto* result = m_IRProgram.CreateValue<ir::AllocaInstruction>(
                                ir::IntType::Create(m_IRProgram, 1), "tmp", m_CurrentBB);
    m_AllocaBuffer.push(result);

    ir::Value* leftValue = Accept(*leftExpr);
    ir::Instruction* leftCmpInstr = nullptr;
    if (auto* cmpInstr = dynamic_cast<ir::CompareInstruction*>(leftValue)) {
        leftCmpInstr = cmpInstr;
    } else {
        leftCmpInstr = generateCompareZeroInstruction(
                            ast::BinaryExpression::OpType::kNEqual,
                            leftValue, leftType);
    }

    ir::BasicBlock* trueBB = rightBB;
    ir::BasicBlock* falseBB = endBB;
    if (!isAnd) {
        std::swap(trueBB, falseBB);
    }

    ir::StoreInstruction* leftStoreInstr = createStoreInstruction(leftCmpInstr, result);

    auto* branchInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
        leftCmpInstr, trueBB, falseBB, m_CurrentBB
    );
    m_CurrentBB->AddInstruction(branchInstr);

    // Right expression
    m_CurrentBB = rightBB;

    ir::Value* rightValue = Accept(*rightExpr);
    ir::Instruction* rightCmpInstr = nullptr;
    if (auto* cmpInstr = dynamic_cast<ir::CompareInstruction*>(rightValue)) {
        rightCmpInstr = cmpInstr;
    } else {
        rightCmpInstr = generateCompareZeroInstruction(
                            ast::BinaryExpression::OpType::kNEqual,
                            rightValue, rightType);
    }

    ir::StoreInstruction* rightStoreInstr = createStoreInstruction(rightCmpInstr, result);

    auto* branchEndInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
                            endBB, m_CurrentBB);
    m_CurrentBB->AddInstruction(branchEndInstr);

    // End
    m_CurrentBB = endBB;

    ir::LoadInstruction* loadInstr = createLoadInstruction(result, ir::IntType::Create(m_IRProgram, 1));

    return loadInstr;
}


ir::Instruction* IRGenAstVisitor::createCompareInstruction(BinaryExpression::OpType opType,
                                                           ir::Value* leftValue, ir::Value* rightValue,
                                                           ast::Type* astType) {
    bool isSignedInt = false;
    bool isUnsignedInt = false;
    bool isFloat = false;
    bool isPointer = dynamic_cast<ast::PointerType*>(astType);
    if (auto* builtinType = dynamic_cast<ast::BuiltinType*>(astType)) {
        isSignedInt = builtinType->IsSignedInteger();
        isUnsignedInt = builtinType->IsUnsignedInteger();
        isFloat = builtinType->IsFloat();
    }


    auto irOpType = ir::CompareInstruction::OpType::kNone;
    switch (opType) {
        case BinaryExpression::OpType::kLess:
            if (isSignedInt) {
                irOpType = ir::CompareInstruction::OpType::kISLess;
            } else if (isUnsignedInt || isPointer) {
                irOpType = ir::CompareInstruction::OpType::kIULess;
            } else if (isFloat) {
                irOpType = ir::CompareInstruction::OpType::kFLess;
            }
            break;
        case BinaryExpression::OpType::kGreater:
            if (isSignedInt) {
                irOpType = ir::CompareInstruction::OpType::kISGreater;
            } else if (isUnsignedInt || isPointer) {
                irOpType = ir::CompareInstruction::OpType::kIUGreater;
            } else if (isFloat) {
                irOpType = ir::CompareInstruction::OpType::kFGreater;
            }
            break;
        case BinaryExpression::OpType::kLessEq:
            if (isSignedInt) {
                irOpType = ir::CompareInstruction::OpType::kISLessEq;
            } else if (isUnsignedInt || isPointer) {
                irOpType = ir::CompareInstruction::OpType::kIULessEq;
            } else if (isFloat) {
                irOpType = ir::CompareInstruction::OpType::kFLessEq;
            }
            break;
        case BinaryExpression::OpType::kGreaterEq:
            if (isSignedInt) {
                irOpType = ir::CompareInstruction::OpType::kISGreaterEq;
            } else if (isUnsignedInt || isPointer) {
                irOpType = ir::CompareInstruction::OpType::kIUGreaterEq;
            } else if (isFloat) {
                irOpType = ir::CompareInstruction::OpType::kFGreaterEq;
            }
            break;
        case BinaryExpression::OpType::kEqual:
            if (isSignedInt || isUnsignedInt || isPointer) {
                irOpType = ir::CompareInstruction::OpType::kIEqual;
            } else if (isFloat) {
                irOpType = ir::CompareInstruction::OpType::kFEqual;
            }
            break;
        case BinaryExpression::OpType::kNEqual:
            if (isSignedInt || isUnsignedInt || isPointer) {
                irOpType = ir::CompareInstruction::OpType::kINEqual;
            } else if (isFloat) {
                irOpType = ir::CompareInstruction::OpType::kFNEqual;
            }
            break;
    }

    // auto* leftConst = dynamic_cast<ir::Constant*>(leftValue);
    // auto* rightConst =  dynamic_cast<ir::Constant*>(rightValue);
    // if (leftConst && rightConst) {
    //     return m_Constexpr.EvaluateCompareConstExpr(leftConst, rightConst, irOpType);
    // }

    auto* cmpInstr = m_IRProgram.CreateValue<ir::CompareInstruction>(
                                irOpType, "cmp", leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(cmpInstr);
    return cmpInstr;
}

ir::Value* IRGenAstVisitor::generateAdditiveExpression(BinaryExpression::OpType opType,
                                                       ir::Value* leftValue, ir::Value* rightValue,
                                                       ast::Type* leftType, ast::Type* rightType,
                                                       ast::Type* resultType) {
    bool isAdd = (opType == BinaryExpression::OpType::kAdd);

    bool isLeftPointer = dynamic_cast<ast::PointerType*>(leftType);
    bool isRightPointer = dynamic_cast<ast::PointerType*>(rightType);

    if (!isAdd && isLeftPointer && isRightPointer) {
        return generatePtrSubExpression(leftValue, rightValue);
    }

    if (isLeftPointer) {
        return generatePtrAddExpression(isAdd, leftValue, rightValue, rightType);
    }
    if (isRightPointer) {
        return generatePtrAddExpression(isAdd, rightValue, leftValue, leftType);
    }

    if (isAdd) {
        return createAddInstruction(leftValue, rightValue, resultType);
    }
    return createSubInstruction(leftValue, rightValue, resultType);
}

ir::Value* IRGenAstVisitor::createMulInstruction(ir::Value* leftValue, ir::Value* rightValue, ast::Type* type) {
    auto* builtinType = dynamic_cast<BuiltinType*>(type);
    
    auto opType = ir::BinaryInstruction::OpType::kMul;
    if (builtinType->IsFloat()) {
        opType = ir::BinaryInstruction::OpType::kFMul;
    }

    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst =  getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst, opType);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                opType, "mul", leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::Value* IRGenAstVisitor::createDivInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                            ast::Type* type) {
    auto* builtinType = dynamic_cast<BuiltinType*>(type);

    auto opType = ir::BinaryInstruction::OpType::kSDiv;
    if (builtinType->IsFloat()) {
        opType = ir::BinaryInstruction::OpType::kFDiv;
    } else if (builtinType->IsUnsignedInteger()) {
        opType = ir::BinaryInstruction::OpType::kUDiv;
    }

    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst =  getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst, opType);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                    opType, "div", leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::Value* IRGenAstVisitor::createRemInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                            ast::Type* type) {
    auto* builtinType = dynamic_cast<BuiltinType*>(type);

    assert(!builtinType->IsFloat());

    auto opType = ir::BinaryInstruction::OpType::kSRem;
    if (builtinType->IsUnsignedInteger()) {
        opType = ir::BinaryInstruction::OpType::kURem;
    }

    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst =  getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst, opType);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                    opType, "rem", leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::Value* IRGenAstVisitor::createShiftLInstruction(ir::Value* leftValue, ir::Value* rightValue) {
    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst =  getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst,
                                                    ir::BinaryInstruction::OpType::kShiftL);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                ir::BinaryInstruction::OpType::kShiftL, "shl",
                                leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::Value* IRGenAstVisitor::createShiftRInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                                ast::Type* type) {
    auto* builtinType = dynamic_cast<BuiltinType*>(type);

    auto opType = ir::BinaryInstruction::OpType::kAShiftR;
    std::string name = "ashr";
    if (builtinType->IsUnsignedInteger()) {
        opType = ir::BinaryInstruction::OpType::kLShiftR;
        name = "lshr";
    }

    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst =  getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst, opType);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                    opType, name, leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::Value* IRGenAstVisitor::createAndInstruction(ir::Value* leftValue, ir::Value* rightValue) {
    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst =  getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst,
                                                    ir::BinaryInstruction::OpType::kAnd);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                        ir::BinaryInstruction::OpType::kAnd, "and",
                                        leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::Value* IRGenAstVisitor::createXorInstruction(ir::Value* leftValue, ir::Value* rightValue) {
    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst =  getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst,
                                                    ir::BinaryInstruction::OpType::kXor);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                        ir::BinaryInstruction::OpType::kXor, "xor",
                                        leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::Value* IRGenAstVisitor::createOrInstruction(ir::Value* leftValue, ir::Value* rightValue) {
    auto* leftConst = getNumberIRConstant(leftValue);
    auto* rightConst = getNumberIRConstant(rightValue);
    if (leftConst && rightConst) {
        return m_Constexpr.EvaluateBinaryConstExpr(leftConst, rightConst,
                                                    ir::BinaryInstruction::OpType::kOr);
    }

    auto* instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
                                        ir::BinaryInstruction::OpType::kOr, "or",
                                        leftValue, rightValue, m_CurrentBB);
    m_CurrentBB->AddInstruction(instruction);
    return instruction;
}

ir::BasicBlock* IRGenAstVisitor::createBasicBlock(const std::string& name) {
    auto* labelType = ir::LabelType::Create(m_IRProgram);
    auto* basicBlock = m_IRProgram.CreateValue<ir::BasicBlock>(name, labelType, m_CurrentFunction);

    m_FunBBMap[name] = basicBlock;
    m_CurrentFunction->AddBasicBlock(basicBlock);

    return basicBlock;
}

ir::Constant* IRGenAstVisitor::getNumberIRConstant(ir::Value* value) {
    if (auto* intConst = dynamic_cast<ir::IntConstant*>(value)) {
        return intConst;
    }
    if (auto* floatConst = dynamic_cast<ir::FloatConstant*>(value)) {
        return floatConst;
    }
    return nullptr;
}

void IRGenAstVisitor::generateAllocas() {
    ir::BasicBlock* entryBlock = m_CurrentFunction->GetEntryBlock(); 
    while (!m_AllocaBuffer.empty()) {
        entryBlock->AddInstructionToBegin(m_AllocaBuffer.top());
        m_AllocaBuffer.pop();
    }
}

void IRGenAstVisitor::resetFunctionData() {
    m_CurrentFunction = nullptr;
    m_CurrentBB = nullptr;
    m_ReturnBlocks.clear();
    m_FunBBMap.clear();
    m_AllocasMap.clear();
}

}  // namespace ast
