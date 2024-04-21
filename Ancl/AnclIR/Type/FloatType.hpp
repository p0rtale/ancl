#pragma once

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class IRProgram;

class FloatType: public Type {
public:
    enum class Kind {
        kNone = 0,
        kFloat,
        kDouble,
        kLongDouble,
    };

public:
    FloatType(IRProgram& program, Kind kind);

    static FloatType* Create(IRProgram& program, Kind kind);

    Kind GetKind() const;

private:
    Kind m_Kind = Kind::kNone;
};

}  // namespace ir
