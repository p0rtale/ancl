#pragma once

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Visitor/AstVisitor.hpp>
#include <Ancl/Visitor/ConstExprAstVisitor.hpp>

#include <Ancl/SymbolTable/Scope.hpp>
#include <Ancl/SymbolTable/SymbolTable.hpp>

#include <Ancl/Grammar/AST/Program.hpp>


namespace ast {

class SemanticAstVisitor: public AstVisitor {
public:
    enum class Status {
        kNone = 0,
        kOk,
        kError,
    };

public:
    SemanticAstVisitor() = default;

    Status Run(Program& program) {
        m_Status = Status::kOk;
        Visit(*program.GetTranslationUnit());

        return m_Status;
    }

private:
    /*
    =================================================================
                                Declaration
    =================================================================
    */

    void Visit(Declaration&) override {
        // Base class
    }

    void Visit(EnumConstDeclaration& enumConstDecl) override {
        auto enumName = enumConstDecl.GetName();
        if (m_CurrentScope->FindSymbol(Scope::NamespaceType::Ident, enumName)) {
            // TODO: handle error
        }
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, enumName, &enumConstDecl);

        auto initExpr = enumConstDecl.GetInit();
        initExpr->Accept(*this);
    }

    void Visit(EnumDeclaration& enumDecl) override {
        auto enumName = enumDecl.GetName();
        if (m_CurrentScope->FindSymbol(Scope::NamespaceType::Tag, enumName)) {
            // TODO: handle error
        }
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Tag, enumName, &enumDecl);

        for (const auto& enumConstDecl : enumDecl.GetEnumerators()) {
            enumConstDecl->Accept(*this);
        }
    }

    void Visit(FieldDeclaration& fieldDecl) override {
        auto fieldName = fieldDecl.GetName();
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, fieldName, &fieldDecl);
    }

    void Visit(FunctionDeclaration& funcDecl) override {
        // TODO: handle declaration/definition, static/extern

        auto body = funcDecl.GetBody();
        if (body) {
            body->Accept(*this);
        }
    }

    void Visit(LabelDeclaration& labelDecl) override {
        auto labelName = labelDecl.GetName();
        if (m_CurrentScope->FindSymbol(Scope::NamespaceType::Label, labelName)) {
            // TODO: handle error
        }
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Label, labelName, &labelDecl);
    }

    void Visit(RecordDeclaration& recordDecl) override {
        auto recordName = recordDecl.GetName();
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Tag, recordName, &recordDecl);

        if (recordDecl.IsDefinition()) {
            // TODO: handle record definition
        } else {  // is declaration
            // TODO: handle record declaration
        }

        auto recordScope = m_SymbolTable.CreateScope("record", m_CurrentScope);
        for (const auto& decl : recordDecl.GetInternalDecls()) {
            if (auto fieldDecl = dynamic_cast<FieldDeclaration*>(decl)) {
                m_CurrentScope = recordScope;
                fieldDecl->Accept(*this);
                m_CurrentScope = m_CurrentScope->GetParentScope();
            } else {
                // internal Tag declaration
                decl->Accept(*this);
            }
        }
    }

    void Visit(TagDeclaration&) override {
        // Base class
    }

    void Visit(TranslationUnit& unit) override {
        for (const auto& decl : unit.GetDeclarations()) {
            decl->Accept(*this);
        }
    }

    void Visit(TypeDeclaration&) override {
        // Base class
    }

    void Visit(TypedefDeclaration& typedefDecl) override {
        auto typedefName = typedefDecl.GetName();
        if (m_CurrentScope->FindSymbol(Scope::NamespaceType::Ident, typedefName)) {
            // TODO: handle error
        }
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, typedefName, &typedefDecl);
    }

    void Visit(ValueDeclaration& valueDecl) override {
        // Base class
    }

    void Visit(VariableDeclaration& varDecl) override {
        auto varName = varDecl.GetName();
        if (m_CurrentScope->FindSymbol(Scope::NamespaceType::Ident, varName)) {
            // TODO: handle error
        }
        m_CurrentScope->AddSymbol(Scope::NamespaceType::Ident, varName, &varDecl);
    }


    /*
    =================================================================
                                Statement
    =================================================================
    */

    void Visit(Statement&) override {
        // Base class
    }

    void Visit(CaseStatement& caseStmt) override {
        // TODO: check for outer switch statement

        auto constExpr = caseStmt.GetExpression();
        constExpr->Accept(*this);

        // TODO: evaluate constant expression
        auto constExprVisitor = ConstExprAstVisitor();
        constExprVisitor.Visit(*constExpr);

        auto body = caseStmt.GetBody();
        body->Accept(*this);
    }

    void Visit(CompoundStatement& compoundStmt) override {
        m_CurrentScope = m_SymbolTable.CreateScope(/*name=*/"", m_CurrentScope);
        for (const auto& stmt : compoundStmt.GetBody()) {
            stmt->Accept(*this);
        }
        m_CurrentScope = m_CurrentScope->GetParentScope();   
    }

    void Visit(DeclStatement& declStmt) override {
        for (const auto& decl : declStmt.GetDeclarations()) {
            decl->Accept(*this);
        }
    }

    void Visit(DefaultStatement& defaultStmt) override {
        // TODO: check for outer switch statement

        auto body = defaultStmt.GetBody();
        body->Accept(*this);
    }

    void Visit(DoStatement& doStmt) override {
        auto cond = doStmt.GetCondition();
        cond->Accept(*this);

        auto body = doStmt.GetBody();
        body->Accept(*this);    
    }

    void Visit(ForStatement& forStmt) override {
        auto init = forStmt.GetInit();
        init->Accept(*this);

        auto cond = forStmt.GetCondition();
        cond->Accept(*this);

        auto step = forStmt.GetStep();
        step->Accept(*this);

        auto body = forStmt.GetBody();
        body->Accept(*this); 
    }

    void Visit(GotoStatement& gotoStmt) override {
        auto labelOldDecl = gotoStmt.GetLabel();
        auto declName = labelOldDecl->GetName();

        auto declOpt = m_CurrentScope->FindSymbol(Scope::NamespaceType::Label, declName);
        if (!declOpt) {
            // TODO: handle error
        }

        auto decl = *declOpt;
        auto labelDecl = dynamic_cast<LabelDeclaration*>(decl);
        if (!labelDecl) {
            // TODO: handle error
        }

        gotoStmt.SetLabel(labelDecl);
    }

    void Visit(IfStatement& ifStmt) override {
        auto cond = ifStmt.GetCondition();
        cond->Accept(*this);

        auto thenStmt = ifStmt.GetThen();
        thenStmt->Accept(*this);

        auto elseStmt = ifStmt.GetThen();
        elseStmt->Accept(*this);
    }

    void Visit(LabelStatement& labelStmt) override {
        auto labelDecl = labelStmt.GetLabel();
        labelDecl->Accept(*this);

        auto body = labelStmt.GetBody();
        body->Accept(*this);
    }

    void Visit(LoopJumpStatement& loopJmpStmt) override {
        // TODO: check for outer loop statement
    }

    void Visit(ReturnStatement& returnStmt) override {
        auto retExpr = returnStmt.GetReturnExpression();
        retExpr->Accept(*this);
    }

    void Visit(SwitchCase& switchCase) override {
        // Base class
    }

    void Visit(SwitchStatement& switchStmt) override {
        auto expr = switchStmt.GetExpression();
        expr->Accept(*this);

        auto body = switchStmt.GetBody();
        body->Accept(*this);
    }

    void Visit(ValueStatement& valueStmt) override {
        // Base class
    }

    void Visit(WhileStatement& whileStmt) override {
        auto cond = whileStmt.GetCondition();
        cond->Accept(*this);

        auto body = whileStmt.GetBody();
        body->Accept(*this);
    }


    /*
    =================================================================
                                Expression
    =================================================================
    */

    void Visit(Expression&) override {
        // Base class
    }

    void Visit(BinaryExpression& binaryExpr) override {
        auto leftOperand = binaryExpr.GetLeftOperand();
        leftOperand->Accept(*this);        

        auto rightOperand = binaryExpr.GetRightOperand();
        rightOperand->Accept(*this);
    }

    void Visit(CallExpression& callExpr) override {
        auto calleeExpr = callExpr.GetCallee();
        calleeExpr->Accept(*this);

        for (const auto& argExpr : callExpr.GetArguments()) {
            argExpr->Accept(*this);
        }
    }

    void Visit(CastExpression& castExpr) override {
        auto subExpr = castExpr.GetSubExpression();
        subExpr->Accept(*this);

        auto toType = castExpr.GetToType();
        toType->Accept(*this);
    }

    // Skip
    void Visit(CharExpression& charExpr) override {}

    void Visit(ConditionalExpression& condExpr) override {
        auto condition = condExpr.GetCondition();
        condition->Accept(*this);

        auto trueExpr = condExpr.GetTrueExpression();
        trueExpr->Accept(*this);

        auto falseExpr = condExpr.GetFalseExpression();
        falseExpr->Accept(*this);
    }

    void Visit(ConstExpression& constExpr) override {
        auto expr = constExpr.GetExpression();
        expr->Accept(*this);
    }

    void Visit(DeclRefExpression& declrefExpr) override {
        auto oldDecl = declrefExpr.GetDeclaration();
        auto declName = oldDecl->GetName();

        auto declOpt = m_CurrentScope->FindSymbol(Scope::NamespaceType::Ident, declName);
        if (!declOpt) {
            // TODO: handle error
        }

        auto decl = *declOpt;
        auto valDecl = dynamic_cast<ValueDeclaration*>(decl);
        if (!valDecl) {
            // TODO: handle error
        }

        declrefExpr.SetDeclaration(valDecl);
    }

    void Visit(ExpressionList& exprList) override {
        for (const auto& expr : exprList.GetExpressions()) {
            expr->Accept(*this);
        }
    }

    // Skip
    void Visit(FloatExpression& floatExpr) override {}

    void Visit(InitializerList& initList) override {
        for (const auto& init : initList.GetInits()) {
            init->Accept(*this);
        }
    }

    // Skip
    void Visit(IntExpression& intExpr) override {}

    // Skip
    void Visit(SizeofTypeExpression& sizeofTypeExpr) override {}

    // Skip
    void Visit(StringExpression& stringExpr) override {}

    void Visit(UnaryExpression& unaryExpr) override {
        auto operand = unaryExpr.GetOperand();
        operand->Accept(*this);
    }


    /*
    =================================================================
                                Type
    =================================================================
    */

    void Visit(ArrayType& arrayType) override {
        // TODO: evaluate constant expression for array size

        auto subType = arrayType.GetSubType();
        subType->Accept(*this);
    }

    // Skip
    void Visit(BuiltinType& builtinType) override {}

    void Visit(EnumType& enumType) override {
        // TODO: check for enum declaration in scope
    }

    void Visit(FunctionType& funcType) override {
        auto retType = funcType.GetSubType();
        retType->Accept(*this);

        for (const auto& paramType : funcType.GetParamTypes()) {
            paramType->Accept(*this);
        }
    }

    void Visit(PointerType& ptrType) override {
        auto subType = ptrType.GetSubType();
        subType->Accept(*this); 
    }

    void Visit(QualType& qualType) override {
        auto subType = qualType.GetSubType();
        subType->Accept(*this);
    }

    void Visit(RecordType& recordType) override {
        // TODO: check for record declaration/definition
        //       and update info
    }

    void Visit(TagType&) override {
        // Base class
    }
    
    void Visit(Type&) override {
        // Base class
    }

    void Visit(TypedefType& typedefType) override {
        auto oldDecl = typedefType.GetDeclaration();
        auto typedefName = oldDecl->GetName();

        auto declOpt = m_CurrentScope->FindSymbol(Scope::NamespaceType::Ident, typedefName);
        if (!declOpt) {
            // TODO: handle error
        }

        auto decl = *declOpt;
        auto typedefDecl = dynamic_cast<TypedefDeclaration*>(decl);
        if (!typedefDecl) {
            // TODO: handle error
        }

        typedefType.SetDeclaration(typedefDecl);
    }

private:
    void handleCompileError() {
        m_Status = Status::kError;
    }

private:
    Status m_Status;

    SymbolTable m_SymbolTable;
    Scope* m_CurrentScope = m_SymbolTable.GetGlobalScope();
};

}  // namespace ast
