#pragma once

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Visitor/AstVisitor.hpp>


namespace ast {

class ConstExprAstVisitor: public AstVisitor {
public:
    ConstExprAstVisitor() = default;

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

    void Visit(TranslationUnit& unit) override {}

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

    void Visit(SwitchCase& switchCase) override {
        // Base class
    }

    void Visit(SwitchStatement& switchStmt) override {}

    void Visit(ValueStatement& valueStmt) override {
        // Base class
    }

    void Visit(WhileStatement& whileStmt) override {}


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

    void Visit(ConstExpression& constExpr) override {
        
    }

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

};

}  // namespace ast
