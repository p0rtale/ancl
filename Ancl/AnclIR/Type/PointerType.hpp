#pragma once

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class IRProgram;

class PointerType: public Type {
public:
    PointerType(IRProgram& program, Type* subType);

    static PointerType* Create(Type* subType);

    Type* GetSubType() const;

private:
    Type* m_SubType;
};

}  // namespace ir
