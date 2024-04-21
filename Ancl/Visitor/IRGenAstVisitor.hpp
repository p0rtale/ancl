#pragma once

#include <unordered_map>

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Visitor/AstVisitor.hpp>
#include <Ancl/Grammar/AST/ASTProgram.hpp>

#include <Ancl/AnclIR/IR.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

#include <Ancl/DataLayout/Alignment.hpp>


namespace ast {

class IRGenAstVisitor: public AstVisitor {
public:
    IRGenAstVisitor(ir::IRProgram& irProgram): m_IRProgram(irProgram) {}

    void Run(const ASTProgram& astProgram) {
        Visit(*astProgram.GetTranslationUnit());

        // TODO: update instructions names
    }

public:
    /*
    =================================================================
                                Declaration
    =================================================================
    */

    void Visit(Declaration&) override {
        // Base class
    }

    // Skip
    void Visit(EnumConstDeclaration& enumConstDecl) override {}

    // Skip
    void Visit(EnumDeclaration& enumDecl) override {}

    // Skip
    void Visit(FieldDeclaration& fieldDecl) override {}

    void Visit(FunctionDeclaration& funcDecl) override {
        m_InGlobalScope = false;

        auto qualType = funcDecl.GetType();
        auto funcIRType = dynamic_cast<ir::FunctionType*>(Accept(*qualType));
        assert(funcIRType);

        // TODO: handle big struct as return value (add ptr parameter + void return)

        auto storageClass = funcDecl.GetStorageClass();
        auto linkage = ir::GlobalValue::LinkageType::kExtern;
        if (storageClass == StorageClass::kStatic) {
            linkage = ir::GlobalValue::LinkageType::kStatic;
        }

        auto functionValue = m_IRProgram.CreateValue<ir::Function>(
            funcIRType, linkage, funcDecl.GetName()
        );
        m_IRProgram.AddFunction(functionValue);

        m_CurrentFunction = functionValue;
        for (auto* param : funcDecl.GetParams()) {
            m_InsideParams = true;
            param->Accept(*this);
            m_InsideParams = false;
        }

        auto body = funcDecl.GetBody();
        if (body) {
            body->Accept(*this);
            resetFunctionData();
        }

        // TODO: handle function without return (add void return)
        // TODO: handle multiple returns

    }

    void Visit(LabelDeclaration& labelDecl) override {
        auto nextBB = createBasicBlock(labelDecl.GetName());

        auto branch = m_IRProgram.CreateValue<ir::BranchInstruction>(nextBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(branch);

        m_CurrentBB = nextBB;
    }

    // Skip
    void Visit(RecordDeclaration& recordDecl) override {}

    void Visit(TagDeclaration&) override {
        // Base class
    }

    void Visit(TranslationUnit& unit) override {
        for (const auto& decl : unit.GetDeclarations()) {
            m_InGlobalScope = true;
            decl->Accept(*this);
        }
    }

    void Visit(TypeDeclaration&) override {
        // Base class
    }

    // Skip
    void Visit(TypedefDeclaration& typedefDecl) override {}

    void Visit(ValueDeclaration& valueDecl) override {
        // Base class
    }

    void Visit(VariableDeclaration& varDecl) override {
        auto varIRType = Accept(*varDecl.GetType());
        auto name = varDecl.GetName();
        auto storageClass = varDecl.GetStorageClass();

        // TODO: handle volatile, auto and register
        // TODO: handle global var init
        ir::GlobalVariable* globalVar = nullptr;
        if (m_InGlobalScope) {
            auto linkage = ir::GlobalValue::LinkageType::kExtern;
            if (storageClass == StorageClass::kStatic) {
                linkage = ir::GlobalValue::LinkageType::kStatic;
            }
            auto ptrType = ir::PointerType::Create(varIRType);
            globalVar = m_IRProgram.CreateValue<ir::GlobalVariable>(
                ptrType, linkage, name
            );
            m_IRProgram.AddGlobalVar(globalVar);
        } else if (storageClass == StorageClass::kStatic) {
            auto mangledName = std::format("{}.{}", m_CurrentFunction->GetName(), name);
            auto ptrType = ir::PointerType::Create(varIRType);
            globalVar = m_IRProgram.CreateValue<ir::GlobalVariable>(
                ptrType, ir::GlobalValue::LinkageType::kStatic, mangledName
            );
            m_IRProgram.AddGlobalVar(globalVar);     
        }

        if (globalVar) {
            auto init = varDecl.GetInit();
            if (!init) {
                return;
            }

            auto initList = dynamic_cast<InitializerList*>(init);
            assert(initList && "Must be initializer list");

            auto constants = AcceptConst(*initList);
            globalVar->SetInit(constants);
            return;
        }

        auto mangledName = std::format("{}.addr", name);
        if (!m_InsideParams) {
            mangledName = getValueName(name);
        }

        auto alloca = m_IRProgram.CreateValue<ir::AllocaInstruction>(
            varIRType, mangledName, m_CurrentBB
        );
        m_FunValuesMap[alloca->GetName()] = alloca;
        m_CurrentBB->AddInstruction(alloca);

        assert(varDecl.GetInit() && "Decl and Init must be separated");

        if (m_InsideParams) {
            auto paramValue = m_IRProgram.CreateValue<ir::Parameter>(
                    name, varIRType, m_CurrentFunction);
            auto storeInstr = createStoreInstruction(alloca, paramValue);
            m_CurrentBB->AddInstruction(storeInstr);
        }
    }


    /*
    =================================================================
                                Statement
    =================================================================
    */

    void Visit(Statement&) override {
        // Base class
    }

    void Visit(CaseStatement& caseStmt) override {}

    void Visit(CompoundStatement& compoundStmt) override {
        for (const auto& stmt : compoundStmt.GetBody()) {
            stmt->Accept(*this);
        }
    }

    void Visit(DeclStatement& declStmt) override {
        for (const auto& decl : declStmt.GetDeclarations()) {
            decl->Accept(*this);
        } 
    }

    void Visit(DefaultStatement& defaultStmt) override {}

    void Visit(DoStatement& doStmt) override {
        auto bodyBB = createBasicBlock("do.body");
        auto condBB = createBasicBlock("do.cond");
        auto endBB = createBasicBlock("do.end");

        // Entry
        auto bodyBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(bodyBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(bodyBranch);

        // Body
        m_CurrentBB = bodyBB;
        m_ContinueBB = condBB;
        m_BreakBB = endBB;
        auto body = doStmt.GetBody();
        body->Accept(*this);
        auto loopBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(loopBranch);

        // Condition
        // TODO: handle const condition
        m_CurrentBB = condBB;

        auto condExpr = doStmt.GetCondition();
        auto condValue = Accept(*condExpr);
        auto compareInstr = dynamic_cast<ir::CompareInstruction*>(condValue);
        if (!compareInstr) {
            auto exprQualType = condExpr->GetType();
            auto exprType = exprQualType.GetSubType();
            condValue = generateCompareZeroInstruction(ast::BinaryExpression::OpType::kNEqual,
                                                       condValue, exprType);
        }

        auto condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, bodyBB, endBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(condBranch);

        m_CurrentBB = endBB;
    }

    void Visit(ForStatement& forStmt) override {
        auto condBB = createBasicBlock("for.cond");
        
        auto forCondition = forStmt.GetCondition();
        ir::BasicBlock* bodyBB = nullptr;
        if (forCondition) {
            bodyBB = createBasicBlock("for.body");
        }

        auto stepExpr = forStmt.GetStep();
        ir::BasicBlock* stepBB = nullptr;
        if (stepExpr) {
            stepBB = createBasicBlock("for.step");
        }
        
        auto endBB = createBasicBlock("for.end");

        // Init
        auto initStmt = forStmt.GetInit();
        initStmt->Accept(*this);

        auto entryBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(entryBranch);

        // Condition
        m_CurrentBB = condBB;
        if (forCondition) {
            auto condValue = Accept(*forCondition);
            auto condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, bodyBB, endBB, m_CurrentBB);
            m_CurrentBB->AddInstruction(condBranch);
            m_CurrentBB = bodyBB;
        }

        // Body
        if (stepBB) {
            m_ContinueBB = stepBB;
        } else {
            m_ContinueBB = condBB;
        }
        m_BreakBB = endBB;
        auto bodyStmt = forStmt.GetBody();
        bodyStmt->Accept(*this);

        if (stepBB) {
            auto stepBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(stepBB, m_CurrentBB);
            m_CurrentBB->AddInstruction(stepBranch);

            // Step
            m_CurrentBB = stepBB;
            stepExpr->Accept(*this);  // Ignore result value
        }
        auto condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(condBranch);

        m_CurrentBB = endBB;
    }

    void Visit(GotoStatement& gotoStmt) override {}

    void Visit(IfStatement& ifStmt) override {
        auto thenBB = createBasicBlock("if.then");

        auto elseStmt = ifStmt.GetElse();
        ir::BasicBlock* elseBB = nullptr;
        if (elseStmt) {
            elseBB = createBasicBlock("if.else");
        }

        auto endBB = createBasicBlock("if.end");

        // Condition
        // TODO: handle const condition
        auto condExpr = ifStmt.GetCondition();
        auto condValue = Accept(*condExpr);
        auto compareInstr = dynamic_cast<ir::CompareInstruction*>(condValue);
        if (!compareInstr) {
            auto exprQualType = condExpr->GetType();
            auto exprType = exprQualType.GetSubType();
            condValue = generateCompareZeroInstruction(ast::BinaryExpression::OpType::kNEqual,
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

        auto thenStmt = ifStmt.GetThen();
        thenStmt->Accept(*this);

        auto endBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(endBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(endBranch);

        // Else
        if (elseBB) {
            m_CurrentBB = elseBB;

            elseStmt->Accept(*this);

            auto endBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(endBB, m_CurrentBB);
            m_CurrentBB->AddInstruction(endBranch);
        }

        m_CurrentBB = endBB;
    }

    void Visit(LabelStatement& labelStmt) override {
        auto labelDecl = labelStmt.GetLabel();
        labelDecl->Accept(*this);

        auto body = labelStmt.GetBody();
        body->Accept(*this);
    }

    void Visit(LoopJumpStatement& loopJmpStmt) override {
        ir::BasicBlock* toBasicBlock = nullptr;

        auto jmpType = loopJmpStmt.GetType();
        if (jmpType == LoopJumpStatement::Type::kContinue) {
            toBasicBlock = m_ContinueBB;
        } else {  // Break
            toBasicBlock = m_BreakBB;
        }

        auto branchInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
            toBasicBlock, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(branchInstr);
    }

    void Visit(ReturnStatement& returnStmt) override {
        auto returnExpr = returnStmt.GetReturnExpression();
        auto returnValue = Accept(*returnExpr);

        // TODO: handle struct value

        auto returnInstr = m_IRProgram.CreateValue<ir::ReturnInstruction>(
            returnValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(returnInstr);
    }

    void Visit(SwitchCase& switchCase) override {
        // Base class
    }

    void Visit(SwitchStatement& switchStmt) override {}

    void Visit(ValueStatement& valueStmt) override {
        // Base class
    }

    void Visit(WhileStatement& whileStmt) override {
        auto condBB = createBasicBlock("while.cond");
        auto bodyBB = createBasicBlock("while.body");
        auto endBB = createBasicBlock("while.end");

        // Entry
        auto entryBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(entryBranch);

        // Condition
        // TODO: handle const condition (+ endless loop)
        m_CurrentBB = condBB;

        auto condExpr = whileStmt.GetCondition();
        auto condValue = Accept(*condExpr);
        auto compareInstr = dynamic_cast<ir::CompareInstruction*>(condValue);
        if (!compareInstr) {
            auto exprQualType = condExpr->GetType();
            auto exprType = exprQualType.GetSubType();
            condValue = generateCompareZeroInstruction(ast::BinaryExpression::OpType::kNEqual,
                                                       condValue, exprType);
        }

        auto condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, bodyBB, endBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(condBranch);

        // Body
        m_CurrentBB = bodyBB;
        m_ContinueBB = condBB;
        m_BreakBB = endBB;
        auto body = whileStmt.GetBody();
        body->Accept(*this);
        auto loopBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, m_CurrentBB);
        m_CurrentBB->AddInstruction(loopBranch);

        m_CurrentBB = endBB;
    }


    /*
    =================================================================
                                Expression
    =================================================================
    */

    // TODO: template Visitor
    ir::Value* Accept(Expression& expr) {
        expr.Accept(*this);  // -> m_IRValue
        return m_IRValue;
    }

    void Visit(Expression&) override {
        // Base class
    }

    void Visit(BinaryExpression& binaryExpr) override {
        auto opType = binaryExpr.GetOpType();

        auto leftOperand = binaryExpr.GetLeftOperand();
        auto leftQualType = leftOperand->GetType();
        auto leftType = leftQualType.GetSubType();

        auto rightOperand = binaryExpr.GetRightOperand();
        auto rightQualType = rightOperand->GetType();
        auto rightType = rightQualType.GetSubType();

        auto resultQualType = binaryExpr.GetType();
        auto resultType = resultQualType.GetSubType();

        if (opType == BinaryExpression::OpType::kLogAnd ||
                opType == BinaryExpression::OpType::kLogOr) {
            m_IRValue = generateLogInstruction(opType, leftOperand, rightOperand,
                                                leftType, rightType);
            return;
        }

        auto leftValue = Accept(*leftOperand);

        if (opType == BinaryExpression::OpType::kDirectMember ||
                opType == BinaryExpression::OpType::kArrowMember) {
            auto declRefExpr = dynamic_cast<ast::DeclRefExpression*>(rightOperand);
            assert(declRefExpr && "Must be DeclRefExpr");
            m_IRValue = generateStructMemberExpression(leftValue, declRefExpr);
            return;
        }

        auto rightValue = Accept(*rightOperand);

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

            // Skip compound assignments
            // TODO: rewrite syntactic sugar
            case BinaryExpression::OpType::kAssign:
                m_IRValue = createStoreInstruction(leftValue, rightValue);
                return;

            case BinaryExpression::OpType::kArrSubscript:
                m_IRValue = generateArrMemberExpression(leftValue, rightValue, rightType);
                return;
        }

        return;
    }

    void Visit(CallExpression& callExpr) override {
        auto calleeExpr = callExpr.GetCallee();
        auto calleeValue = Accept(*calleeExpr);
        auto functionValue = dynamic_cast<ir::Function*>(calleeValue);
        assert(functionValue);

        auto argExprs = callExpr.GetArguments();
        std::vector<ir::Value*> argValues;
        argValues.reserve(argExprs.size());
        for (const auto argExpr : argExprs) {
            argValues.push_back(Accept(*argExpr));
        }

        // TODO: handle struct arguments and struct return
        auto callInstr = m_IRProgram.CreateValue<ir::CallInstruction>(
            functionValue, argValues,
            getValueName("call"), m_CurrentBB
        );
        m_CurrentBB->AddInstruction(callInstr);
        m_IRValue = callInstr;
    }

    void Visit(CastExpression& castExpr) override {
        // TODO: ...
    }

    void Visit(CharExpression& charExpr) override {
        auto qualType = charExpr.GetType();
        auto intType = dynamic_cast<ir::IntType*>(Accept(*qualType.GetSubType()));
        assert(intType);

        auto intValue = IntValue(charExpr.GetCharValue());
        m_IRValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, intValue);
    }

    void Visit(ConditionalExpression& condExpr) override {
        // TODO: handle const

        auto trueBB = createBasicBlock("cond.true");
        auto falseBB = createBasicBlock("cond.false");
        auto endBB = createBasicBlock("cond.end");

        auto condition = condExpr.GetCondition();
        auto conditionQualType = condition->GetType();
        auto conditionType = conditionQualType.GetSubType();
        auto condValue = Accept(*condition);

        ir::Instruction* compareInstr = nullptr;
        if (auto cmpInstr = dynamic_cast<ir::CompareInstruction*>(condValue)) {
            compareInstr = cmpInstr;
        } else {
            compareInstr = generateCompareZeroInstruction(
                                ast::BinaryExpression::OpType::kNEqual,
                                condValue, conditionType);
        }

        auto branchInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
            compareInstr, trueBB, falseBB, m_CurrentBB
        );

        m_CurrentBB->AddInstruction(compareInstr);
        m_CurrentBB->AddInstruction(branchInstr);

        // True
        m_CurrentBB = trueBB;

        auto trueExpr = condExpr.GetTrueExpression();
        auto trueValue = Accept(*trueExpr);
 
        auto result = m_IRProgram.CreateValue<ir::AllocaInstruction>(
            trueValue->GetType(), getValueName("tmp"), m_CurrentBB
        );
        m_CurrentBB->AddInstruction(result);

        auto trueStoreInstr = createStoreInstruction(result, trueValue);

        auto trueBranchEndInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
                                    endBB, m_CurrentBB);

        m_CurrentBB->AddInstruction(trueStoreInstr);
        m_CurrentBB->AddInstruction(trueBranchEndInstr);
    
        // False
        m_CurrentBB = falseBB;

        auto falseExpr = condExpr.GetFalseExpression();
        auto falseValue = Accept(*falseExpr);
 
        auto falseStoreInstr = createStoreInstruction(result, falseValue);

        auto falseBranchEndInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
                                      endBB, m_CurrentBB);

        m_CurrentBB->AddInstruction(falseBranchEndInstr);
        m_CurrentBB->AddInstruction(falseBranchEndInstr);

        // End
        m_CurrentBB = endBB;

        auto loadInstr = createLoadInstruction(result, trueValue->GetType());
        m_CurrentBB->AddInstruction(loadInstr);

        m_IRValue = loadInstr; 
    }

    // Skip
    void Visit(ConstExpression& constExpr) override {}

    void Visit(DeclRefExpression& declrefExpr) override {
        // TODO: add load for cast[lvalue -> rvalue]

        auto declaration = declrefExpr.GetDeclaration();
        auto name = declaration->GetName();

        if (m_IRProgram.HasGlobalVar(name)) {
            m_IRValue = m_FunValuesMap.at(name);
            return;       
        }

        if (m_IRProgram.HasFunction(name)) {
            m_IRValue = m_FunValuesMap.at(name);
            return;
        }

        if (m_FunValuesMap.contains(name)) {
            m_IRValue = m_FunValuesMap.at(name);
            return;
        }

        assert(false && "Unknown name (DeclRefExpr)");
        return;
    }

    void Visit(ExpressionList& exprList) override {
        // TODO: ...
    }

    void Visit(FloatExpression& floatExpr) override {
        auto qualType = floatExpr.GetType();
        auto floatType = dynamic_cast<ir::FloatType*>(Accept(*qualType.GetSubType()));
        assert(floatType);

        auto floatValue = floatExpr.GetFloatValue();
        m_IRValue = m_IRProgram.CreateValue<ir::FloatConstant>(floatType, floatValue);
    }

    std::vector<ir::Constant*> AcceptConst(InitializerList& initList) {
        m_ConstantList.clear();
        initList.Accept(*this);  // -> m_ConstantList
        return m_ConstantList;
    }

    void Visit(InitializerList& initList) override {
        // TODO: memcpy or loop
    }

    void Visit(IntExpression& intExpr) override {
        auto qualType = intExpr.GetType();
        auto intType = dynamic_cast<ir::IntType*>(Accept(*qualType.GetSubType()));
        assert(intType);

        auto intValue = intExpr.GetIntValue();
        m_IRValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, intValue);
    }

    // Skip
    void Visit(SizeofTypeExpression& sizeofTypeExpr) override {}

    void Visit(StringExpression& stringExpr) override {
        static uint counter = 0;
        auto mangledName = std::format(".L.str{}", counter++);

        auto qualType = stringExpr.GetType();
        auto charsType = Accept(qualType);

        auto strValue = m_IRProgram.CreateValue<ir::GlobalVariable>(
                charsType, ir::GlobalValue::LinkageType::kStatic, mangledName);

        m_IRProgram.AddGlobalVar(strValue);
        m_IRValue = strValue;
    }

    void Visit(UnaryExpression& unaryExpr) override {
        auto operandExpr = unaryExpr.GetOperand();
        auto operandValue = Accept(*operandExpr);

        auto exprQualType = unaryExpr.GetType();
        auto exprType = exprQualType.GetSubType();

        auto opType = unaryExpr.GetOpType();
        switch (opType) {
            case UnaryExpression::OpType::kPreInc:
            case UnaryExpression::OpType::kPreDec:
            case UnaryExpression::OpType::kPostInc:
            case UnaryExpression::OpType::kPostDec:
                m_IRValue = generateIncDecExpression(opType, operandValue, exprType);
                return;

            case UnaryExpression::OpType::kAddrOf:
            case UnaryExpression::OpType::kDeref:
                // TODO: check (rely on cast[lvalue -> rvalue])
                m_IRValue = operandValue;
                return;

            // Ignore Plus
            case UnaryExpression::OpType::kMinus:
                m_IRValue = generateNegExpression(operandValue);
                return;
            case UnaryExpression::OpType::kNot:
                m_IRValue = generateNotExpression(operandValue);
                return;
            case UnaryExpression::OpType::kLogNot:
                m_IRValue = generateCompareZeroInstruction(ast::BinaryExpression::OpType::kEqual,
                                                           operandValue, exprType);
                return;

            // Skip sizeof
        }
    }


    /*
    =================================================================
                                Type
    =================================================================
    */

    // TODO: template Visitor
    ir::Type* Accept(Type& type) {
        type.Accept(*this);  // -> m_IRType
        return m_IRType;
    }

    ir::Type* Accept(QualType& qualType) {
        qualType.Accept(*this);  // -> m_IRType
        return m_IRType;
    }

    void Visit(ArrayType& arrayType) override {
        auto elementType = Accept(*arrayType.GetSubType());
        auto sizeValue = arrayType.GetSize();  // unsigned
        m_IRType = m_IRProgram.CreateType<ir::ArrayType>(elementType, sizeValue.GetValue());
    }

    void Visit(BuiltinType& builtinType) override {
        switch (builtinType.GetKind()) {
            case BuiltinType::Kind::kVoid:
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
                // TODO: handle error
                break;
        }
    }

    void Visit(EnumType& enumType) override {
        m_IRType = ir::IntType::Create(m_IRProgram, 32);
    }

    void Visit(FunctionType& funcType) override {
        auto returnType = Accept(*funcType.GetSubType());

        std::vector<ir::Type*> paramIRTypes;
        auto paramAstTypes = funcType.GetParamTypes();
        paramIRTypes.reserve(paramAstTypes.size());
        for (const auto paramAstType : paramAstTypes) {
            paramIRTypes.push_back(Accept(*paramAstType));
        }

        bool isVariadic = funcType.IsVariadic();

        m_IRType = m_IRProgram.CreateType<ir::FunctionType>(returnType, paramIRTypes, isVariadic);
    }

    void Visit(PointerType& ptrType) override {
        auto elementType = Accept(*ptrType.GetSubType());
        m_IRType = ir::PointerType::Create(elementType);
    }

    void Visit(QualType& qualType) override {
        auto astType = qualType.GetSubType();
        // TODO: handle qualifiers
        m_IRType = Accept(*astType);
    }

    void Visit(RecordType& recordType) override {
        std::vector<ir::Type*> elementTypes;

        auto decl = recordType.GetDeclaration();
        auto fields = decl->GetFields();
        elementTypes.reserve(fields.size());

        for (const auto fieldDecl : fields) {
            auto fieldASTType = fieldDecl->GetType();
            auto fieldIRType = Accept(*fieldASTType);
            elementTypes.push_back(fieldIRType);
        }

        if (decl->IsStruct()) {
            m_IRType = ir::StructType::Create(elementTypes);
            return;
        }
        
        // Union
        size_t maxSize = 0;
        size_t maxAlignment = 0;
        ir::Type* maxAlignType = nullptr;
        size_t maxAlignTypeSize = 0;
        for (const auto elemType : elementTypes) {
            size_t elemSize = ir::Alignment::GetTypeSize(elemType);
            maxSize = std::max(maxSize, elemSize);

            size_t elemAlignment = ir::Alignment::GetTypeAlignment(elemType);
            if (elemAlignment > maxAlignment) {
                maxAlignment = elemAlignment;
                maxAlignType = elemType;
                maxAlignTypeSize = elemSize;
            }
        }

        std::vector<ir::Type*> unionElemTypes{maxAlignType};

        size_t unionSize = ir::Alignment::Align(maxSize, maxAlignment);
        if (unionSize > maxSize) {
            auto byteType = ir::IntType::Create(m_IRProgram, 1);
            auto restBytesType = ir::ArrayType::Create(byteType, unionSize - maxSize);
            unionElemTypes.push_back(restBytesType);
        }

        m_IRType = ir::StructType::Create(unionElemTypes);
    }

    void Visit(TagType&) override {
        // Base class
    }
    
    void Visit(Type&) override {
        // Base class
    }

    // Skip
    void Visit(TypedefType& typedefType) override {}

private:
    ir::BinaryInstruction* createAddInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                                ast::Type* type) {
        auto builtinType = dynamic_cast<BuiltinType*>(type);

        auto opType = ir::BinaryInstruction::OpType::kAdd;
        if (builtinType->IsFloat()) {
            opType = ir::BinaryInstruction::OpType::kFAdd;
        }

        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            opType, getValueName("add"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::BinaryInstruction* createSubInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                                ast::Type* type) {
        auto builtinType = dynamic_cast<BuiltinType*>(type);

        auto opType = ir::BinaryInstruction::OpType::kSub;
        if (builtinType->IsFloat()) {
            opType = ir::BinaryInstruction::OpType::kFSub;
        }

        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            opType, getValueName("sub"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    // TODO:!!! dynamic_cast to ptr<ir::type>
    ir::Instruction* generateIncDecExpression(UnaryExpression::OpType opType, ir::Value* value,
                                              ast::Type* type) {
        bool isInc = true;
        if (opType == UnaryExpression::OpType::kPreDec ||
                opType == UnaryExpression::OpType::kPostDec) {
            isInc = false;
        }

        auto valuePtrType = dynamic_cast<ir::PointerType*>(value->GetType());
        assert(valuePtrType);
        auto valueSubType = valuePtrType->GetSubType();

        bool isPointer = false;
        ir::Value* constValue = nullptr;
        if (auto intType = dynamic_cast<ir::IntType*>(valueSubType)) {
            int inc = isInc ? 1 : -1;
            constValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(inc));
        } else if (auto floatType = dynamic_cast<ir::FloatType*>(valueSubType)) {
            double inc = isInc ? 1. : -1.;
            constValue = m_IRProgram.CreateValue<ir::FloatConstant>(floatType, FloatValue(inc));
        } else if (auto pointerType = dynamic_cast<ir::PointerType*>(valueSubType)) {
            isPointer = true;
            int inc = isInc ? 1 : -1;
            auto intType = ir::IntType::Create(m_IRProgram, ir::Alignment::GetPointerTypeSize());
            constValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(inc));
        }

        auto loadInstr = createLoadInstruction(value, valueSubType);

        ir::Instruction* resultInstr = nullptr;
        if (isPointer) {
            resultInstr = generatePtrAddExpression(/*isAdd=*/true, loadInstr, constValue);
        } else {
            resultInstr = createAddInstruction(loadInstr, constValue, type);
        }

        auto storeInstr = createStoreInstruction(value, resultInstr);

        if (opType == UnaryExpression::OpType::kPreDec ||
                opType == UnaryExpression::OpType::kPreInc) {
            return resultInstr;
        }

        return loadInstr;
    }

    ir::BinaryInstruction* generatePtrSubExpression(ir::Value* leftValue, ir::Value* rightValue) {
        auto ptrType = dynamic_cast<ir::PointerType*>(leftValue->GetType());
        auto bytesType = ir::IntType::Create(m_IRProgram, ir::Alignment::GetPointerTypeSize());

        auto leftPTIInstr = m_IRProgram.CreateValue<ir::CastInstruction>(
            ir::CastInstruction::OpType::kPtrToI, getValueName("sub.ptr.lhs.cast"),
            leftValue, bytesType, m_CurrentBB
        );
        auto rightPTIInstr = m_IRProgram.CreateValue<ir::CastInstruction>(
            ir::CastInstruction::OpType::kPtrToI, getValueName("sub.ptr.rhs.cast"),
            rightValue, bytesType, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(leftPTIInstr);
        m_CurrentBB->AddInstruction(rightPTIInstr);

        auto subInstr = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            ir::BinaryInstruction::OpType::kSub, getValueName("sub.ptr.sub"),
            leftPTIInstr, rightPTIInstr, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(subInstr);

        size_t subTypeSize = ir::Alignment::GetTypeSize(ptrType->GetSubType());
        auto sizeIntConstant = m_IRProgram.CreateValue<ir::IntConstant>(
                                    bytesType, IntValue(subTypeSize, false));

        auto divInstr = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            ir::BinaryInstruction::OpType::kSDiv, getValueName("sub.ptr.div"),
            subInstr, sizeIntConstant, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(divInstr);

        return divInstr;
    }

    ir::CastInstruction* generateExtCast(ir::Value* fromValue, ast::Type* fromASTType, ir::Type* toIRType) {
        auto builtinType = static_cast<ast::BuiltinType*>(fromASTType);

        auto opType = ir::CastInstruction::OpType::kNone;
        if (builtinType->IsSignedInteger()) {
            opType = ir::CastInstruction::OpType::kSExt;
        } else if (builtinType->IsUnsignedInteger()) {
            opType = ir::CastInstruction::OpType::kZExt;
        } else {
            opType = ir::CastInstruction::OpType::kFExt;
        }

        auto extInstr = m_IRProgram.CreateValue<ir::CastInstruction>(
            opType, getValueName("ext"),
            fromValue, toIRType, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(extInstr);

        return extInstr;
    }

    ir::Instruction* generateNotExpression(ir::Value* value) {
        auto intType = ir::IntType::Create(m_IRProgram, 4);
        auto allOnesValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(-1, false));
        return createXorInstruction(value, allOnesValue);
    }

    ir::Instruction* generateNegExpression(ir::Value* value) {
        auto valueType = value->GetType();

        auto opType = ir::BinaryInstruction::OpType::kNone;
        ir::Value* zeroValue = nullptr;
        if (auto intType = dynamic_cast<ir::IntType*>(valueType)) {
            opType = ir::BinaryInstruction::OpType::kSub;
            zeroValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(0, false));
        } else if (auto floatType = dynamic_cast<ir::FloatType*>(valueType)) {
            opType = ir::BinaryInstruction::OpType::kFSub;
            zeroValue = m_IRProgram.CreateValue<ir::FloatConstant>(floatType, FloatValue(0.));
        }

        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            opType, getValueName("neg"),
            zeroValue, value, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::Instruction* generateStructMemberExpression(ir::Value* structValue,
                                                    ast::DeclRefExpression* memberExpr) {
        auto declaration = memberExpr->GetDeclaration();
        auto fieldDecl = dynamic_cast<ast::FieldDeclaration*>(declaration);
        assert(fieldDecl && "Must be field declaration");

        size_t fieldIndex = fieldDecl->GetPosition();
        auto intType = ir::IntType::Create(m_IRProgram, 4);
        auto idxValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(fieldIndex));

        auto memberExprType = memberExpr->GetType();
        auto memberIRType = Accept(memberExprType);
        auto memberPtrType = ir::PointerType::Create(memberIRType);

        // TODO: deref? (cast[arr -> ptr])
        auto memberInstr = m_IRProgram.CreateValue<ir::MemberInstruction>(
            structValue, idxValue, getValueName("member"),
            memberPtrType, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(memberInstr);
        return memberInstr;
    }

    ir::Instruction* generateArrMemberExpression(ir::Value* ptrValue, ir::Value* intValue,
                                                 ast::Type* intType) {
        auto intSize = ir::Alignment::GetTypeSize(intValue->GetType());
        auto ptrSize = ir::Alignment::GetPointerTypeSize();

        auto idxValue = intValue;
        if (intSize < ptrSize) {
            auto bytesType = ir::IntType::Create(m_IRProgram, ptrSize);
            idxValue = generateExtCast(idxValue, intType, bytesType);
        }

        auto ptrType = ptrValue->GetType();
        if (auto arrType = dynamic_cast<ir::ArrayType*>(ptrType)) {
            ptrType = ir::PointerType::Create(arrType->GetSubType());
        }

        // TODO: deref? (cast[arr -> ptr])
        auto memberInstr = m_IRProgram.CreateValue<ir::MemberInstruction>(
            ptrValue, idxValue, getValueName("member"),
            ptrType, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(memberInstr);
        return memberInstr;
    }

    ir::Instruction* generatePtrAddExpression(bool isAdd, ir::Value* ptrValue, ir::Value* intValue,
                                              ast::Type* intType = nullptr) {
        auto intSize = ir::Alignment::GetTypeSize(intValue->GetType());
        auto ptrSize = ir::Alignment::GetPointerTypeSize();

        auto idxValue = intValue;
        if (intType && intSize < ptrSize) {
            auto bytesType = ir::IntType::Create(m_IRProgram, ptrSize);
            idxValue = generateExtCast(intValue, intType, bytesType);
        }

        if (!isAdd) {
            idxValue = generateNegExpression(idxValue);
        }

        // TODO: deref? (cast[arr -> ptr])
        auto memberInstr = m_IRProgram.CreateValue<ir::MemberInstruction>(
            ptrValue, idxValue, getValueName("add.ptr"),
            ptrValue->GetType(), m_CurrentBB
        );
        m_CurrentBB->AddInstruction(memberInstr);
        return memberInstr;
    }

    ir::StoreInstruction* createStoreInstruction(ir::Value* toValue, ir::Value* fromValue) {
        auto storeInstr = m_IRProgram.CreateValue<ir::StoreInstruction>(
            fromValue, toValue, "", m_CurrentBB
        );
        m_CurrentBB->AddInstruction(storeInstr);
        return storeInstr;
    }

    ir::LoadInstruction* createLoadInstruction(ir::Value* fromPointer, ir::Type* toType) {
        auto loadInstr = m_IRProgram.CreateValue<ir::LoadInstruction>(
            fromPointer, toType, "", m_CurrentBB
        );
        m_CurrentBB->AddInstruction(loadInstr);
        return loadInstr;
    }

    ir::Instruction* generateCompareZeroInstruction(BinaryExpression::OpType opType,
                                                  ir::Value* value, ast::Type* astType) {
        // TODO: handle float
        auto intType = ir::IntType::Create(m_IRProgram, 4);
        auto zeroValue = m_IRProgram.CreateValue<ir::IntConstant>(intType, IntValue(0));
        return createCompareInstruction(ast::BinaryExpression::OpType::kNEqual,
                                        value, zeroValue, astType);
    }

    ir::Instruction* generateLogInstruction(BinaryExpression::OpType opType,
                                            ast::Expression* leftExpr,
                                            ast::Expression* rightExpr,
                                            ast::Type* leftType, ast::Type* rightType) {
        // TODO: handle const
        bool isAnd = (opType == BinaryExpression::OpType::kLogAnd);

        auto rightBB = createBasicBlock("land.rhs");
        auto endBB = createBasicBlock("land.end");

        auto result = m_IRProgram.CreateValue<ir::AllocaInstruction>(
            ir::IntType::Create(m_IRProgram, 1), getValueName("tmp"), m_CurrentBB
        );
        m_CurrentBB->AddInstruction(result);

        auto leftValue = Accept(*leftExpr);
        ir::Instruction* leftCmpInstr = nullptr;
        if (auto cmpInstr = dynamic_cast<ir::CompareInstruction*>(leftValue)) {
            leftCmpInstr = cmpInstr;
        } else {
            leftCmpInstr = generateCompareZeroInstruction(
                                ast::BinaryExpression::OpType::kNEqual,
                                leftValue, leftType);
        }

        auto leftStoreInstr = createStoreInstruction(result, leftCmpInstr);

        auto trueBB = rightBB;
        auto falseBB = endBB;
        if (!isAnd) {
            std::swap(trueBB, falseBB);
        }

        auto branchInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
            leftCmpInstr, trueBB, falseBB, m_CurrentBB
        );

        m_CurrentBB->AddInstruction(leftCmpInstr);
        m_CurrentBB->AddInstruction(leftStoreInstr);
        m_CurrentBB->AddInstruction(branchInstr);

        // Right expression
        m_CurrentBB = rightBB;

        auto rightValue = Accept(*rightExpr);
        ir::Instruction* rightCmpInstr = nullptr;
        if (auto cmpInstr = dynamic_cast<ir::CompareInstruction*>(rightValue)) {
            rightCmpInstr = cmpInstr;
        } else {
            rightCmpInstr = generateCompareZeroInstruction(
                                ast::BinaryExpression::OpType::kNEqual,
                                rightValue, rightType);
        }
 

        auto rightStoreInstr = createStoreInstruction(result, rightCmpInstr);

        auto branchEndInstr = m_IRProgram.CreateValue<ir::BranchInstruction>(
                                endBB, m_CurrentBB);

        m_CurrentBB->AddInstruction(rightCmpInstr);
        m_CurrentBB->AddInstruction(rightStoreInstr);
        m_CurrentBB->AddInstruction(branchEndInstr);
    
        // End
        m_CurrentBB = rightBB;

        auto loadInstr = createLoadInstruction(result, ir::IntType::Create(m_IRProgram, 1));
        m_CurrentBB->AddInstruction(loadInstr);

        return loadInstr;
    }


    ir::Instruction* createCompareInstruction(BinaryExpression::OpType opType,
                                              ir::Value* leftValue, ir::Value* rightValue,
                                              ast::Type* astType) {
        auto builtinType = static_cast<ast::BuiltinType*>(astType);
        bool isSignedInt = builtinType->IsSignedInteger();
        bool isUnsignedInt = builtinType->IsUnsignedInteger();
        bool isFloat = builtinType->IsFloat();

        auto irOpType = ir::CompareInstruction::OpType::kNone;
        switch (opType) {
            case BinaryExpression::OpType::kLess:
                if (isSignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kISLess;
                } else if (isUnsignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kIULess;
                } else if (isFloat) {
                    irOpType = ir::CompareInstruction::OpType::kFLess;
                }
                break;
            case BinaryExpression::OpType::kGreater:
                if (isSignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kISGreater;
                } else if (isUnsignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kIUGreater;
                } else if (isFloat) {
                    irOpType = ir::CompareInstruction::OpType::kFGreater;
                }
                break;
            case BinaryExpression::OpType::kLessEq:
                if (isSignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kISLessEq;
                } else if (isUnsignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kIULessEq;
                } else if (isFloat) {
                    irOpType = ir::CompareInstruction::OpType::kFLessEq;
                }
                break;
            case BinaryExpression::OpType::kGreaterEq:
                if (isSignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kISGreaterEq;
                } else if (isUnsignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kIUGreaterEq;
                } else if (isFloat) {
                    irOpType = ir::CompareInstruction::OpType::kFGreaterEq;
                }
                break;
            case BinaryExpression::OpType::kEqual:
                if (isSignedInt || isUnsignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kIEqual;
                } else if (isFloat) {
                    irOpType = ir::CompareInstruction::OpType::kFEqual;
                }
                break;
            case BinaryExpression::OpType::kNEqual:
                if (isSignedInt || isUnsignedInt) {
                    irOpType = ir::CompareInstruction::OpType::kINEqual;
                } else if (isFloat) {
                    irOpType = ir::CompareInstruction::OpType::kFNEqual;
                }
                break;
        }

        auto cmpInstr = m_IRProgram.CreateValue<ir::CompareInstruction>(
            irOpType, getValueName("cmp"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(cmpInstr);
        return cmpInstr;
    }

    ir::Instruction* generateAdditiveExpression(BinaryExpression::OpType opType,
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

    ir::BinaryInstruction* createMulInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                                ast::Type* type) {
        auto builtinType = dynamic_cast<BuiltinType*>(type);
        
        auto opType = ir::BinaryInstruction::OpType::kMul;
        if (builtinType->IsFloat()) {
            opType = ir::BinaryInstruction::OpType::kFMul;
        }

        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            opType, getValueName("mul"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::BinaryInstruction* createDivInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                                ast::Type* type) {
        auto builtinType = dynamic_cast<BuiltinType*>(type);

        auto opType = ir::BinaryInstruction::OpType::kSDiv;
        if (builtinType->IsFloat()) {
            opType = ir::BinaryInstruction::OpType::kFDiv;
        } else if (builtinType->IsUnsignedInteger()) {
            opType = ir::BinaryInstruction::OpType::kUDiv;
        }

        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            opType, getValueName("div"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::BinaryInstruction* createRemInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                                ast::Type* type) {
        auto builtinType = dynamic_cast<BuiltinType*>(type);

        auto opType = ir::BinaryInstruction::OpType::kSRem;
        if (builtinType->IsFloat()) {
            opType = ir::BinaryInstruction::OpType::kFRem;
        } else if (builtinType->IsUnsignedInteger()) {
            opType = ir::BinaryInstruction::OpType::kURem;
        }

        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            opType, getValueName("rem"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::BinaryInstruction* createShiftLInstruction(ir::Value* leftValue, ir::Value* rightValue) {
        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            ir::BinaryInstruction::OpType::kShiftL, getValueName("shl"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::BinaryInstruction* createShiftRInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                                   ast::Type* type) {
        auto builtinType = dynamic_cast<BuiltinType*>(type);

        auto opType = ir::BinaryInstruction::OpType::kAShiftR;
        auto name = "ashr";
        if (builtinType->IsUnsignedInteger()) {
            opType = ir::BinaryInstruction::OpType::kLShiftR;
            name = "lshr";
        }

        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            opType, getValueName(name),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::BinaryInstruction* createAndInstruction(ir::Value* leftValue, ir::Value* rightValue) {
        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            ir::BinaryInstruction::OpType::kAnd, getValueName("and"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::BinaryInstruction* createXorInstruction(ir::Value* leftValue, ir::Value* rightValue) {
        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            ir::BinaryInstruction::OpType::kXor, getValueName("xor"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::BinaryInstruction* createOrInstruction(ir::Value* leftValue, ir::Value* rightValue) {
        auto instruction = m_IRProgram.CreateValue<ir::BinaryInstruction>(
            ir::BinaryInstruction::OpType::kOr, getValueName("or"),
            leftValue, rightValue, m_CurrentBB
        );
        m_CurrentBB->AddInstruction(instruction);
        return instruction;
    }

    ir::BasicBlock* createBasicBlock(const std::string& name) {
        auto labelType = ir::LabelType::Create(m_IRProgram);
        auto labelName = getLabelName(name);
        auto basicBlock = m_IRProgram.CreateValue<ir::BasicBlock>(name, labelType, m_CurrentFunction);

        m_FunBBMap[name] = basicBlock;
        m_CurrentFunction->AddBasicBlock(basicBlock);

        return basicBlock;
    }

    std::string getValueName(const std::string& name) {
        if (!m_FunValueNames.contains(name)) {
            m_FunValueNames[name] = 1;
            return name;
        }
        auto newName = name;
        newName.append(std::to_string(m_FunValueNames[name]++));
        return newName;
    }

    std::string getLabelName(const std::string& name) {
        if (!m_FunLabelNames.contains(name)) {
            m_FunLabelNames[name] = 1;
            return name;
        }
        auto newName = name;
        newName.append(std::to_string(m_FunLabelNames[name]++));
        return newName;
    }

private:
    void resetFunctionData() {
        m_CurrentFunction = nullptr;
        m_CurrentBB = nullptr;
        m_FunBBMap.clear();
        m_FunValuesMap.clear();
        m_FunLabelNames.clear();
        m_FunValueNames.clear();
    }

private:
    ir::IRProgram& m_IRProgram;

    bool m_InGlobalScope = true;
    bool m_InsideParams = false;

    ir::Function* m_CurrentFunction = nullptr;
    ir::BasicBlock* m_CurrentBB = nullptr;

    std::unordered_map<std::string, ir::BasicBlock*> m_FunBBMap;
    std::unordered_map<std::string, ir::Value*> m_FunValuesMap;

    std::unordered_map<std::string, unsigned int> m_FunLabelNames;
    std::unordered_map<std::string, unsigned int> m_FunValueNames;

    ir::BasicBlock* m_ContinueBB = nullptr;
    ir::BasicBlock* m_BreakBB = nullptr;

    ir::Type* m_IRType = nullptr;
    ir::Value* m_IRValue = nullptr;

    std::vector<ir::Constant*> m_ConstantList;
};

}  // namespace ast
