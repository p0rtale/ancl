#pragma once

#include <string>
#include <vector>


namespace gen::target {

class Register {
public:
    Register() = default;
    explicit Register(uint number): m_Number(number) {}

    Register(const std::string& name, uint number, uint bytes,
             const std::vector<uint>& subRegs)
        : m_Name(name), m_Number(number), m_Bytes(bytes),
          m_SubRegNumbers(subRegs) {}

    std::string GetName() const {
        return m_Name;
    }

    bool IsValid() const {
        return m_Number;
    }

    void SetFloat() {
        m_IsFloat = true;
    }

    bool IsFloat() const {
        return m_IsFloat;
    }

    uint GetNumber() const {
        return m_Number;
    }

    uint GetBytes() const {
        return m_Bytes;
    }

    void SetParentRegNumber(uint number) {
        m_ParentRegNumber = number;
    }

    uint GetParentRegNumber() const {
        return m_ParentRegNumber;
    }

    std::vector<uint> GetSubRegNumbers() const {
        return m_SubRegNumbers;
    }

    void SetPairedRegNumber(uint number) {
        m_PairedRegNumber = number;
    }

    uint GetPairedRegNumber() const {
        return m_PairedRegNumber;
    }

    bool HasPairedRegister() const {
        return m_PairedRegNumber;
    }

private:
    std::string m_Name;

    bool m_IsFloat = false;

    uint m_Number = 0;
    uint m_Bytes = 0;

    uint m_ParentRegNumber = 0;
    std::vector<uint> m_SubRegNumbers;

    uint m_PairedRegNumber = 0;
};

}  // namespace target
