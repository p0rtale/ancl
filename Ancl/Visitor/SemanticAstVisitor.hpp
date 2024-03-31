#pragma once

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Visitor/AstVisitor.hpp>

#include <Ancl/SymbolTable/Scope.hpp>
#include <Ancl/SymbolTable/SymbolTable.hpp>


namespace ast {

class SemanticAstVisitor: public AstVisitor {
public:
    SemanticAstVisitor() = default;

public:
    /*
    =================================================================
                                Declaration
    =================================================================
    */

    void Visit(Declaration&) override {
        // Base class
    }

    void Visit(EnumConstDeclaration& enumConstDecl) override {}

    void Visit(EnumDeclaration& enumDecl) override {}

    void Visit(FieldDeclaration& fieldDecl) override {}

    void Visit(FunctionDeclaration& funcDecl) override {}

    void Visit(LabelDeclaration& labelDecl) override {}

    void Visit(RecordDeclaration& recordDecl) override {}

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

    void Visit(TypedefDeclaration& typedefDecl) override {}

    void Visit(ValueDeclaration& valueDecl) override {
        // Base class
    }

    void Visit(VariableDeclaration& varDecl) override {}


    /*
    =================================================================
                                Statement
    =================================================================
    */

    void Visit(Statement&) override {
        // Base class
    }

    void Visit(CaseStatement& caseStmt) override {
        auto constExpr = caseStmt.GetExpression();
        

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

    void Visit(DefaultStatement& defaultStmt) override {}

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

    void Visit(GotoStatement& gotoStmt) override {}

    void Visit(IfStatement& ifStmt) override {
        auto cond = ifStmt.GetCondition();
        cond->Accept(*this);

        auto thenStmt = ifStmt.GetThen();
        thenStmt->Accept(*this);

        auto elseStmt = ifStmt.GetThen();
        elseStmt->Accept(*this);
    }

    void Visit(LabelStatement& labelStmt) override {}

    void Visit(LoopJumpStatement& loopJmpStmt) override {}

    void Visit(ReturnStatement& returnStmt) override {}

    void Visit(SwitchCase& switchCase) override {
        // Base class
    }

    void Visit(SwitchStatement& switchStmt) override {}

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

    void Visit(BinaryExpression& binaryExpr) override {}

    void Visit(CallExpression& callExpr) override {}

    void Visit(CastExpression& castExpr) override {}

    void Visit(CharExpression& charExpr) override {}

    void Visit(ConditionalExpression& condExpr) override {}

    void Visit(ConstExpression& constExpr) override {}

    void Visit(DeclRefExpression& declrefExpr) override {}

    void Visit(ExpressionList& exprList) override {}

    void Visit(FloatExpression& floatExpr) override {}

    void Visit(InitializerList& initList) override {}

    void Visit(IntExpression& intExpr) override {}

    void Visit(SizeofTypeExpression& sizeofTypeExpr) override {}

    void Visit(StringExpression& stringExpr) override {}

    void Visit(UnaryExpression& unaryExpr) override {}


    /*
    =================================================================
                                Type
    =================================================================
    */

    void Visit(ArrayType& arrayType) override {}

    void Visit(BuiltinType& builtinType) override {}

    void Visit(EnumType& enumType) override {}

    void Visit(FunctionType& funcType) override {}

    void Visit(PointerType& ptrType) override {}

    void Visit(QualType& qualType) override {}

    void Visit(RecordType& recordType) override {}

    void Visit(TagType&) override {
        // Base class
    }
    
    void Visit(Type&) override {
        // Base class
    }

    void Visit(TypedefType& typedefType) override {}

private:
    SymbolTable m_SymbolTable;
    Scope* m_CurrentScope = m_SymbolTable.GetGlobalScope();
};

}  // namespace ast
