%option noyywrap
%option c++
%option prefix="pre"
%option yyclass="preprocessor::Lexer"

%option debug


%{

#include <Ancl/Preprocessor/Lexer/Token.hpp>
#include <Ancl/Preprocessor/Lexer/LexerInternal.hpp>

using namespace preproc;

Position g_CurrentPosition = {
    .Line = 1,
    .Column = 1,
    .Index = 0,
};

Fragment g_CurrentFragment;

bool continueFragment = false;

#define YY_USER_ACTION                                      \
    {                                                       \
        int i;                                              \
        if (!continueFragment) {                            \
            g_CurrentFragment.Begin = g_CurrentPosition;    \
        }                                                   \
        continueFragment = false;                           \
                                                            \
        for (i = 0; i < yyleng; i++) {                      \
            if (yytext[i] == '\n') {                        \
                g_CurrentPosition.Line++;                   \
                g_CurrentPosition.Column = 1;               \
            } else {                                        \
                g_CurrentPosition.Column++;                 \
                g_CurrentPosition.Index++;                  \
            }                                               \
        }                                                   \
                                                            \
        g_CurrentFragment.End = g_CurrentPosition;          \
    }

%}

blank           [ \t\r]
directive_start #[ \t]*
identifier      [a-zA-Z][a-zA-Z_0-9]*
separator       [,()\+\-*/&|!=;]

%x COMMENT

%%
{blank}+ {
    return Token(TokenType::Space, g_CurrentFragment);
}

\n {
    return Token(TokenType::NewLine, g_CurrentFragment);
}

"/**/" {
    return Token(TokenType::Comment, g_CurrentFragment);
}
"/*" {
    BEGIN(COMMENT);
    continueFragment = true;
}
<COMMENT>"*/" {
    BEGIN(INITIAL);
    return Token(TokenType::Comment, g_CurrentFragment);
}
<COMMENT>\n {
    continueFragment = true;
}
<COMMENT>. {
    continueFragment = true;
}

"//".+ {
    return Token(TokenType::Comment, g_CurrentFragment);
}

{directive_start}include {
    return Token(TokenType::Include, g_CurrentFragment, yytext);
}

{directive_start}define {
    return Token(TokenType::Define, g_CurrentFragment, yytext);
}

{directive_start}ifdef {
    return Token(TokenType::IfDef, g_CurrentFragment, yytext);
}

{directive_start}ifndef {
    return Token(TokenType::IfnDef, g_CurrentFragment, yytext);
}

{directive_start}endif {
    return Token(TokenType::EndIf, g_CurrentFragment, yytext);
}

{identifier} {
    return Token(TokenType::Identifier, g_CurrentFragment, yytext);    
}

\" {
    return Token(TokenType::Quotes, g_CurrentFragment, yytext);
}

\< {
    return Token(TokenType::AngleBracketLeft, g_CurrentFragment, yytext);
}

\> {
    return Token(TokenType::AngleBracketRight, g_CurrentFragment, yytext);
}

. {
    return Token(TokenType::Symbol, g_CurrentFragment, yytext); 
}

<<EOF>> {
    return Token(TokenType::End, g_CurrentFragment);
}
%%
