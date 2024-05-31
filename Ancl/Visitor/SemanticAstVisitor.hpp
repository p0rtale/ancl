#pragma once

#include <optional>

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Visitor/AstVisitor.hpp>

#include <Ancl/SymbolTable/Scope.hpp>
#include <Ancl/SymbolTable/SymbolTable.hpp>
#include <Ancl/SymbolTable/DotConverter.hpp>

#include <Ancl/Grammar/AST/ASTProgram.hpp>

#include <Ancl/Logger/Logger.hpp>


namespace ast {

class SemanticAstVisitor: public AstVisitor {
public:
    enum class Status {
        kNone = 0,
        kOk,
        kError,
    };

public:
    SemanticAstVisitor(ASTProgram& program)
        : m_Program(program) {}

    Status Run() {
        m_Status = Status::kOk;
        Visit(*m_Program.GetTranslationUnit());

        return m_Status;
    }

    void PrintScopeInfoDot(const std::string& filename) {
        SymbolTreeDotConverter dotConverter(m_SymbolTable, filename);
        dotConverter.Convert();
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

    void Visit(EnumConstDeclaration& enumConstDecl) override;
    void Visit(EnumDeclaration& enumDecl) override;
    void Visit(FieldDeclaration& fieldDecl) override;
    void Visit(FunctionDeclaration& funcDecl) override;
    void Visit(LabelDeclaration& labelDecl) override;
    void Visit(ParameterDeclaration& paramDecl) override;
    void Visit(RecordDeclaration& recordDecl) override;

    void Visit(TagDeclaration&) override {
        // Base class
    }

    void Visit(TranslationUnit& unit) override;

    void Visit(TypeDeclaration&) override {
        // Base class
    }

    void Visit(TypedefDeclaration& typedefDecl) override;

    void Visit(ValueDeclaration& valueDecl) override {
        // Base class
    }

    void Visit(VariableDeclaration& varDecl) override;


    /*
    =================================================================
                                Statement
    =================================================================
    */

    void Visit(Statement&) override {
        // Base class
    }

    void Visit(CaseStatement& caseStmt) override;
    void Visit(CompoundStatement& compoundStmt) override;
    void Visit(DeclStatement& declStmt) override;
    void Visit(DefaultStatement& defaultStmt) override;
    void Visit(DoStatement& doStmt) override;
    void Visit(ForStatement& forStmt) override;
    void Visit(GotoStatement& gotoStmt) override;
    void Visit(IfStatement& ifStmt) override;
    void Visit(LabelStatement& labelStmt) override;
    void Visit(LoopJumpStatement& loopJmpStmt) override;
    void Visit(ReturnStatement& returnStmt) override;

    void Visit(SwitchCase& switchCase) override {
        // Base class
    }

    void Visit(SwitchStatement& switchStmt) override;

    void Visit(ValueStatement& valueStmt) override {
        // Base class
    }

    void Visit(WhileStatement& whileStmt) override;


    /*
    =================================================================
                                Expression
    =================================================================
    */

    void Visit(Expression&) override {
        // Base class
    }

    void VisitAssignmentExpression(BinaryExpression& assignExpr);
    void VisitArrSubscriptExpression(BinaryExpression& arrExpr);
    void VisitMemberExpression(BinaryExpression& memberExpr);

    void Visit(BinaryExpression& binaryExpr) override;
    void Visit(CallExpression& callExpr) override;
    void Visit(CastExpression& castExpr) override;
    void Visit(CharExpression& charExpr) override;
    void Visit(ConditionalExpression& condExpr) override;
    void Visit(ConstExpression& constExpr) override;
    void Visit(DeclRefExpression& declrefExpr) override;
    void Visit(ExpressionList& exprList) override;
    void Visit(FloatExpression& floatExpr) override;
    void Visit(InitializerList& initList) override;
    void Visit(IntExpression& intExpr) override;
    void Visit(SizeofTypeExpression& sizeofTypeExpr) override;
    void Visit(StringExpression& stringExpr) override;
    void Visit(UnaryExpression& unaryExpr) override;


    /*
    =================================================================
                                Type
    =================================================================
    */

    void Visit(ArrayType& arrayType) override;

    // Skip
    void Visit(BuiltinType& builtinType) override {}

    void Visit(EnumType& enumType) override;
    void Visit(FunctionType& funcType) override;
    void Visit(PointerType& ptrType) override;
    void Visit(RecordType& recordType) override;

    void Visit(TagType&) override {
        // Base class
    }
    
    void Visit(Type&) override {
        // Base class
    }

    void Visit(TypedefType& typedefType) override;

private:
    bool checkArrayInitialization(QualType arrayQualType, Expression* initExpr);

    std::optional<QualType> decayType(QualType qualType);

    bool areCompatibleTypes(QualType leftQualType, QualType rightQualType, bool isPointer);

    QualType promoteIntegerType(QualType qualType);
    BuiltinType::Kind promoteIntegerKind(BuiltinType::Kind kind);

    QualType getCommonRealType(QualType leftQualType,
                               QualType rightQualType);

    BuiltinType::Kind getCommonRealTypeKind(BuiltinType::Kind leftKind,
                                            BuiltinType::Kind rightKind);

    bool isIncompleteType(QualType qualType);
    bool isPointerType(QualType qualType);
    bool isPointerToFunctionType(QualType qualType);
    bool isPointerToIncompleteType(QualType qualType);
    bool isVoidType(QualType qualType);
    bool isPointerToVoidType(QualType qualType);
    bool isNullPointerConstant(Expression* expr);
    bool isIntegerType(QualType qualType);
    bool isFloatType(QualType qualType);
    bool isRecordType(QualType qualType);
    bool isRealType(QualType qualType);
    bool isScalarType(QualType qualType);
    bool isModifiableLValue(Expression& expr);

    void handleTagDeclaration(TagDeclaration* decl, const std::string& name,
                              TagDeclaration* scopeDecl);

private:
    void printSemanticWarning(const std::string& text, const Location& location) {
        ANCL_WARN("{} {}", location.ToString(), text);
    }

    void printSemanticError(const std::string& text, const Location& location) {
        ANCL_ERROR("{} {}", location.ToString(), text);

        // TODO: Handle error
        m_Status = Status::kError;
        exit(EXIT_FAILURE);
    }

private:
    ASTProgram& m_Program;
    Status m_Status;

    SymbolTable m_SymbolTable;
    Scope* m_CurrentScope = m_SymbolTable.GetGlobalScope();

    FunctionDeclaration* m_CurrentFunctionDecl = nullptr;
    Scope* m_FunctionScope = nullptr;

    // TODO: Simplify
    bool m_IgnoreCompoundScope = false;
    
    bool m_InsideLoop = false;
    bool m_InsideSwitch = false;

    bool m_HasReturn = false;
};

}  // namespace ast
