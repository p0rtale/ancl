#pragma once

/*
=====================================================================
                              Base
=====================================================================
*/

#include <Ancl/Grammar/AST/Base/ASTNode.hpp>


/*
=====================================================================
                             Declaration
=====================================================================
*/

#include <Ancl/Grammar/AST/Declaration/Declaration.hpp>
#include <Ancl/Grammar/AST/Declaration/EnumConstDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/EnumDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/FieldDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/FunctionDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/LabelDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/RecordDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/TagDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/TranslationUnit.hpp>
#include <Ancl/Grammar/AST/Declaration/TypeDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/TypedefDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/ValueDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/VariableDeclaration.hpp>


/*
=====================================================================
                             Statement
=====================================================================
*/

#include <Ancl/Grammar/AST/Statement/CaseStatement.hpp>
#include <Ancl/Grammar/AST/Statement/CompoundStatement.hpp>
#include <Ancl/Grammar/AST/Statement/DeclStatement.hpp>
#include <Ancl/Grammar/AST/Statement/DefaultStatement.hpp>
#include <Ancl/Grammar/AST/Statement/DoStatement.hpp>
#include <Ancl/Grammar/AST/Statement/ForStatement.hpp>
#include <Ancl/Grammar/AST/Statement/GotoStatement.hpp>
#include <Ancl/Grammar/AST/Statement/IfStatement.hpp>
#include <Ancl/Grammar/AST/Statement/LabelStatement.hpp>
#include <Ancl/Grammar/AST/Statement/LoopJumpStatement.hpp>
#include <Ancl/Grammar/AST/Statement/ReturnStatement.hpp>
#include <Ancl/Grammar/AST/Statement/Statement.hpp>
#include <Ancl/Grammar/AST/Statement/SwitchCase.hpp>
#include <Ancl/Grammar/AST/Statement/SwitchStatement.hpp>
#include <Ancl/Grammar/AST/Statement/ValueStatement.hpp>
#include <Ancl/Grammar/AST/Statement/WhileStatement.hpp>


/*
=====================================================================
                            Expression
=====================================================================
*/

#include <Ancl/Grammar/AST/Statement/Expression/BinaryExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/CallExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/CastExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/CharExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/ConditionalExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/ConstExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/DeclRefExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/Expression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/ExpressionList.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/FloatExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/InitializerList.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/IntExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/SizeofTypeExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/StringExpression.hpp>
#include <Ancl/Grammar/AST/Statement/Expression/UnaryExpression.hpp>


/*
=====================================================================
                            Type
=====================================================================
*/

#include <Ancl/Grammar/AST/Type/ArrayType.hpp>
#include <Ancl/Grammar/AST/Type/BuiltinType.hpp>
#include <Ancl/Grammar/AST/Type/EnumType.hpp>
#include <Ancl/Grammar/AST/Type/FunctionType.hpp>
#include <Ancl/Grammar/AST/Type/NodeType.hpp>
#include <Ancl/Grammar/AST/Type/PointerType.hpp>
#include <Ancl/Grammar/AST/Type/QualType.hpp>
#include <Ancl/Grammar/AST/Type/RecordType.hpp>
#include <Ancl/Grammar/AST/Type/TagType.hpp>
#include <Ancl/Grammar/AST/Type/Type.hpp>
#include <Ancl/Grammar/AST/Type/TypedefType.hpp>
#include <Ancl/Grammar/AST/Type/TypeNode.hpp>
