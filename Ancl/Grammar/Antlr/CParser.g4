/*
 [The "BSD licence"]
 Copyright (c) 2013 Sam Harwell
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. The name of the author may not be used to endorse or promote products
    derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/** C 2011 grammar built from the C11 Spec */

// $antlr-format alignTrailingComments true, columnLimit 150, minEmptyLines 1, maxEmptyLinesToKeep 1, reflowComments false, useTab false
// $antlr-format allowShortRulesOnASingleLine false, allowShortBlocksOnASingleLine true, alignSemicolons hanging, alignColons hanging

parser grammar CParser;

options {
	tokenVocab = CLexer;
}

primaryExpression
    : Identifier
    | numberConstant
    | StringLiteral+
    | '(' expression ')'
    ;

numberConstant
    : IntegerConstant
    | FloatingConstant
    | CharacterConstant
    ;

enumerationConstant
    : Identifier
    ;

// Without compound literals
postfixExpression
    // : (primaryExpression | '(' typeName ')' '{' initializerList ','? '}') (
    // : primaryExpression (elements+=postfixExpressionTailElement)*
    : primaryExpression
    | array=postfixExpression '[' index=expression ']'
    | callee=postfixExpression '(' args=argumentExpressionList? ')'
    | tag=postfixExpression member=('.' | '->') Identifier
    | postfixExpression inc='++'
    | postfixExpression dec='--'
    ;

argumentExpressionList
    : expr=assignmentExpression (',' exprTail+=assignmentExpression)*
    ;

unaryExpression
    // : ('++' | '--' | 'sizeof')* unaryExpressionTail
    : unaryExpressionTail
    | inc='++' unaryExpression
    | dec='--' unaryExpression
    | size='sizeof' unaryExpression
    ;

unaryExpressionTail
    : postfixExpression
    | unaryOperator castExpression
    | 'sizeof' '(' typeName ')'
    ;

unaryOperator
    : '&'
    | '*'
    | '+'
    | '-'
    | '~'
    | '!'
    ;

castExpression
    : '(' typeName ')' castExpression
    | unaryExpression
    // | DigitSequence // for ?????????????????????
    ;

multiplicativeExpression
    // : castExpression (('*' | '/' | '%') castExpression)*
    : castexpr=castExpression
    | multiplicativeExpression mul='*' castExpression
    | multiplicativeExpression div='/' castExpression
    | multiplicativeExpression rem='%' castExpression
    ;

additiveExpression
    // : multiplicativeExpression (('+' | '-') multiplicativeExpression)*
    : mulexpr=multiplicativeExpression
    | additiveExpression add='+' multiplicativeExpression
    | additiveExpression sub='-' multiplicativeExpression
    ;

shiftExpression
    // : additiveExpression (('<<' | '>>') additiveExpression)*
    : addexpr=additiveExpression
    | shiftExpression shiftl='<<' additiveExpression
    | shiftExpression shiftr='>>' additiveExpression
    ;

relationalExpression
    // : shiftExpression (('<' | '>' | '<=' | '>=') shiftExpression)*
    : shiftexpr=shiftExpression
    | relationalExpression less='<' shiftExpression
    | relationalExpression greater='>' shiftExpression
    | relationalExpression lesseq='<=' shiftExpression
    | relationalExpression greatereq='>=' shiftExpression
    ; 

equalityExpression
    // : relationalExpression (('==' | '!=') relationalExpression)*
    : relexpr=relationalExpression
    | equalityExpression equal='==' relationalExpression
    | equalityExpression nequal='!=' relationalExpression
    ;

andExpression
    // : expr=equalityExpression ('&' exprTail+=equalityExpression)*
    : eqexpr=equalityExpression
    | andExpression '&' equalityExpression
    ;

exclusiveOrExpression
    // : expr=andExpression ('^' exprTail+=andExpression)*
    : andexpr=andExpression
    | exclusiveOrExpression '^' andExpression
    ;

inclusiveOrExpression
    // : expr=exclusiveOrExpression ('|' exprTail+=exclusiveOrExpression)*
    : exorexpr=exclusiveOrExpression
    | inclusiveOrExpression '|' exclusiveOrExpression
    ;

logicalAndExpression
    // : expr=inclusiveOrExpression ('&&' exprTail+=inclusiveOrExpression)*
    : incorexpr=inclusiveOrExpression
    | logicalAndExpression '&&' inclusiveOrExpression
    ;

logicalOrExpression
    // : expr=logicalAndExpression ('||' exprTail+=logicalAndExpression)*
    : logandexpr=logicalAndExpression
    | logicalOrExpression '||' logicalAndExpression
    ;

conditionalExpression
    // : expr=logicalOrExpression ('?' exprTail+=expression ':' condExprTail+=conditionalExpression)?
    : logorexpr=logicalOrExpression
    | logicalOrExpression '?' expression ':' conditionalExpression
    ;

assignmentExpression
    : condexpr=conditionalExpression
    | unaryExpression assignmentOperator assignmentExpression
    // | DigitSequence // for ??????????????????????????
    ;

assignmentOperator
    : '='
    | '*='
    | '/='
    | '%='
    | '+='
    | '-='
    | '<<='
    | '>>='
    | '&='
    | '^='
    | '|='
    ;

expression
    : expr=assignmentExpression (',' exprTail+=assignmentExpression)*
    ;

constantExpression
    : conditionalExpression
    ;

declaration
    // : declarationSpecifiers initDeclaratorList? ';'    ?????????????
    : declspecs=declarationSpecifiers initdecl=initDeclaratorList ';'
    | declspecs=declarationSpecifiers ';'
    ;

declarationSpecifiers
    : specs+=declarationSpecifier+
    ;

declarationSpecifier
    : storage=storageClassSpecifier
    | typespec=typeSpecifier
    | qualifier=typeQualifier
    | funcspec=functionSpecifier
    ;

initDeclaratorList
    : init=initDeclarator (',' initTail+=initDeclarator)*
    ;

initDeclarator
    : decl=declarator ('=' init=initializer)?
    ;

storageClassSpecifier
    : 'typedef'
    | 'extern'
    | 'static'
    | 'auto'  // ignore?
    | 'register'
    ;

typeSpecifier
    : type='void'
    | type='char'
    | type='short'
    | type='int'
    | type='long'
    | type='float'
    | type='double'
    | type='signed'
    | type='unsigned'
    | recordspec=structOrUnionSpecifier
    | enumspec=enumSpecifier
    | typedefname=typedefName
    ;

structOrUnionSpecifier
    : structOrUnion Identifier? '{' body=structDeclarationList '}'
    | structOrUnion Identifier
    ;

structOrUnion
    : 'struct'
    | 'union'
    ;

structDeclarationList
    : decls+=structDeclaration+
    ;

structDeclaration // The first two rules have priority order and cannot be simplified to one expression.
    : specifierQualifierList structDeclaratorList ';'
    | specifierQualifierList ';'  // avoid last typeSpecifier -> typedefName instead of declarator
    ;

specifierQualifierList
    : (typeSpecifier | typeQualifier) specifierQualifierList?
    ;

structDeclaratorList
    : decl=structDeclarator (',' declTail+=structDeclarator)*
    ;

structDeclarator
    : declarator
    // | declarator? ':' constantExpression
    /* bit field */
    ;

enumSpecifier
    : 'enum' Identifier? '{' enumerators=enumeratorList ','? '}'
    | 'enum' Identifier
    ;

enumeratorList
    : firstenum=enumerator (',' enumTail+=enumerator)*
    ;

enumerator
    : ident=enumerationConstant ('=' init=constantExpression)?
    ;

typeQualifier
    : 'const'
    | 'restrict'
    | 'volatile'
    ;

functionSpecifier
    : 'inline'
    ;

declarator
    : pointer? directDeclarator
    ;

directDeclarator
    : Identifier
    | '(' nested=declarator ')'
    | arrdecl=directDeclarator '[' sizeexpr=constantExpression? ']'
    | fundecl=directDeclarator '(' paramlist=parameterTypeList? ')'
    // | directDeclarator '[' typeQualifierList? assignmentExpression? ']'
    // | directDeclarator '[' 'static' typeQualifierList? assignmentExpression ']'
    // | directDeclarator '[' typeQualifierList 'static' assignmentExpression ']'
    // | directDeclarator '[' typeQualifierList? '*' ']'
    // | directDeclarator '(' parameterTypeList ')'
    // | directDeclarator '(' identifierList? ')'

    // | Identifier ':' DigitSequence         // bit field  ???????????
    ;

pointer
    : ('*' quals+=typeQualifierList?)+
    ;

typeQualifierList
    : qualifiers+=typeQualifier+
    ;

parameterTypeList
    : params=parameterList (',' vararg='...')?
    ;

parameterList
    : decl=parameterDeclaration (',' declTail+=parameterDeclaration)*
    ;

parameterDeclaration
    : declarationSpecifiers declarator
    | declarationSpecifiers abstractDeclarator?
    ;

// identifierList
//     : Identifier (',' Identifier)*
//     ;
/* for directDeclarator */

typeName
    : specifierQualifierList abstractDeclarator?
    ;

abstractDeclarator
    : pointer
    | pointer? decl=directAbstractDeclarator
    ;

directAbstractDeclarator
    : '(' nested=abstractDeclarator ')'
    | leafarr='[' sizeexpr=constantExpression? ']'
    // | '[' typeQualifierList? assignmentExpression? ']'
    // | '[' 'static' typeQualifierList? assignmentExpression ']'
    // | '[' typeQualifierList 'static' assignmentExpression ']'
    // | '[' '*' ']'
    | arrdecl=directAbstractDeclarator '[' sizeexpr=constantExpression? ']'
    | leaffun='(' paramlist=parameterTypeList? ')'
    // | directAbstractDeclarator '[' typeQualifierList? assignmentExpression? ']'
    // | directAbstractDeclarator '[' 'static' typeQualifierList? assignmentExpression ']'
    // | directAbstractDeclarator '[' typeQualifierList 'static' assignmentExpression ']'
    // | directAbstractDeclarator '[' '*' ']'
    | fundecl=directAbstractDeclarator '(' paramlist=parameterTypeList? ')'
    ;

typedefName
    : Identifier
    ;

initializer
    : init=assignmentExpression
    | '{' initlist=initializerList ','? '}'
    ;

initializerList
    // : designation? initializer (',' designation? initializer)*
    /*
        C99 designated initializer
    */
    : init=initializer (',' initTail+=initializer)*
    ;

// designation
//     : designatorList '='
//     ;

// designatorList
//     : designator+
//     ;

// designator
//     : '[' constantExpression ']'
//     | '.' Identifier
//     ;

statement
    : labeledStatement
    | compoundStatement
    | expressionStatement
    | selectionStatement
    | iterationStatement
    | jumpStatement
    ;

labeledStatement
    : Identifier ':' statement?   // optional statement?????????????????
    | 'case' constantExpression ':' statement
    | 'default' ':' statement
    ;

compoundStatement
    : '{' blockItemList? '}'
    ;

blockItemList
    : items+=blockItem+
    ;

blockItem
    : statement
    | declaration
    ;

expressionStatement
    : expression? ';'   // ;; ??????????????????????????
    ;

selectionStatement
    : 'if' '(' cond=expression ')' thenstmt=statement ('else' elsestmt=statement)?
    | 'switch' '(' expression ')' switchstmt=statement
    ;

iterationStatement
    : loop='while' '(' cond=expression ')' stmt=statement
    | loop='do' stmt=statement 'while' '(' cond=expression ')' ';'
    | loop='for' '(' forCondition ')' stmt=statement
    ;

//    |   'for' '(' expression? ';' expression?  ';' forUpdate? ')' statement
//    |   For '(' declaration  expression? ';' expression? ')' statement

forCondition
    : (decl=forDeclaration | expr=expression?) ';' cond=forExpression? ';' step=forExpression?
    ;

forDeclaration
    // : declarationSpecifiers initDeclaratorList?   ?????????????
    : declarationSpecifiers initDeclaratorList
    | declarationSpecifiers
    ;

forExpression
    : expr=assignmentExpression (',' exprTail+=assignmentExpression)*
    ;

jumpStatement
    : (
        'goto' Identifier
        | 'continue'
        | 'break'
        | 'return' expression?
    ) ';'
    ;

translationUnit
    : decls+=externalDeclaration+ EOF
    ;

externalDeclaration
    : functionDefinition
    | declaration
    // | ';' // stray ;    ??????????????
    ;

functionDefinition
    // : declarationSpecifiers? declarator declarationList? compoundStatement
    /*

        C99 does no longer allow empty declarationSpecifiers
        and declarationList before compound statement

    */
    : declspecs=declarationSpecifiers declarator body=compoundStatement
    ;

// declarationList
//     : declaration+
//     ;
/* used in functionDefinition */
