#pragma once

namespace ast {

// Declaration
class Declaration;
class EnumConstDeclaration;
class EnumDeclaration;
class FieldDeclaration;
class FunctionDeclaration;
class LabelDeclaration;
class RecordDeclaration;
class TagDeclaration;
class TranslationUnit;
class TypeDeclaration;
class TypedefDeclaration;
class ValueDeclaration;
class VariableDeclaration;


// Statement
class CaseStatement;
class CompoundStatement;
class DeclStatement;
class DefaultStatement;
class DoStatement;
class ForStatement;
class GotoStatement;
class IfStatement;
class LabelStatement;
class LoopJumpStatement;
class ReturnStatement;
class Statement;
class SwitchCase;
class SwitchStatement;
class ValueStatement;
class WhileStatement;


// Expression
class BinaryExpression;
class CallExpression;
class CastExpression;
class CharExpression;
class ConditionalExpression;
class ConstExpression;
class DeclRefExpression;
class Expression;
class ExpressionList;
class FloatExpression;
class InitializerList;
class IntExpression;
class SizeofTypeExpression;
class StringExpression;
class UnaryExpression;


// Type
class ArrayType;
class BuiltinType;
class EnumType;
class FunctionType;
class PointerType;
class QualType;
class RecordType;
class TagType;
class Type;
class TypedefType;

}  // namespace ast
