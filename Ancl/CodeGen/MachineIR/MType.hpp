#pragma once

#include <string>


namespace gen {

class MType {
public:
    enum class Kind {
        kNone,
        kInteger,
        kFloat,
        kPointer,
    };

public:
    MType() = default;
    MType(Kind kind, uint bytes)
        : m_Kind(kind), m_Bytes(bytes) {}

    static MType CreateScalar(uint bytes, bool isFloat = false) {
        if (isFloat) {
            return MType(Kind::kFloat, bytes);
        }
        return MType(Kind::kInteger, bytes);
    }

    static MType CreatePointer(uint bytes = 8) {
        return MType(Kind::kPointer, bytes);
    }

    void SetBytes(uint bytes) {
        m_Bytes = bytes;
    }

    uint GetBytes() const {
        return m_Bytes;
    }

    bool IsNone() const {
        return m_Kind == Kind::kNone;
    }

    bool IsInteger() const {
        return m_Kind == Kind::kInteger;
    }

    bool IsFloat() const {
        return m_Kind == Kind::kFloat;
    }

    bool IsScalar() const {
        return IsInteger() || IsFloat();
    }

    bool IsPointer() const {
        return m_Kind == Kind::kPointer;
    }

private:
    Kind m_Kind = Kind::kNone;

    uint m_Bytes = 0;
};

}  // namespace gen
