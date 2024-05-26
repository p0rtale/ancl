#pragma once

#include <Ancl/Preprocessor/Lexer/Token.hpp>


namespace preproc {

#undef YY_DECL
#define YY_DECL Token Lexer::scanToken()

class Lexer: public preFlexLexer {
public:
    using preFlexLexer::preFlexLexer;
	virtual ~Lexer() = default;

	virtual Token scanToken();
};

}  // namespace preproc
