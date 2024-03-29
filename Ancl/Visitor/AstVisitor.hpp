#pragma once

#include <Ancl/Grammar/AST/ForwardDecl.hpp>


namespace ast {

class AstVisitor {
public:
    /*
    =================================================================
                                Declaration
    =================================================================
    */

    virtual void Visit(Declaration&) = 0;
    virtual void Visit(EnumConstDeclaration&) = 0;
    virtual void Visit(EnumDeclaration&) = 0;
    virtual void Visit(FieldDeclaration&) = 0;
    virtual void Visit(FunctionDeclaration&) = 0;
    virtual void Visit(LabelDeclaration&) = 0;
    virtual void Visit(RecordDeclaration&) = 0;
    virtual void Visit(TagDeclaration&) = 0;
    virtual void Visit(TranslationUnit&) = 0;
    virtual void Visit(TypeDeclaration&) = 0;
    virtual void Visit(TypedefDeclaration&) = 0;
    virtual void Visit(ValueDeclaration&) = 0;
    virtual void Visit(VariableDeclaration&) = 0;


    /*
    =================================================================
                                Statement
    =================================================================
    */

    virtual void Visit(Statement&) = 0;
    virtual void Visit(CaseStatement&) = 0;
    virtual void Visit(CompoundStatement&) = 0;
    virtual void Visit(DeclStatement&) = 0;
    virtual void Visit(DefaultStatement&) = 0;
    virtual void Visit(DoStatement&) = 0;
    virtual void Visit(ForStatement&) = 0;
    virtual void Visit(GotoStatement&) = 0;
    virtual void Visit(IfStatement&) = 0;
    virtual void Visit(LabelStatement&) = 0;
    virtual void Visit(LoopJumpStatement&) = 0;
    virtual void Visit(ReturnStatement&) = 0;
    virtual void Visit(SwitchCase&) = 0;
    virtual void Visit(SwitchStatement&) = 0;
    virtual void Visit(ValueStatement&) = 0;
    virtual void Visit(WhileStatement&) = 0;


    /*
    =================================================================
                                Expression
    =================================================================
    */

    virtual void Visit(Expression&) = 0;
    virtual void Visit(BinaryExpression&) = 0;
    virtual void Visit(CallExpression&) = 0;
    virtual void Visit(CastExpression&) = 0;
    virtual void Visit(CharExpression&) = 0;
    virtual void Visit(ConditionalExpression&) = 0;
    virtual void Visit(ConstExpression&) = 0;
    virtual void Visit(DeclRefExpression&) = 0;
    virtual void Visit(ExpressionList&) = 0;
    virtual void Visit(FloatExpression&) = 0;
    virtual void Visit(InitializerList&) = 0;
    virtual void Visit(IntExpression&) = 0;
    virtual void Visit(SizeofTypeExpression&) = 0;
    virtual void Visit(StringExpression&) = 0;
    virtual void Visit(UnaryExpression&) = 0;
};

}  // namespace ast
