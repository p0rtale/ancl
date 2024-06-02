#include "antlr4-runtime.h"
#include "CParserBaseVisitor.h"

#include <Ancl/Grammar/AST/Base/Location.hpp>
#include <Ancl/Grammar/AST/AST.hpp>
#include <Ancl/Grammar/AST/ASTProgram.hpp>

#include <Ancl/Logger/Logger.hpp>


namespace anclgrammar {

class BuildAstVisitor: public CParserBaseVisitor {
public:
    BuildAstVisitor(ast::ASTProgram& program): m_Program(program) {}

    void setLineTokens(const std::vector<antlr4::Token*>& lineTokens) {
        m_LocationBuilder.SetLineTokens(lineTokens);
    }

public:
    std::any visitPrimaryExpression(CParser::PrimaryExpressionContext* ctx) override;
    std::any visitNumberConstant(CParser::NumberConstantContext* ctx) override;

    // Skip
    std::any visitEnumerationConstant(CParser::EnumerationConstantContext* ctx) override {
        return nullptr;
    }

    std::any visitPostfixExpression(CParser::PostfixExpressionContext* ctx) override;
    std::any visitArgumentExpressionList(CParser::ArgumentExpressionListContext *ctx) override;
    std::any visitUnaryExpression(CParser::UnaryExpressionContext *ctx) override;
    std::any visitUnaryExpressionTail(CParser::UnaryExpressionTailContext *ctx) override;

    // Skip
    std::any visitUnaryOperator(CParser::UnaryOperatorContext *ctx) override {
        return nullptr;
    }

    std::any visitCastExpression(CParser::CastExpressionContext* ctx) override;
    std::any visitMultiplicativeExpression(CParser::MultiplicativeExpressionContext* ctx) override;
    std::any visitAdditiveExpression(CParser::AdditiveExpressionContext* ctx) override;
    std::any visitShiftExpression(CParser::ShiftExpressionContext* ctx) override;
    std::any visitRelationalExpression(CParser::RelationalExpressionContext* ctx) override;
    std::any visitEqualityExpression(CParser::EqualityExpressionContext* ctx) override;
    std::any visitAndExpression(CParser::AndExpressionContext* ctx) override;
    std::any visitExclusiveOrExpression(CParser::ExclusiveOrExpressionContext* ctx) override;
    std::any visitInclusiveOrExpression(CParser::InclusiveOrExpressionContext* ctx) override;
    std::any visitLogicalAndExpression(CParser::LogicalAndExpressionContext* ctx) override;
    std::any visitLogicalOrExpression(CParser::LogicalOrExpressionContext* ctx) override;
    std::any visitConditionalExpression(CParser::ConditionalExpressionContext* ctx) override;
    std::any visitAssignmentExpression(CParser::AssignmentExpressionContext* ctx) override;

    // Skip
    std::any visitAssignmentOperator(CParser::AssignmentOperatorContext* ctx) override {
        return nullptr;
    }

    std::any visitExpression(CParser::ExpressionContext* ctx) override;
    std::any visitConstantExpression(CParser::ConstantExpressionContext* ctx) override;

    std::any visitDeclaration(CParser::DeclarationContext* ctx) override;
    std::any visitDeclarationSpecifiers(CParser::DeclarationSpecifiersContext* ctx) override;
    std::any visitDeclarationSpecifier(CParser::DeclarationSpecifierContext* ctx) override;
    std::any visitInitDeclaratorList(CParser::InitDeclaratorListContext* ctx) override;
    std::any visitInitDeclarator(CParser::InitDeclaratorContext* ctx) override;
    std::any visitStorageClassSpecifier(CParser::StorageClassSpecifierContext* ctx) override;
    std::any visitTypeSpecifier(CParser::TypeSpecifierContext* ctx) override;
    std::any visitStructOrUnionSpecifier(CParser::StructOrUnionSpecifierContext* ctx) override;

    // Skip
    std::any visitStructOrUnion(CParser::StructOrUnionContext* ctx) override {
        return nullptr;
    }

    std::any visitStructDeclarationList(CParser::StructDeclarationListContext* ctx) override;
    std::any visitStructDeclaration(CParser::StructDeclarationContext* ctx) override;
    std::any visitSpecifierQualifierList(CParser::SpecifierQualifierListContext* ctx) override;
    std::any visitSpecifierQualifier(CParser::SpecifierQualifierContext* ctx) override;
    std::any visitStructDeclaratorList(CParser::StructDeclaratorListContext* ctx) override;
    std::any visitStructDeclarator(CParser::StructDeclaratorContext *ctx) override;
    std::any visitEnumSpecifier(CParser::EnumSpecifierContext* ctx) override;
    std::any visitEnumeratorList(CParser::EnumeratorListContext* ctx) override;
    std::any visitEnumerator(CParser::EnumeratorContext *ctx) override;
    std::any visitTypeQualifier(CParser::TypeQualifierContext *ctx) override;
    std::any visitFunctionSpecifier(CParser::FunctionSpecifierContext *ctx) override;
    std::any visitDeclarator(CParser::DeclaratorContext* ctx) override;
    std::any visitDirectDeclarator(CParser::DirectDeclaratorContext* ctx) override;
    std::any visitPointer(CParser::PointerContext* ctx) override;
    std::any visitTypeQualifierList(CParser::TypeQualifierListContext* ctx) override;
    std::any visitParameterTypeList(CParser::ParameterTypeListContext* ctx) override;
    std::any visitParameterList(CParser::ParameterListContext* ctx) override;
    std::any visitParameterDeclaration(CParser::ParameterDeclarationContext* ctx) override;
    std::any visitTypeName(CParser::TypeNameContext* ctx) override;
    std::any visitAbstractDeclarator(CParser::AbstractDeclaratorContext* ctx) override;
    std::any visitDirectAbstractDeclarator(CParser::DirectAbstractDeclaratorContext* ctx) override;

    // Skip
    std::any visitTypedefName(CParser::TypedefNameContext* ctx) override {
        return nullptr;
    }

    std::any visitInitializer(CParser::InitializerContext* ctx) override;
    std::any visitInitializerList(CParser::InitializerListContext* ctx) override;

    // Dispatch
    std::any visitStatement(CParser::StatementContext* ctx) override {
        return visitChildren(ctx);
    }

    std::any visitLabeledStatement(CParser::LabeledStatementContext* ctx) override;
    std::any visitCompoundStatement(CParser::CompoundStatementContext* ctx) override;
    std::any visitBlockItemList(CParser::BlockItemListContext* ctx) override;
    std::any visitBlockItem(CParser::BlockItemContext* ctx) override;
    std::any visitExpressionStatement(CParser::ExpressionStatementContext* ctx) override;
    std::any visitSelectionStatement(CParser::SelectionStatementContext* ctx) override;
    std::any visitIterationStatement(CParser::IterationStatementContext* ctx) override;

    std::any visitForCondition(CParser::ForConditionContext* ctx) override;
    std::any visitForDeclaration(CParser::ForDeclarationContext* ctx) override;
    std::any visitForExpression(CParser::ForExpressionContext* ctx) override;

    std::any visitJumpStatement(CParser::JumpStatementContext* ctx) override;

    std::any visitTranslationUnit(CParser::TranslationUnitContext* ctx) override;

    // Dispatch
    std::any visitExternalDeclaration(CParser::ExternalDeclarationContext* ctx) override {
        return visitChildren(ctx);
    }

    std::any visitFunctionDefinition(CParser::FunctionDefinitionContext* ctx) override;

private:
    // visitStorageClassSpecifier return type
    enum class StorageClassInfo {
        kNone = 0,
        kTypedef,
        kExtern,
        kStatic,
        kAuto,
        kRegister,
    };

    // visitFunctionSpecifier return type
    enum class FunctionSpecifier {
        kNone = 0,
        kInline,
    };

    struct BuiltinTypeInfo {
        enum class SizeModifier {
            kNone = 0, kShort, kLong, kLongLong,
        } Size = SizeModifier::kNone;

        enum class BasicType {
            kNone = 0, kVoid, kChar, kInt, kFloat, kDouble,
        } Type = BasicType::kNone;

        enum class SignModifier {
            kNone = 0, kSigned, kUnsigned,
        } Sign = SignModifier::kNone;
    };

    // visitDeclarationSpecifiers, visitSpecifierQualifierList return type
    struct DeclSpecifiers {
        StorageClassInfo Storage = StorageClassInfo::kNone;
        FunctionSpecifier FunSpec = FunctionSpecifier::kNone;
        ast::QualType Type;
        BuiltinTypeInfo BuiltinType;
    };

    // TODO: avoid identifier string copying
    // visitDeclarator return type
    struct DeclaratorInfo {
        std::string Identifier;

        ast::QualType HeadType;
        ast::QualType TailType;

        ast::FunctionDeclaration* FunctionDecl = nullptr;
    };

    // visitInitDeclarator return type
    struct InitDeclaratorInfo {
        DeclaratorInfo DeclInfo;
        ast::Expression* Init = nullptr;
    };

    // visitAbstractDeclarator, visitDirectAbstractDeclarator return type
    struct AbstractDeclaratorInfo {
        ast::QualType HeadType;
        ast::QualType TailType;
    };

    // visitStructDeclarationList return type (std::vector<FieldInfo>)
    struct FieldInfo {
        ast::TagDeclaration* TagPreDecl;
        ast::FieldDeclaration* FieldDecl;
    };

    // visitPointer return type
    struct PointerInfo {
        ast::QualType HeadType;
        ast::QualType TailType;
    };

    // visitParameterTypeList return type
    struct ParamTypesInfo {
        std::vector<ast::ParameterDeclaration*> ParamDecls;
        bool HasVariadic = false;
    };

    // visitTypeQualifierList return type
    struct TypeQualifiers {
        bool Const = false;
        bool Restrict = false;
        bool Volatile = false;
    };

    // visitTypeSpecifier return type
    enum class BuiltinTypeSpecifier {
        kVoid = 0,
        kChar,
        kShort,
        kInt,
        kLong,
        kFloat,
        kDouble,
        kSigned,
        kUnsigned,
    };

    // visitTypeQualifier return type
    enum class Qualifier {
        kNone = 0,
        kConst,
        kRestrict,
        kVolatile,
    };

    // visitDeclaration return type
    struct DeclarationInfo {
        ast::TagDeclaration* TagPreDecl = nullptr;
        ast::Declaration* Decl = nullptr;
    };

    BuildAstVisitor::DeclarationInfo createDeclaration(const DeclSpecifiers& declSpecs,
                                                       const std::vector<DeclaratorInfo>& declList,
                                                       antlr4::ParserRuleContext* ctx,
                                                       bool isParam = false);

    DeclarationInfo createDeclaration(const DeclSpecifiers& declSpecs,
                                      const std::vector<InitDeclaratorInfo>& initDeclList,
                                      antlr4::ParserRuleContext* ctx,
                                      bool isParam = false);

    ast::Type* createBuiltinTypeFromInfo(const BuiltinTypeInfo& info,
                                         antlr4::ParserRuleContext* ctx);

    void updateDeclSpecs(DeclSpecifiers& declSpecs,
                         const std::any& declSpecAny,
                         antlr4::ParserRuleContext* ctx);

    std::string getFunctionSpecifierString(FunctionSpecifier info);

    std::string getStorageClassString(StorageClassInfo info);

private:
    void printSemanticWarning(const std::string& text, const Location& location) {
        ANCL_WARN("{} {}", location.ToString(), text);
    }

    void printSemanticError(const std::string& text, const Location& location) {
        ANCL_ERROR("{} {}", location.ToString(), text);

        // TODO: Handle error
        throw std::runtime_error("Semantic error");
        // exit(EXIT_FAILURE);
    }

    class ASTLocationBuilder {
    public:
        ASTLocationBuilder() = default;

        ASTLocationBuilder(const std::vector<antlr4::Token*>& lineTokens)
            : m_LineTokens(m_LineTokens) {}

        void SetLineTokens(const std::vector<antlr4::Token*>& lineTokens) {
            m_LineTokens = lineTokens;
        }

        Location CreateASTLocation(antlr4::tree::TerminalNode* node) {
            antlr4::Token* token = node->getSymbol();
            size_t line = token->getLine();
            size_t column = token->getCharPositionInLine() + 1;

            std::string sourceName = getLocationFileName(token->getTokenIndex());
            Position start(sourceName, line, column);
            Position stop(sourceName, line, column);
            return Location(std::move(start), std::move(stop));
        }

        Location CreateASTLocation(antlr4::ParserRuleContext* ctx) {
            antlr4::Token* startToken = ctx->getStart();
            size_t startLine = startToken->getLine();
            size_t startColumn = startToken->getCharPositionInLine() + 1;

            antlr4::Token* stopToken = ctx->getStop();
            size_t stopLine = stopToken->getLine();
            size_t stopColumn = stopToken->getCharPositionInLine() + 1;

            std::string sourceName = getLocationFileName(startToken->getTokenIndex());
            Position start(sourceName, startLine, startColumn);
            Position stop(sourceName, stopLine, stopColumn);
            return Location(std::move(start), std::move(stop));
        }

    private:
        std::string getLocationFileName(size_t tokenIndex) {
            if (m_LineTokens.empty()) {
                return "unknown";
            }

            antlr4::Token* lineToken = m_LineTokens[m_LineTokenIndex];
            size_t lineIndex = lineToken->getTokenIndex();
            while (m_LineTokenIndex + 1 < m_LineTokens.size() && tokenIndex > lineIndex) {
                lineToken = m_LineTokens[m_LineTokenIndex + 1];
                lineIndex = lineToken->getTokenIndex();
                if (tokenIndex > lineIndex) {
                    ++m_LineTokenIndex;
                }
            }

            return m_LineTokens[m_LineTokenIndex]->getText();
        }

    private:
        std::vector<antlr4::Token*> m_LineTokens;
        size_t m_LineTokenIndex = 0;
    };

private:
    ast::ASTProgram& m_Program;

    ASTLocationBuilder m_LocationBuilder;
};

};
