#pragma once

#include <fstream>
#include <format>

#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Visitor/AstVisitor.hpp>


namespace ast {

class AstDotVisitor: public AstVisitor {
public:
    AstDotVisitor(const std::string& filename): m_OutputStream(filename) {}

public:
    /*
    =================================================================
                                Declaration
    =================================================================
    */

    void Visit(Declaration&) override {
        // Base class
    }

    void Visit(EnumConstDeclaration& enumConstDecl) override {
        printNode("enumconst_declaration",
                  std::format("EnumConstDecl [\\\"{}\\\"]",
                              enumConstDecl.GetName()),
                  enumConstDecl.GetLocation().ToString());

        if (enumConstDecl.HasInit()) {
            acceptNode(enumConstDecl.GetInit(), "Init");
        }
    }

    void Visit(EnumDeclaration& enumDecl) override {
        printNode("enum_declaration",
                  std::format("EnumDecl [\\\"{}\\\"]",
                              enumDecl.GetName()),
                  enumDecl.GetLocation().ToString());

        acceptNodeList(enumDecl.GetEnumerators(), "Enumerator");
    }

    void Visit(FieldDeclaration& fieldDecl) override {
        printNode("field_declaration",
                  std::format("FieldDecl [\\\"{}\\\"]",
                              fieldDecl.GetName()),
                  fieldDecl.GetLocation().ToString());

        acceptQualType(fieldDecl.GetType(), "Type");
    }

    void Visit(FunctionDeclaration& funcDecl) override {
        std::string traitsStr;
        if (funcDecl.IsInline()) {
            traitsStr.append(",inline");
        }
        auto storageClass = funcDecl.GetStorageClass();
        switch (storageClass) {
        case StorageClass::kExtern:
            traitsStr.append(",extern");
            break;
        case StorageClass::kStatic:
            traitsStr.append(",static");
            break;
        case StorageClass::kAuto:
            traitsStr.append(",auto");
            break;
        case StorageClass::kRegister:
            traitsStr.append(",register");
            break;
        }

        printNode("function_declaration",
                  std::format("FunctionDecl [\\\"{}\\\"{}]",
                              funcDecl.GetName(), traitsStr),
                  funcDecl.GetLocation().ToString());
    
        auto body = funcDecl.GetBody();
        if (body) {
            acceptNode(body, "Body");
        }
        
        acceptNodeList(funcDecl.GetParams(), "ParamDecl");
        acceptQualType(funcDecl.GetType(), "Type");
    }

    void Visit(LabelDeclaration& labelDecl) override {
        printNode("label_declaration",
                  std::format("LabelDecl [\\\"{}\\\"]",
                              labelDecl.GetName()),
                  labelDecl.GetLocation().ToString());

        acceptNode(labelDecl.GetStatement(), "Statement");
    }

    void Visit(ParameterDeclaration& paramDecl) override {
        std::string traitsStr;
        auto storageClass = paramDecl.GetStorageClass();
        switch (storageClass) {
        case StorageClass::kAuto:
            traitsStr.append(",auto");
            break;
        case StorageClass::kRegister:
            traitsStr.append(",register");
            break;
        }

        printNode("parameter_declaration",
                  std::format("ParameterDecl [\\\"{}\\\"{}]",
                              paramDecl.GetName(), traitsStr),
                  paramDecl.GetLocation().ToString());

        acceptQualType(paramDecl.GetType(), "Type");
    }

    void Visit(RecordDeclaration& recordDecl) override {
        std::string traitsStr = ",struct";
        if (recordDecl.IsUnion()) {
            traitsStr = ",union";
        }
        printNode("record_declaration",
                  std::format("RecordDecl [\\\"{}\\\"{}]",
                              recordDecl.GetName(), traitsStr),
                  recordDecl.GetLocation().ToString());

        acceptNodeList(recordDecl.GetInternalDecls(), "InternalDecl");
    }

    void Visit(TagDeclaration&) override {
        // Base class
    }

    void Visit(TranslationUnit& unit) override {
        printWithSpaces(std::format("digraph \"{}\" {{\n", m_GraphName));

        m_CurrentNodeIdent = std::string{"translation_unit"};

        spaceLevelUp();
        printWithSpaces(std::format("{} [\n", m_CurrentNodeIdent));

        spaceLevelUp();
        printNodeAttributes("TranslationUnit");
        spaceLevelDown();

        printWithSpaces("];\n");

        acceptNodeList(unit.GetDeclarations(), "Decl");
        spaceLevelDown();

        printWithSpaces("}\n");
    }

    void Visit(TypeDeclaration&) override {
        // Base class
    }

    void Visit(TypedefDeclaration& typedefDecl) override {
        printNode("typedef_declaration",
                  std::format("TypedefDecl [\\\"{}\\\"]",
                              typedefDecl.GetName()),
                  typedefDecl.GetLocation().ToString());

        acceptQualType(typedefDecl.GetType(), "Type");
    }

    void Visit(ValueDeclaration& valueDecl) override {
        // (Base class)
        printNode("value_declaration",
                  std::format("ValueDecl [\\\"{}\\\"]",
                              valueDecl.GetName()),
                  valueDecl.GetLocation().ToString());
    }

    void Visit(VariableDeclaration& varDecl) override {
        std::string traitsStr;
        auto storageClass = varDecl.GetStorageClass();
        switch (storageClass) {
        case StorageClass::kExtern:
            traitsStr.append(",extern");
            break;
        case StorageClass::kStatic:
            traitsStr.append(",static");
            break;
        case StorageClass::kAuto:
            traitsStr.append(",auto");
            break;
        case StorageClass::kRegister:
            traitsStr.append(",register");
            break;
        }

        printNode("variable_declaration",
                  std::format("VariableDecl [\\\"{}\\\"{}]",
                              varDecl.GetName(), traitsStr),
                  varDecl.GetLocation().ToString());

        acceptQualType(varDecl.GetType(), "Type");

        auto initExpr = varDecl.GetInit();
        if (initExpr) {
            acceptNode(initExpr, "Init");
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

    void Visit(CaseStatement& caseStmt) override {
        printNode("case_statement", "CaseStmt",
                  caseStmt.GetLocation().ToString());

        acceptNode(caseStmt.GetExpression(), "ConstExpr");
        acceptNode(caseStmt.GetBody(), "Body");
    }

    void Visit(CompoundStatement& compoundStmt) override {
        printNode("compound_statement", "CompoundStmt",
                  compoundStmt.GetLocation().ToString());

        acceptNodeList(compoundStmt.GetBody(), "Stmt");
    }

    void Visit(DeclStatement& declStmt) override {
        printNode("decl_statement", "DeclStmt",
                  declStmt.GetLocation().ToString());

        acceptNodeList(declStmt.GetDeclarations(), "Decl");
    }

    void Visit(DefaultStatement& defaultStmt) override {
        printNode("default_statement", "DefaultStmt",
                  defaultStmt.GetLocation().ToString());

        acceptNode(defaultStmt.GetBody(), "Body");
    }

    void Visit(DoStatement& doStmt) override {
        printNode("do_statement", "DoStmt",
                  doStmt.GetLocation().ToString());

        acceptNode(doStmt.GetCondition(), "Condition");
        acceptNode(doStmt.GetBody(), "Body");
    }

    void Visit(ForStatement& forStmt) override {
        printNode("for_statement", "ForStmt",
                  forStmt.GetLocation().ToString());

        if (forStmt.HasInit()) {
            acceptNode(forStmt.GetInit(), "Init");
        }
        
        if (forStmt.HasCondition()) {
            acceptNode(forStmt.GetCondition(), "Condition");
        }

        if (forStmt.HasStep()) {
            acceptNode(forStmt.GetStep(), "Step");
        }

        acceptNode(forStmt.GetBody(), "Body");
    }

    void Visit(GotoStatement& gotoStmt) override {
        printNode("goto_statement", "GotoStmt",
                  gotoStmt.GetLocation().ToString());

        acceptNode(gotoStmt.GetLabel(), "Label");
    }

    void Visit(IfStatement& ifStmt) override {
        printNode("if_statement", "IfStmt",
                  ifStmt.GetLocation().ToString());

        acceptNode(ifStmt.GetCondition(), "Condition");
        acceptNode(ifStmt.GetThen(), "Then");
        if (ifStmt.HasElse()) {
            acceptNode(ifStmt.GetElse(), "Else");
        }
    }

    void Visit(LabelStatement& labelStmt) override {
        printNode("label_statement", "LabelStmt",
                  labelStmt.GetLocation().ToString());

        acceptNode(labelStmt.GetLabel(), "Label");
        acceptNode(labelStmt.GetBody(), "Body");
    }

    void Visit(LoopJumpStatement& loopJmpStmt) override {
        auto typeStr = loopJmpStmt.GetTypeStr();
        printNode("loopjump_statement",
                  std::format("LoopJumpStmt [{}]", typeStr),
                  loopJmpStmt.GetLocation().ToString());
    }

    void Visit(ReturnStatement& returnStmt) override {
        printNode("return_statement", "ReturnStmt",
                  returnStmt.GetLocation().ToString());

        if (returnStmt.HasReturnExpression()) {
            acceptNode(returnStmt.GetReturnExpression(), "ReturnExpr");
        }
    }

    void Visit(SwitchCase& switchCase) override {
        // Base class
    }

    void Visit(SwitchStatement& switchStmt) override {
        printNode("switch_statement", "SwitchStmt",
                  switchStmt.GetLocation().ToString());

        acceptNode(switchStmt.GetExpression(), "Expr");
        acceptNode(switchStmt.GetBody(), "Body");
    }

    void Visit(ValueStatement& valueStmt) override {
        // Base class
    }

    void Visit(WhileStatement& whileStmt) override {
        printNode("while_statement", "WhileStmt",
                  whileStmt.GetLocation().ToString());

        acceptNode(whileStmt.GetCondition(), "Condition");
        acceptNode(whileStmt.GetBody(), "Body");
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
        auto opTypeStr = binaryExpr.GetOpTypeStr();
        printNode("binary_expression",
                  std::format("BinaryExpr [\\{}]", opTypeStr),
                  binaryExpr.GetLocation().ToString());

        acceptNode(binaryExpr.GetLeftOperand(), "LeftOperand");
        acceptNode(binaryExpr.GetRightOperand(), "RightOperand");
    }

    void Visit(CallExpression& callExpr) override {
        printNode("call_expression", "CallExpr",
                  callExpr.GetLocation().ToString());

        acceptNode(callExpr.GetCallee(), "Callee");
        acceptNodeList(callExpr.GetArguments(), "Arg");
    }

    void Visit(CastExpression& castExpr) override {
        if (castExpr.IsLValueToRValue()) {
            printNode("cast_expression", "LValueToRValue",
                      castExpr.GetLocation().ToString());
            acceptNode(castExpr.GetSubExpression(), "SubExpr");
        } else {
            printNode("cast_expression", "CastExpr",
                      castExpr.GetLocation().ToString());
            acceptNode(castExpr.GetSubExpression(), "SubExpr");
            acceptQualType(castExpr.GetToType(), "ToType");
        }
    }

    void Visit(CharExpression& charExpr) override {
        printNode("char_expression",
                  std::format("CharExpression ['{}']",
                              charExpr.GetCharValue()),
                  charExpr.GetLocation().ToString());
    }

    void Visit(ConditionalExpression& condExpr) override {
        printNode("conditional_expression", "CondExpr",
                  condExpr.GetLocation().ToString());
    
        acceptNode(condExpr.GetCondition(), "Condition");
        acceptNode(condExpr.GetTrueExpression(), "TrueExpr");
        acceptNode(condExpr.GetFalseExpression(), "FalseExpr");
    }

    void Visit(ConstExpression& constExpr) override {
        printNode("const_expression", "ConstExpr",
                  constExpr.GetLocation().ToString());

        acceptNode(constExpr.GetExpression(), "Expr");
    }

    void Visit(DeclRefExpression& declrefExpr) override {
        printNode("declref_expression", "DeclRefExpr",
                  declrefExpr.GetLocation().ToString());

        acceptNode(declrefExpr.GetDeclaration(), "Decl");
    }

    void Visit(ExpressionList& exprList) override {
        printNode("expression_list", "ExprList",
                  exprList.GetLocation().ToString());

        acceptNodeList(exprList.GetExpressions(), "Expr");
    }

    void Visit(FloatExpression& floatExpr) override {
        auto floatValue = floatExpr.GetFloatValue();
        printNode("float_expression",
                  std::format("FloatExpr [{}]",
                              floatValue.GetValue()),
                  floatExpr.GetLocation().ToString());
    }

    void Visit(InitializerList& initList) override {
        printNode("initializer_list", "InitList",
                  initList.GetLocation().ToString());

        acceptNodeList(initList.GetInits(), "Init");
    }

    void Visit(IntExpression& intExpr) override {
        auto intValue = intExpr.GetIntValue();
        std::string sign = "unsigned";
        if (intValue.IsSigned()) {
            sign = "signed";
        }
        printNode("int_expression",
                  std::format("IntExpr [{},{}]",
                              intValue.GetSignedValue(), sign),
                  intExpr.GetLocation().ToString()); 
    }

    void Visit(SizeofTypeExpression& sizeofTypeExpr) override {
        printNode("sizeoftype_expression", "SizeofTypeExpr",
                  sizeofTypeExpr.GetLocation().ToString());

        acceptQualType(sizeofTypeExpr.GetType(), "Type");
    }

    void Visit(StringExpression& stringExpr) override {
        printNode("string_expression",
                  std::format("StringExpr [\\\"{}\\\"]",
                              stringExpr.GetStringValue()),
                  stringExpr.GetLocation().ToString());
    }

    void Visit(UnaryExpression& unaryExpr) override {
        auto opTypeStr = unaryExpr.GetOpTypeStr();
        printNode("unary_expression",
                  std::format("UnaryExpr [{}]", opTypeStr),
                  unaryExpr.GetLocation().ToString());

        acceptNode(unaryExpr.GetOperand(), "Operand");
    }


    /*
    =================================================================
                                Type
    =================================================================
    */

    void Visit(ArrayType& arrayType) override {
        auto intValue = arrayType.GetSize();
        std::string sign = "unsigned";
        if (intValue.IsSigned()) {
            sign = "signed";
        }
        printNode("array_type",
                  std::format("ArrayType [{},{}]",
                              intValue.GetSignedValue(), sign));

        acceptQualType(arrayType.GetSubType(), "ElementType");
    }

    void Visit(BuiltinType& builtinType) override {
        auto kindStr = builtinType.GetKindStr();
        printNode("builtin_type",
                  std::format("BuiltinType [{}]", kindStr));
    }

    void Visit(EnumType& enumType) override {
        printNode("enum_type", "EnumType");

        acceptNode(enumType.GetDeclaration(), "Declaration");
    }

    void Visit(FunctionType& funcType) override {
        std::string label = "FunctionType";
        if (funcType.IsVariadic()) {
            label = "FunctionType [variadic]";
        }
        printNode("function_type", label);

        acceptQualType(funcType.GetSubType(), "ReturnType");
        acceptQualTypeList(funcType.GetParamTypes(), "ParamType");
    }

    void Visit(PointerType& ptrType) override {
        printNode("pointer_type", "PointerType");

        acceptQualType(ptrType.GetSubType(), "SubType");
    }

    void Visit(RecordType& recordType) override {
        printNode("record_type", "RecordType");

        acceptNode(recordType.GetDeclaration(), "Declaration");
    }

    void Visit(TagType&) override {
        // Base class
    }
    
    void Visit(Type&) override {
        // Base class
    }

    void Visit(TypedefType& typedefType) override {
        printNode("typedef_type", "TypedefType");

        acceptNode(typedefType.GetDeclaration(), "Declaration");
    }

private:
    void spaceLevelUp() {
        ++m_SpaceLevel;
    }

    void spaceLevelDown() {
        if (m_SpaceLevel > 0) { 
            --m_SpaceLevel;
        }
    }

    void printWithSpaces(const std::string& string) {
        printSpaces();
        m_OutputStream << string;
    }

    void printSpaces() {
        m_OutputStream << std::string(m_SpaceLevel, '\t');
    }
    
    void printNodeAttributes(const std::string& label) {
        printSpaces();
        m_OutputStream << "shape=record,\n";
        printSpaces();
        m_OutputStream << "style=filled,\n";
        printSpaces();
        m_OutputStream << "fillcolor=\"#181818\",\n";
        printSpaces();
        m_OutputStream << std::format("label=\"{}\",\n", label);
        printSpaces();
        m_OutputStream << "fontsize=\"15pt\",\n";
        printSpaces();
        m_OutputStream << "fontcolor=\"#22bd74\",\n";
    }

    void printNode(const std::string& ident, const std::string& label, const std::string& location = "") {
        m_CurrentNodeIdent = getId(ident);

        std::string locLabel = label;
        if (!location.empty()) {
            locLabel = std::format("{} \\<{}\\>", label, location);
        }

        printWithSpaces(std::format("{} [\n", m_CurrentNodeIdent));

        spaceLevelUp();
        printNodeAttributes(locLabel);
        spaceLevelDown();

        printWithSpaces("];\n");

        printWithSpaces(std::format("{} -> {} [label=\"{}\", fontsize=\"13pt\"];\n",
                        m_PreviousNodeIdent, m_CurrentNodeIdent, m_EdgeLabel));
    }

    template <typename S>
    void acceptNode(S* node, const std::string& edgeLabel) {
        m_PreviousNodeIdent = m_CurrentNodeIdent;
        std::string prevNodeIdent = m_PreviousNodeIdent;
        m_EdgeLabel = edgeLabel;
        node->Accept(*this);
        m_CurrentNodeIdent = prevNodeIdent;
    }

    void acceptQualType(QualType qualType, const std::string& edgeLabel) {
        std::string qualStr;
        if (qualType.IsConst()) {
            qualStr.push_back('C');
        }
        if (qualType.IsVolatile()) {
            qualStr.push_back('V');
        }
        if (qualType.IsRestrict()) {
            qualStr.push_back('R');
        }

        acceptNode(qualType.GetSubType(), std::format("{} [{}]", edgeLabel, qualStr));
    }

    void acceptQualTypeList(const std::vector<QualType>& typeList, const std::string& edgeLabel) {
        m_PreviousNodeIdent = m_CurrentNodeIdent;
        std::string prevNodeIdent = m_PreviousNodeIdent;
        for (size_t i = 0; i < typeList.size(); ++i) {
            m_EdgeLabel = std::format("{}{}", edgeLabel, i);
            typeList[i].GetSubType()->Accept(*this);
            m_PreviousNodeIdent = prevNodeIdent;
        }
        m_CurrentNodeIdent = prevNodeIdent;
    }

    template <typename S>
    void acceptNodeList(const std::vector<S*>& nodeList, const std::string& edgeLabel) {
        m_PreviousNodeIdent = m_CurrentNodeIdent;
        std::string prevNodeIdent = m_PreviousNodeIdent;
        for (size_t i = 0; i < nodeList.size(); ++i) {
            m_EdgeLabel = std::format("{}{}", edgeLabel, i);
            nodeList[i]->Accept(*this);
            m_PreviousNodeIdent = prevNodeIdent;
        }
        m_CurrentNodeIdent = prevNodeIdent;
    }

    std::string getId(const std::string& ident) {
        return std::format("{}_{}", ident, m_IdCounter++);
    }

private:
    std::ofstream m_OutputStream;
    size_t m_SpaceLevel = 0;

    const std::string m_GraphName = "program";

    std::string m_CurrentNodeIdent;
    std::string m_PreviousNodeIdent;
    std::string m_EdgeLabel;

    size_t m_IdCounter = 0;
};

}  // namespace ast
