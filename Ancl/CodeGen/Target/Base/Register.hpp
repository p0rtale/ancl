#pragma once

#include <cstdint>
#include <string>
#include <vector>


namespace gen::target {

class Register {
public:
    Register() = default;
    explicit Register(uint64_t number): m_Number(number) {}

    Register(const std::string& name, uint64_t number, uint64_t bytes,
             const std::vector<uint64_t>& subRegs)
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

    uint64_t GetNumber() const {
        return m_Number;
    }

    uint64_t GetBytes() const {
        return m_Bytes;
    }

    void SetParentRegNumber(uint64_t number) {
        m_ParentRegNumber = number;
    }

    uint64_t GetParentRegNumber() const {
        return m_ParentRegNumber;
    }

    std::vector<uint64_t> GetSubRegNumbers() const {
        return m_SubRegNumbers;
    }

    void SetPairedRegNumber(uint64_t number) {
        m_PairedRegNumber = number;
    }

    uint64_t GetPairedRegNumber() const {
        return m_PairedRegNumber;
    }

    bool HasPairedRegister() const {
        return m_PairedRegNumber;
    }

private:
    std::string m_Name;

    bool m_IsFloat = false;

    uint64_t m_Number = 0;
    uint64_t m_Bytes = 0;

    uint64_t m_ParentRegNumber = 0;
    std::vector<uint64_t> m_SubRegNumbers;

    uint64_t m_PairedRegNumber = 0;
};

}  // namespace target
