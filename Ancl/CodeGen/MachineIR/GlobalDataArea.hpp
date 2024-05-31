#pragma once

#include <cstdint>
#include <vector>
#include <string>


namespace gen {

class GlobalDataArea {
public:
    enum class DataType {
        kNone,

        kZero,
        kByte, kShort,
        kInt, kQuad,

        // AT&T ?
        kFloat, kDouble,

        kString,

        kLabel,
    };

    struct Slot {
        DataType Type;

        // TODO: generalize?
        std::string Init;
    };

public:
    GlobalDataArea(const std::string& name);

    std::vector<Slot> GetSlots() const;

    void SetConst();
    bool IsConst() const;

    bool IsLocal() const;
    void SetLocal();

    bool IsInitialized() const;

    void AddIntegerSlot(uint64_t bytes, uint64_t init);
    void AddLabelSlot(uint64_t bytes, const std::string& label);
    void AddStringSlot(const std::string& init);
    void AddDoubleSlot(double init);
    void AddFloatSlot(float init);

    std::string GetName() const;

    size_t GetSize() const;

private:
    DataType getIntegerTypeFromBytes(uint64_t bytes);

private:
    std::string m_Name;

    bool m_IsLocal = false;
    bool m_IsConst = false;

    size_t m_Size;

    std::vector<Slot> m_Slots;
};

}  // namespace gen
