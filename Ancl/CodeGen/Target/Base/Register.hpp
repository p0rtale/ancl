#pragma once

#include <string>
#include <vector>


namespace gen::target {

class Register {
public:
    Register(uint number): m_Number(number) {}

    Register(const std::string& name, uint number, uint bytes,
             const std::vector<uint>& subRegs)
        : m_Name(name), m_Number(number), m_Bytes(bytes),
          m_SubRegNumbers(subRegs) {}

    std::string GetName() const {
        return m_Name;
    }

    uint GetNumber() const {
        return m_Number;
    }

    uint GetBytes() const {
        return m_Bytes;
    }

    std::vector<uint> GetSubRegNumbers() const {
        return m_SubRegNumbers;
    }

private:
    std::string m_Name;

    uint m_Number = 0;
    uint m_Bytes = 0;

    std::vector<uint> m_SubRegNumbers;
};

}  // namespace target
