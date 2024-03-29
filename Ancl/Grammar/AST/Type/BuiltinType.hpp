#pragma once

#include <Ancl/Grammar/AST/Type/Type.hpp>


namespace ast {

class BuiltinType: public Type {
public:
    enum class Kind {
        kVoid = 0,
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

public:
    BuiltinType(): m_Kind(Kind::kVoid) {}
    BuiltinType(Kind kind): m_Kind(kind) {}

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
