#pragma once

#include <string>


namespace gen {

class Register {
public:
    Register(const std::string& name, uint number, uint bytes,
             bool isFloat = false)
        : m_Name(name), m_Number(number), m_Bytes(bytes),
          m_IsFloat(isFloat) {}

    std::string GetName() const {
        return m_Name;
    }

    uint GetBytes() const {
        return m_Bytes;
    }

    bool IsFloat() const {
        return m_IsFloat;
    }

private:
    std::string m_Name;

    uint m_Number = 0;
    uint m_Bytes = 0;

    // TODO: subregisters

    bool m_IsFloat = false;
};

}  // namespace gen
