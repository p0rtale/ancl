#pragma once

#include <Ancl/Grammar/AST/Type/Type.hpp>


namespace ast {

class BuiltinType: public Type {
public:
    enum class Kind {
        kNone = 0,
        kVoid,
        kChar,
        kUChar,
        kShort,
        kUShort,
        kInt,
        kUInt,
        kLong,
        kULong,
        kFloat,
        kDouble,
        kLongDouble,
    };

    enum Rank : unsigned int {
        kVoidRank = 0,
        kCharRank = 1,
        kUCharRank = 1,
        kShortRank = 2,
        kUShortRank = 2,
        kIntRank = 3,
        kUIntRank = 3,
        kLongRank = 4,
        kULongRank = 4,
        kFloatRank = 5,
        kDoubleRank = 6,
        kLongDoubleRank = 7,
    };

public:
    BuiltinType(): m_Kind(Kind::kNone) {}
    BuiltinType(Kind kind): m_Kind(kind) {}

    void Accept(AstVisitor& visitor) override {
        visitor.Visit(*this);
    }

    bool IsVoid() const {
        return m_Kind == Kind::kVoid;
    }

    bool IsInteger() const {
        return IsSignedInteger() || IsUnsignedInteger();
    }

    unsigned int GetRank() const {
        switch (m_Kind) {
            case Kind::kVoid:        return 0;
            case Kind::kChar:        return 1;
            case Kind::kUChar:       return 1;
            case Kind::kShort:       return 2;
            case Kind::kUShort:      return 2;
            case Kind::kInt:         return 3;
            case Kind::kUInt:        return 3;
            case Kind::kLong:        return 4;
            case Kind::kULong:       return 4;
            case Kind::kFloat:       return 5;
            case Kind::kDouble:      return 6;
            case Kind::kLongDouble:  return 7;
        }
        return 0;
    }

    bool IsSignedInteger() const {
        switch (m_Kind) {
            case Kind::kChar:
            case Kind::kShort:
            case Kind::kInt:
            case Kind::kLong:
                return true;
        }
        return false;  
    }

    bool IsUnsignedInteger() const {
        switch (m_Kind) {
            case Kind::kUChar:
            case Kind::kUShort:
            case Kind::kUInt:
            case Kind::kULong:
                return true;
        }
        return false;  
    }

    bool IsFloat() const {
        switch (m_Kind) {
            case Kind::kFloat:
            case Kind::kDouble:
            case Kind::kLongDouble:
                return true;
        }
        return false;
    }

    bool IsSinglePrecision() const {
        return m_Kind == Kind::kFloat;
    }

    bool IsDoublePrecision() const {
        return m_Kind == Kind::kDouble;
    }

    void SetKind(Kind kind) {
        m_Kind = kind;
    }  

    Kind GetKind() const {
        return m_Kind;
    }

    std::string GetKindStr() const {
        switch (m_Kind) {
            case Kind::kVoid:  return "void";

            case Kind::kChar:   return "char";
            case Kind::kUChar:  return "unsigned char";
 
            case Kind::kShort:   return "short";
            case Kind::kUShort:  return "unsigned short";
            case Kind::kInt:     return "int";
            case Kind::kUInt:    return "unsigned int";
            case Kind::kLong:    return "long";
            case Kind::kULong:   return "unsigned long";

            case Kind::kFloat:       return "float";
            case Kind::kDouble:      return "double";
            case Kind::kLongDouble:  return "long double";

            default: {
                return "";
            }
        }
    }

private:
    Kind m_Kind;
};

}  // namespace ast
