#pragma once

#include <string>
#include <array>
#include <vector>
#include <unordered_map>


namespace preproc {

enum class TokenType {
    None = 0,
    Include,
    Define, IfDef, IfnDef, EndIf,
    Identifier,
    Space, NewLine,
    Quotes,
    AngleBracketLeft,
    AngleBracketRight,
    Comment,
    Symbol,
    End,
};

struct Position {
    size_t Line;
    size_t Column;
    size_t Index;
};

struct Fragment {
    Position Begin;
    Position End;
};

class Token {
public:
    Token(TokenType type, Fragment coords, std::string value = std::string{})
        : m_Type(type), m_Value(std::move(value)), m_Coords(coords) {};

    Token(): m_Type(TokenType::None) {}

    TokenType getType() const {
        return m_Type;
    }

    bool isEnd() const {
        return m_Type == TokenType::End;
    }

    bool isSpace() const {
        return m_Type == TokenType::Space;
    }

    bool isNewLine() const {
        return m_Type == TokenType::NewLine;
    }

    bool isIdentifier() const {
        return m_Type == TokenType::Identifier;  
    }

    bool isEndIf() const {
        return m_Type == TokenType::EndIf;
    }

    bool isQuotes() const {
        return m_Type == TokenType::Quotes;
    }

    bool isAngleBracketLeft() const {
        return m_Type == TokenType::AngleBracketLeft;
    }

    bool isAngleBracketRight() const {
        return m_Type == TokenType::AngleBracketRight;
    }

    bool isComment() const {
        return m_Type == TokenType::Comment;
    }

    bool isAmongTypes(const std::vector<TokenType>& types) const {
        for (auto type: types) {
            if (m_Type == type) {
                return true;
            }
        }
        return false;
    }

    std::string getValue() const {
        return m_Value;
    }

    Fragment getCoords() const {
        return m_Coords;
    }
        
private:
    TokenType m_Type;
    std::string m_Value;
    Fragment m_Coords;
};

}  // namespace preproc
