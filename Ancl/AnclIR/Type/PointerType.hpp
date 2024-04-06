#pragma once

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class PointerType: public Type {
public:
    PointerType(Type* subType): m_SubType(subType) {}

    Type* GetSubType() {
        return m_SubType;
    }

private:
    Type* m_SubType;
};

}  // namespace ir
