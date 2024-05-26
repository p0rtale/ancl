#pragma once

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
    GlobalDataArea(const std::string& name): m_Name(name) {}

    std::vector<Slot> GetSlots() const {
        return m_Slots;
    }

    void SetConst() {
        m_IsConst = true;
    }

    bool IsConst() const {
        return m_IsConst;
    }

    bool IsLocal() const {
        return m_IsLocal;
    }

    void SetLocal() {
        m_IsLocal = true;
    }

    bool IsInitialized() const {
        for (const Slot& slot : m_Slots) {
            if (slot.Type != DataType::kZero && slot.Init != "0") {
                return true;
            }
        }
        return false;
    }

    void AddIntegerSlot(uint bytes, uint init) {
        DataType type = getIntegerTypeFromBytes(bytes);
        if (type == DataType::kNone) {
            assert(init == 0);
            type = DataType::kZero;
            init = bytes;
        }
        m_Slots.emplace_back(type, std::to_string(init));
        m_Size += bytes;
    }

    void AddLabelSlot(uint bytes, const std::string& label) {
        DataType type = getIntegerTypeFromBytes(bytes);
        assert(type != DataType::kNone);
        m_Slots.emplace_back(type, label);
        m_Size += bytes;
    }

    void AddStringSlot(const std::string& init) {
        m_Slots.emplace_back(DataType::kString, init);
        m_Size += init.size() + 1;  // assume null-terminated string
    }

    void AddDoubleSlot(double init) {
        m_Slots.emplace_back(DataType::kDouble, std::to_string(init));
        m_Size += 8;
    }

    void AddFloatSlot(float init) {
        m_Slots.emplace_back(DataType::kFloat, std::to_string(init));
        m_Size += 4;
    }

    std::string GetName() const {
        return m_Name;
    }

    size_t GetSize() const {
        return m_Size;
    }

private:
    DataType getIntegerTypeFromBytes(uint bytes) {
        switch (bytes) {
        case 1:
            return DataType::kByte;
        case 2:
            return DataType::kShort;
        case 4:
            return DataType::kInt;
        case 8:
            return DataType::kQuad;
        }
        return DataType::kNone;
    }

private:
    std::string m_Name;

    bool m_IsLocal = false;
    bool m_IsConst = false;

    size_t m_Size;

    std::vector<Slot> m_Slots;
};

}  // namespace gen
