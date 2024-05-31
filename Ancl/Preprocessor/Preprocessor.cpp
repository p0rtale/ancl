#include <Ancl/Preprocessor/Preprocessor.hpp>

#include <format>

#include <Ancl/Base.hpp>


namespace preproc {

std::string Preprocessor::Run(const std::string& filename, bool debug) {
    initRun(filename, debug);
    // ANCL_INFO("Initialization completed");

    Token tokenPrevious;
    Token tokenCurrent;
    while (!m_StreamStack.IsEmpty()) {
        tokenCurrent = scanTokenWithComments();
        if (tokenCurrent.isEnd()) {
            // ANCL_INFO("End of stream");
            auto coords = tokenCurrent.getCoords();
            size_t currentLine = coords.Begin.Line - m_LineOffset;
            m_LineOffset += currentLine - 1;
            auto [nextFileName, line] = m_StreamStack.PopStream();
            if (nextFileName != "") {
                m_PreprocessedString.append(
                    std::format("\n#line {} \"{}\"\n",
                                line, nextFileName)
                );
            }
            continue;
        }

        std::string preprocessed;
        switch (tokenCurrent.getType()) {
        case TokenType::NewLine:
            // ANCL_TRACE("Process NewLine");
            preprocessed = "\n";
            break;
        case TokenType::Include:
            // ANCL_TRACE("Process Include");
            preprocessed = processInclude();
            break;
        case TokenType::Define:
            // ANCL_TRACE("Process Define");
            preprocessed = processDefine();
            break;
        case TokenType::IfDef:
            // ANCL_TRACE("Process IfDef");
            preprocessed = processConditionalDefine(false);
            break;
        case TokenType::IfnDef:
            // ANCL_TRACE("Process IfnDef");
            preprocessed = processConditionalDefine(true);
            break;
        case TokenType::EndIf:
            // ANCL_TRACE("Process EndIf");
            preprocessed = processConditionalEnd();
            break;
        case TokenType::Identifier:
            // ANCL_TRACE("Process Identifier");
            preprocessed = processIdentifier(tokenCurrent.getValue());
            break;
        case TokenType::Space:
            // ANCL_TRACE("Process Space");
            if (tokenPrevious.isAmongTypes({
                        TokenType::NewLine,
                        TokenType::Include, TokenType::Define,
                        TokenType::IfDef, TokenType::IfnDef, TokenType::EndIf
                    })) {
                auto spaceCoords = tokenCurrent.getCoords();
                preprocessed = std::string(spaceCoords.End.Column - 1, ' ');
            } else {
                preprocessed = " ";
            }
            break;
        default:
            // ANCL_TRACE("Process Token");
            preprocessed = tokenCurrent.getValue();
        }

        m_PreprocessedString.append(preprocessed);
        tokenPrevious = tokenCurrent;
    }

    // ANCL_INFO("Preprocessor is completed");

    return m_PreprocessedString;
}

void Preprocessor::initRun(const std::string& filename, bool debug) {
    if (debug) {
        m_Lexer.set_debug(true);
    }
    // ANCL_INFO("Add new stream");
    m_StreamStack.PushStream(filename, 1);
    m_PreprocessedString = std::format("#line {} \"{}\"\n", 1, filename);
}

std::string Preprocessor::processDefine() {
    auto token = scanTokenWithComments();
    if (!token.isSpace()) {
        ANCL_ERROR("processDefine: no Space after keyword");
        return std::string{'\n'};
    }

    auto defineIdentifier = processDefineIdentifier();
    auto parameters = processDefineParameters();
    if (parameters.empty()) {
        m_Defines[defineIdentifier] = "";
    } else {
        m_Defines[defineIdentifier] = concatDefineParameters(parameters);
    }
    return std::string{'\n'};
}

std::string Preprocessor::processDefineIdentifier() {
    auto token = scanTokenWithComments();
    if (token.isIdentifier()) {
        return token.getValue();
    }
    ANCL_ERROR("processDefineIdentifier: Token ({}) is not Identifier", token.getValue());
    return std::string{};
}

std::vector<std::string> Preprocessor::processDefineParameters() {
    std::vector<std::string> parameters;

    Token token;
    std::string parameter;
    while (!token.isNewLine() && !token.isEnd()) {
        token = scanTokenWithComments();
        if (token.isSpace()) {
            if (!parameter.empty()) {
                parameters.push_back(parameter);
            }
            parameter.clear();
        } else {
            parameter.append(token.getValue());
        }
    }

    if (!parameter.empty()) {
        parameters.push_back(parameter);
    }

    return parameters;
}

std::string Preprocessor::concatDefineParameters(const std::vector<std::string>& parameters) {
    // auto parametersStr = std::accumulate(parameters.begin(), parameters.end(), std::string{},
    //     [](std::string a, std::string b){ return std::move(a) + b + ' '; });
    std::string parametersStr;
    for (const auto& parameter: parameters) {
        parametersStr.append(parameter);
        parametersStr.push_back(' ');
    }
    parametersStr.pop_back();
    return parametersStr;
}

std::string Preprocessor::processConditionalDefine(bool negative) {
    auto token = scanTokenWithComments();
    if (!token.isSpace()) {
        ANCL_ERROR("processConditionalDefine: no Space after keyword");
        return std::string{'\n'};
    }

    auto defineIdentifier = processDefineIdentifier();
    skipSpacesToLineEnd();

    bool isDefined = m_Defines.contains(defineIdentifier);
    if (isDefined == !negative) {
        // ANCL_TRACE("processConditionalDefine: go inside conditional define");
        m_IsInsideCondDef = true;
    } else {
        // ANCL_TRACE("processConditionalDefine: skip conditional define to EndIf");
        size_t linesCount = 2;  // IfDef + EndIf
        auto firstToken = scanFirstNonSpace();
        while (!firstToken.isEndIf() && !firstToken.isEnd()) {
            ++linesCount;
            if (!firstToken.isNewLine()) {
                skipLine();
            }
            firstToken = scanFirstNonSpace();
        }
        if (firstToken.isEnd()) {
            ANCL_ERROR("processConditionalDefine: got EOF, EndIf expected");
        }
        skipSpacesToLineEnd();

        return std::string(linesCount, '\n');
    }

    return std::string{'\n'};
}

std::string Preprocessor::processConditionalEnd() {
    if (m_IsInsideCondDef) {
        m_IsInsideCondDef = false;
    } else {
        ANCL_ERROR("processConditionalEnd: unexpected EndIF");
    }
    return std::string{'\n'};
}

std::string Preprocessor::processInclude() {
    auto token = scanTokenWithComments();
    if (!token.isSpace()) {
        ANCL_ERROR("processInclude: no Space after keyword");
        return std::string{'\n'};
    }

    auto leftBoundToken = scanTokenWithComments();   
    if (!leftBoundToken.isQuotes() && !leftBoundToken.isAngleBracketLeft()) {
        ANCL_ERROR("processInclude: no leftBoundToken after space");
        return std::string{'\n'};
    }

    auto rightBoundExpectedType = TokenType::Quotes;
    if (leftBoundToken.isAngleBracketLeft()) {
        rightBoundExpectedType = TokenType::AngleBracketRight;
    }

    std::string includeFilename;
    auto filenameToken = scanTokenWithComments();
    while (filenameToken.getType() != rightBoundExpectedType) {
        std::string value; 
        if (filenameToken.isSpace()) {
            auto spaceCoords = filenameToken.getCoords();
            value = std::string(' ', spaceCoords.End.Column - spaceCoords.Begin.Column);
        } else {
            value = filenameToken.getValue();
        }
        includeFilename.append(value);

        filenameToken = scanTokenWithComments();
    }

    Token nextToken = skipSpacesToLineEnd();

    // ANCL_INFO("processInclude: includeFilename=\"{}\"", includeFilename);
    // ANCL_INFO("Add new stream");
    auto coords = nextToken.getCoords();
    size_t line = coords.End.Line - m_LineOffset;
    m_LineOffset += line - 1;
    m_StreamStack.PushStream(includeFilename, line);
    m_PreprocessedString.append(std::format("#line {} \"{}\"", 1, includeFilename));

    return std::string{'\n'};
}

std::string Preprocessor::processIdentifier(const std::string& identifierValue) {
    if (m_Defines.contains(identifierValue)) {
        return m_Defines[identifierValue];
    }
    return identifierValue;
}

Token Preprocessor::scanTokenWithComments() {
    auto token = m_Lexer.ScanToken();
    while (token.isComment()) {
        auto coords = token.getCoords();
        size_t newlinesCount = coords.End.Line - coords.Begin.Line;
        if (newlinesCount > 0) {
            m_PreprocessedString.append(std::string(newlinesCount, '\n'));
        }
        token = m_Lexer.ScanToken();
    }

    auto [positionBegin, positionEnd] = token.getCoords();
    // ANCL_TRACE("Scan Token {}({}) {},{}-{},{}",
    //     static_cast<uint32_t>(token.getType()), token.getValue(),
    //     positionBegin.Line, positionBegin.Column, positionEnd.Line, positionEnd.Column);

    return token;
}

Token Preprocessor::scanFirstNonSpace() {
    auto token = m_Lexer.ScanToken();
    while (token.isSpace()) {
        token = m_Lexer.ScanToken();
    }

    auto [positionBegin, positionEnd] = token.getCoords();
    // ANCL_TRACE("Scan Token {}({}) {},{}-{},{}",
    //     static_cast<uint32_t>(token.getType()), token.getValue(),
    //     positionBegin.Line, positionBegin.Column, positionEnd.Line, positionEnd.Column);

    return token; 
}

Token Preprocessor::skipSpacesToLineEnd() {
    auto token = m_Lexer.ScanToken();
    while (token.isSpace() || token.isComment()) {
        token = m_Lexer.ScanToken();
    }

    auto [positionBegin, positionEnd] = token.getCoords();
    auto tokenType = token.getType();
    if (!token.isNewLine() && !token.isEnd()) {
        ANCL_ERROR("skipSpacesToLineEnd: unexpected Token {}({}) {},{}-{},{}",
            static_cast<uint32_t>(tokenType), token.getValue(),
            positionBegin.Line, positionBegin.Column, positionEnd.Line, positionEnd.Column);
    }

    // ANCL_TRACE("Scan Token {}({}) {},{}-{},{}",
    //     static_cast<uint32_t>(tokenType), token.getValue(),
    //     positionBegin.Line, positionBegin.Column, positionEnd.Line, positionEnd.Column);

    return token;
}

void Preprocessor::skipLine() {
    auto token = m_Lexer.ScanToken();
    while (!token.isNewLine() && !token.isEnd()) {
        token = m_Lexer.ScanToken();
    }
}

}  // namespace preproc
