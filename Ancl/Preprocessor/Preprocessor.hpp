#pragma once

#include <Ancl/Preprocessor/Lexer/Lexer.hpp>
#include <Ancl/Preprocessor/Streams/Streams.hpp>


namespace preproc {

class Preprocessor {
public:
    Preprocessor() = default;

    std::string Run(const std::string& filename, bool debug = false);

private:
    void initRun(const std::string& filename, bool debug);

    std::string processDefine();
    std::string processDefineIdentifier();
    std::vector<std::string> processDefineParameters();
    std::string concatDefineParameters(const std::vector<std::string>& parameters);

    std::string processConditionalDefine(bool negative = false);
    std::string processConditionalEnd();

    std::string processInclude();

    std::string processIdentifier(const std::string& identifierValue);

    Token scanTokenWithComments();
    Token scanFirstNonSpace();

    Token skipSpacesToLineEnd();
    void skipLine();

private:
    Lexer m_Lexer;

    StreamStack m_StreamStack = StreamStack(
        [this](StreamStack::StreamScopeT& streamScope) {
            m_Lexer.yyrestart(streamScope.get());
        },
        [this](StreamStack::StreamScopeT& streamScope) {
            m_Lexer.yyrestart(streamScope.get());
        }
    );

    size_t m_LineOffset = 0;

    std::string m_PreprocessedString;
    std::unordered_map<std::string, std::string> m_Defines;

    bool m_IsInsideCondDef = false;
};

}  // namespace preproc
