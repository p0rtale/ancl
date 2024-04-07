#pragma once

#include <unordered_map>

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

    // Skip
    void Visit(FieldDeclaration& fieldDecl) override {}

    void Visit(FunctionDeclaration& funcDecl) override {
        auto qualType = funcDecl.GetType();
        auto funcIRType = dynamic_cast<ir::FunctionType*>(Accept(*qualType));
        if (!funcIRType) {
            // TODO: handle error
        }

        auto storageClass = funcDecl.GetStorageClass();
        auto linkage = ir::GlobalValue::LinkageType::kExtern;
        if (storageClass == StorageClass::kStatic) {
            linkage = ir::GlobalValue::LinkageType::kStatic;
        }

        m_IRProgram.CreateValue<ir::Function>(funcIRType, linkage, funcDecl.GetName());

        auto body = funcDecl.GetBody();
        if (body) {
            body->Accept(*this);
        }
    }

    void Visit(LabelDeclaration& labelDecl) override {
        auto nextBB = createBasicBlock(labelDecl.GetName());

        auto voidType = m_IRProgram.CreateType<ir::VoidType>();
        auto branch = m_IRProgram.CreateValue<ir::BranchInstruction>(nextBB, voidType, m_CurrentBB);
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
        auto alloca = m_IRProgram.CreateValue<ir::AllocaInstruction>(varIRType, name, m_CurrentBB);
        m_CurrentBB->AddInstruction(alloca);
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
        auto voidType = m_IRProgram.CreateType<ir::VoidType>();
        auto bodyBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(bodyBB, voidType, m_CurrentBB);
        m_CurrentBB->AddInstruction(bodyBranch);

        // Body
        m_CurrentBB = bodyBB;
        auto body = doStmt.GetBody();
        body->Accept(*this);
        auto loopBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, voidType, m_CurrentBB);
        m_CurrentBB->AddInstruction(loopBranch);

        // Condition
        m_CurrentBB = condBB;
        auto condValue = Accept(*doStmt.GetCondition());
        auto condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, bodyBB, endBB, voidType, m_CurrentBB);
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

        auto voidType = m_IRProgram.CreateType<ir::VoidType>();
        auto entryBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, voidType, m_CurrentBB);
        m_CurrentBB->AddInstruction(entryBranch);

        // Condition
        m_CurrentBB = condBB;
        if (forCondition) {
            auto condValue = Accept(*forCondition);
            auto condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, bodyBB, endBB, voidType, m_CurrentBB);
            m_CurrentBB->AddInstruction(condBranch);
            m_CurrentBB = bodyBB;
        }

        // Body
        auto bodyStmt = forStmt.GetBody();
        bodyStmt->Accept(*this);

        if (stepBB) {
            auto stepBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(stepBB, voidType, m_CurrentBB);
            m_CurrentBB->AddInstruction(stepBranch);

            // Step
            m_CurrentBB = stepBB;
            stepExpr->Accept(*this);  // Ignore result value
        }
        auto condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, voidType, m_CurrentBB);
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
        auto condValue = Accept(*ifStmt.GetCondition());
        auto voidType = m_IRProgram.CreateType<ir::VoidType>();
        ir::BranchInstruction* condBranch = nullptr;
        if (elseBB) {
            condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, thenBB, elseBB, voidType, m_CurrentBB);
        } else {
            condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, thenBB, endBB, voidType, m_CurrentBB);
        }
        m_CurrentBB->AddInstruction(condBranch);

        // Then
        m_CurrentBB = thenBB;

        auto thenStmt = ifStmt.GetThen();
        thenStmt->Accept(*this);

        auto endBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(endBB, voidType, m_CurrentBB);
        m_CurrentBB->AddInstruction(endBranch);

        // Else
        if (elseBB) {
            m_CurrentBB = elseBB;

            elseStmt->Accept(*this);

            auto endBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(endBB, voidType, m_CurrentBB);
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
        auto condBB = createBasicBlock("while.cond");
        auto bodyBB = createBasicBlock("while.body");
        auto endBB = createBasicBlock("while.end");

        // Entry
        auto voidType = m_IRProgram.CreateType<ir::VoidType>();
        auto entryBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, voidType, m_CurrentBB);
        m_CurrentBB->AddInstruction(entryBranch);

        // Condition
        m_CurrentBB = condBB;
        auto condValue = Accept(*whileStmt.GetCondition());
        auto condBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condValue, bodyBB, endBB, voidType, m_CurrentBB);
        m_CurrentBB->AddInstruction(condBranch);

        // Body
        m_CurrentBB = bodyBB;
        auto body = whileStmt.GetBody();
        body->Accept(*this);
        auto loopBranch = m_IRProgram.CreateValue<ir::BranchInstruction>(condBB, voidType, m_CurrentBB);
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
    ir::BasicBlock* createBasicBlock(const std::string& name) {
        auto labelType = m_IRProgram.CreateType<ir::LabelType>();
        auto labelName = getLabelName(name);
        auto basicBlock = m_IRProgram.CreateValue<ir::BasicBlock>(name, labelType, m_CurrentFunction);

        m_FunBBMap[name] = basicBlock;
        m_CurrentFunction->AddBasicBlock(basicBlock);

        return basicBlock;
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
    ir::IRProgram& m_IRProgram;

    ir::Function* m_CurrentFunction = nullptr;
    std::unordered_map<std::string, ir::BasicBlock*> m_FunBBMap;
    std::unordered_map<std::string, unsigned int> m_FunLabelNames;
    ir::BasicBlock* m_CurrentBB = nullptr;

    std::unordered_map<std::string, ir::Value*> m_FunValuesMap;

    ir::Type* m_IRType = nullptr;
    ir::Value* m_IRValue = nullptr;
};

}  // namespace ast
