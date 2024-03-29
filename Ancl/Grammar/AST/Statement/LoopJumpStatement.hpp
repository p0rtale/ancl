#pragma once

#include <Ancl/Grammar/AST/Statement/Statement.hpp>


namespace ast {

class LoopJumpStatement: public Statement {
public:
    enum class Type {
        kContinue,
        kBreak,
    };

public:
    LoopJumpStatement(Type type): m_Type(type) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    std::string GetTypeStr() const {
        switch (m_Type) {
            case Type::kContinue: return "continue";
            case Type::kBreak:    return "break";
            default: {
                return "";
            }
        }
    }

private:
    Type m_Type;
};

}  // namespace ast
