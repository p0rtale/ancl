#pragma once

#include <string>

#include <Ancl/AnclIR/Value.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class Function;


class Parameter: public Value {
public:
    Parameter(const std::string& name, Type* type, Function* function,
             bool isImplicit = false)
            : Value(type), m_Function(function), m_IsImplicit(isImplicit) {
        SetName(name);
    }

    bool IsImplicit() const {
        return m_IsImplicit;
    }

    Function* GetFunction() const {
        return m_Function;
    }

private:
    Function* m_Function;

    bool m_IsImplicit = false;
};

}  // namespace ir
