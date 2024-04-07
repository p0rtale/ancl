#pragma once

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Visitor/AstVisitor.hpp>
#include <Ancl/Grammar/AST/ASTProgram.hpp>

#include <Ancl/AnclIR/IR.hpp>
#include <Ancl/AnclIR/IRProgram.hpp>

#include <Ancl/AnclIR/DataLayout/Alignment.hpp>


namespace ast {

class IRGenAstVisitor: public AstVisitor {
public:
    IRGenAstVisitor(ir::IRProgram& irProgram): m_IRProgram(irProgram) {}

    void Run(const ASTProgram& astProgram) {
        Visit(*astProgram.GetTranslationUnit());
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

    void Visit(FieldDeclaration& fieldDecl) override {}

    void Visit(FunctionDeclaration& funcDecl) override {
        auto type = funcDecl.GetType();
        auto funcType = dynamic_cast<FunctionType*>(type);
        if (!funcType) {
            // TODO: handle error
        }

        auto returnSubType = funcType->GetSubType();

        bool isVariadic = funcType->IsVariadic();

        // m_IRProgram.CreateType<ir::FunctionType>();

        // m_IRProgram.CreateValue<ir::Function>();

        auto body = funcDecl.GetBody();
        if (body) {
            body->Accept(*this);
        }
    }

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

    // Skip
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

    // Skip
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
                m_IRType = m_IRProgram.CreateType<ir::IntType>(1);
                break;

            case BuiltinType::Kind::kShort:
            case BuiltinType::Kind::kUShort:
                m_IRType = m_IRProgram.CreateType<ir::IntType>(2);
                break;

            case BuiltinType::Kind::kInt:
            case BuiltinType::Kind::kUInt:
                m_IRType = m_IRProgram.CreateType<ir::IntType>(4);
                break;

            case BuiltinType::Kind::kLong:
            case BuiltinType::Kind::kULong:
                m_IRType = m_IRProgram.CreateType<ir::IntType>(8);
                break;

            case BuiltinType::Kind::kFloat:
                m_IRType = m_IRProgram.CreateType<ir::FloatType>(ir::FloatType::Kind::kFloat);
                break;

            case BuiltinType::Kind::kDouble:  
                m_IRType = m_IRProgram.CreateType<ir::FloatType>(ir::FloatType::Kind::kDouble);
                break;

            case BuiltinType::Kind::kLongDouble:  
                m_IRType = m_IRProgram.CreateType<ir::FloatType>(ir::FloatType::Kind::kLongDouble);
                break;

            default:
                // TODO: handle error
                break;
        }
    }

    void Visit(EnumType& enumType) override {
        m_IRType = m_IRProgram.CreateType<ir::IntType>(32);
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
        m_IRType = m_IRProgram.CreateType<ir::PointerType>(elementType);
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
            m_IRType = m_IRProgram.CreateType<ir::StructType>(elementTypes);
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
            auto byteType = m_IRProgram.CreateType<ir::IntType>(1);
            auto restBytesType = m_IRProgram.CreateType<ir::ArrayType>(byteType, unionSize - maxSize);
            unionElemTypes.push_back(restBytesType);
        }

        m_IRType = m_IRProgram.CreateType<ir::StructType>(unionElemTypes);
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
    ir::IRProgram& m_IRProgram;

    ir::Type* m_IRType = nullptr;
};

}  // namespace ast
