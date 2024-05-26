#pragma once

#include <Ancl/Grammar/AST/Type/Type.hpp>
#include <Ancl/Grammar/AST/Type/TypeNode.hpp>


namespace ast {

class QualType {
public:
    QualType() = default;
    QualType(Type* subType): m_SubType(subType) {}

    void SetSubType(Type* subType) {
        m_SubType = subType;
    }

    bool HasSubType() {
        return m_SubType;
    }

    Type* GetSubType() const {
        return m_SubType;
    }

    bool IsConst() const {
        return m_IsConst;
    }

    void AddConst() {
        m_IsConst = true;
    }

    void RemoveConst() {
        m_IsConst = false;
    }

    bool IsVolatile() const {
        return m_IsVolatile;
    }

    void AddVolatile() {
        m_IsVolatile = true;
    }

    void RemoveVolatile() {
        m_IsVolatile = false;
    }

    bool IsRestrict() const {
        return m_IsRestrict;
    }

    void AddRestrict() {
        m_IsRestrict = true;
    }

    void RemoveRestrict() {
        m_IsRestrict = false;
    }

    void RemoveQualifiers() {
        RemoveConst();
        RemoveVolatile();
        RemoveRestrict();
    }

private:
    Type* m_SubType = nullptr;

    bool m_IsConst = false;
    bool m_IsVolatile = false;
    bool m_IsRestrict = false;
};

}  // namespace ast
