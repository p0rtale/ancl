#pragma once

#include <Ancl/Preprocessor/Lexer/Token.hpp>


namespace preproc {

#undef YY_DECL
#define YY_DECL Token Lexer::ScanToken()

class Lexer: public preFlexLexer {
public:
    using preFlexLexer::preFlexLexer;
	virtual ~Lexer() = default;

	virtual Token ScanToken();
};

}  // namespace preproc
