#pragma once

#include <stack>
#include <unordered_map>

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Visitor/AstVisitor.hpp>
#include <Ancl/Grammar/AST/ASTProgram.hpp>

#include <Ancl/AnclIR/IR.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

#include <Ancl/AnclIR/Constexpr.hpp>


namespace ast {

class IRGenAstVisitor: public AstVisitor {
public:
    IRGenAstVisitor(ir::IRProgram& irProgram);

    void Run(const ASTProgram& astProgram);

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

    void Visit(FunctionDeclaration& funcDecl) override;
    void Visit(LabelDeclaration& labelDecl) override;

    void Visit(ParameterDeclaration& paramDecl) override;

    // Skip
    void Visit(RecordDeclaration& recordDecl) override {}

    void Visit(TagDeclaration&) override {
        // Base class
    }

    void Visit(TranslationUnit& unit) override;

    void Visit(TypeDeclaration&) override {
        // Base class
    }

    // Skip
    void Visit(TypedefDeclaration& typedefDecl) override {}

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

    // TODO: template Visitor
    ir::Value* Accept(Expression& expr) {
        expr.Accept(*this);  // -> m_IRValue
        return m_IRValue;
    }

    void Visit(Expression&) override {
        // Base class
    }

    void Visit(BinaryExpression& binaryExpr) override;
    void Visit(CallExpression& callExpr) override;
    void Visit(CastExpression& castExpr) override;
    void Visit(CharExpression& charExpr) override;
    void Visit(ConditionalExpression& condExpr) override;
    void Visit(ConstExpression& constExpr) override;
    void Visit(DeclRefExpression& declrefExpr) override;
    void Visit(ExpressionList& exprList) override;
    void Visit(FloatExpression& floatExpr) override;

    std::vector<ir::Constant*> AcceptConst(InitializerList& initList) {
        m_ConstantList.clear();
        // TODO: ...
        initList.Accept(*this);  // -> m_ConstantList
        return m_ConstantList;
    }

    std::vector<ir::Value*> AcceptInitList(InitializerList& initList) {
        m_ValueList.clear();
        initList.Accept(*this);  // -> m_ValueList
        return m_ValueList;
    }

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

    // TODO: template Visitor
    ir::Type* Accept(Type& type) {
        type.Accept(*this);  // -> m_IRType
        return m_IRType;
    }

    void Visit(ArrayType& arrayType) override;
    void Visit(BuiltinType& builtinType) override;
    void Visit(EnumType& enumType) override;
    void Visit(FunctionType& funcType) override;
    void Visit(PointerType& ptrType) override;

    ir::Type* VisitQualType(QualType qualType);

    void Visit(RecordType& recordType) override;

    void Visit(TagType&) override {
        // Base class
    }
    
    void Visit(Type&) override {
        // Base class
    }

    void Visit(TypedefType& typedefType) override;

private:
    ir::Value* createAddInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                    ast::Type* type);

    ir::Value* createSubInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                    ast::Type* type);

    ir::Value* generateIncDecExpression(UnaryExpression::OpType opType, ir::Value* value,
                                        QualType qualType);

    ir::BinaryInstruction* generatePtrSubExpression(ir::Value* leftValue, ir::Value* rightValue);

    ir::CastInstruction* generateExtCast(ir::Value* fromValue, ast::Type* fromASTType, ir::Type* toIRType);

    ir::Value* generateNotExpression(ir::Value* value);

    ir::Value* generateLogNotExpression(ir::Value* operandValue, ast::Type* exprType);

    ir::Value* generateSizeofExpression(ir::Value* operandValue);

    ir::Value* generateNegExpression(ir::Value* value);

    ir::Instruction* generateStructMemberExpression(ir::Value* structValue,
                                                    ast::DeclRefExpression* memberExpr,
                                                    ast::Type* astType);

    ir::Value* generateCompoundAssignment(BinaryExpression::OpType opType,
                                          Expression* leftOperand, Expression* rightOperand,
                                          ast::Type* resultType);

    ir::Value* generateDerefExpression(ir::Value* operandValue);

    ir::Instruction* generateArrToPointerDecay(ir::Value* ptrValue);

    ir::Instruction* generateArrMemberExpression(ir::Value* ptrValue, ir::Value* intValue,
                                                 ast::Type* intType);

    ir::Instruction* generatePtrAddExpression(bool isAdd, ir::Value* ptrValue, ir::Value* intValue,
                                              ast::Type* intType = nullptr);

    ir::MemoryCopyInstruction* createMemoryCopyInstruction(ir::Value* destination, ir::Value* source,
                                                           size_t size);

    ir::StoreInstruction* createStoreInstruction(ir::Value* value, ir::Value* address,
                                                 bool isVolatile = false);

    ir::LoadInstruction* createLoadInstruction(ir::Value* fromPointer, ir::Type* toType,
                                               bool isVolatile = false);

    ir::CastInstruction* createCastInstruction(ir::CastInstruction::OpType opType,
                                               ir::Value* fromValue, ir::Type* toType);

    ir::Instruction* generateCompareZeroInstruction(BinaryExpression::OpType opType,
                                                  ir::Value* value, ast::Type* astType);

    ir::Value* generateLogInstruction(BinaryExpression::OpType opType,
                                      ast::Expression* leftExpr,
                                      ast::Expression* rightExpr,
                                      ast::Type* leftType, ast::Type* rightType);


    ir::Instruction* createCompareInstruction(BinaryExpression::OpType opType,
                                              ir::Value* leftValue, ir::Value* rightValue,
                                              ast::Type* astType);

    ir::Value* generateAdditiveExpression(BinaryExpression::OpType opType,
                                          ir::Value* leftValue, ir::Value* rightValue,
                                          ast::Type* leftType, ast::Type* rightType,
                                          ast::Type* resultType);

    ir::Value* createMulInstruction(ir::Value* leftValue, ir::Value* rightValue, ast::Type* type);

    ir::Value* createDivInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                    ast::Type* type);

    ir::Value* createRemInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                    ast::Type* type);

    ir::Value* createShiftLInstruction(ir::Value* leftValue, ir::Value* rightValue);

    ir::Value* createShiftRInstruction(ir::Value* leftValue, ir::Value* rightValue,
                                       ast::Type* type);

    ir::Value* createAndInstruction(ir::Value* leftValue, ir::Value* rightValue);

    ir::Value* createXorInstruction(ir::Value* leftValue, ir::Value* rightValue);

    ir::Value* createOrInstruction(ir::Value* leftValue, ir::Value* rightValue);

    ir::BasicBlock* createBasicBlock(const std::string& name);

    ir::Constant* getNumberIRConstant(ir::Value* value);

private:
    void generateAllocas();
    void resetFunctionData();

private:
    ir::IRProgram& m_IRProgram;

    ir::Constexpr m_Constexpr;

    bool m_IsConstVar = false;
    std::vector<ir::BasicBlock*> m_ReturnBlocks;

    ir::Function* m_CurrentFunction = nullptr;
    ir::BasicBlock* m_CurrentBB = nullptr;

    std::unordered_map<std::string, ir::BasicBlock*> m_FunBBMap;

    std::unordered_map<Declaration*, ir::AllocaInstruction*> m_AllocasMap;

    std::stack<ir::AllocaInstruction*> m_AllocaBuffer;

    std::unordered_map<std::string, ir::GlobalVariable*> m_StringLabelsMap;

    std::stack<ir::BasicBlock*> m_ContinueBBStack;
    std::stack<ir::BasicBlock*> m_BreakBBStack;

    ir::Type* m_IRType = nullptr;
    ir::Value* m_IRValue = nullptr;

    bool m_IsVolatileType = false;
    bool m_IsRestrictType = false;

    std::vector<ir::Constant*> m_ConstantList;
    std::vector<ir::Value*> m_ValueList;
};

}  // namespace ast
