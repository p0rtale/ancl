#pragma once

#include <string>

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class IRProgram;


class Value {
public:
    Value(Type* type): m_Type(type) {} 

    virtual ~Value() = default;

    bool HasName() const {
        return !m_Name.empty();
    }

    void SetName(const std::string& name) {
        m_Name = name;
    }

    std::string GetName() const {
        return m_Name;
    }

    Type* GetType() const {
        return m_Type;
    }

    IRProgram& GetProgram() const {
        return m_Type->GetProgram();
    }

private:
    // TODO: users of this value

    std::string m_Name;
    Type* m_Type;
};

}  // namespace ir
