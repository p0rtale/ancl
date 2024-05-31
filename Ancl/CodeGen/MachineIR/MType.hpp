#pragma once

#include <cstdint>
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
    MType(Kind kind, uint64_t bytes)
        : m_Kind(kind), m_Bytes(bytes) {}

    static MType CreateScalar(uint64_t bytes, bool isFloat = false) {
        if (isFloat) {
            return MType(Kind::kFloat, bytes);
        }
        return MType(Kind::kInteger, bytes);
    }

    static MType CreatePointer(uint64_t bytes = 8) {
        return MType(Kind::kPointer, bytes);
    }

    void SetBytes(uint64_t bytes) {
        m_Bytes = bytes;
    }

    uint64_t GetBytes() const {
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

    uint64_t m_Bytes = 0;
};

}  // namespace gen
