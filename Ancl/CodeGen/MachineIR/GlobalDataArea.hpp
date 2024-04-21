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
    };

public:
    GlobalDataArea(const std::string& name): m_Name(name) {}

    void AddSlot(uint bytes, uint init) {
        auto type = DataType::kNone; 
        switch (bytes) {
        case 1:
            type = DataType::kByte;
            break;
        case 2:
            type = DataType::kShort;
            break;
        case 4:
            type = DataType::kInt;
            break;
        case 8:
            type = DataType::kQuad;
            break;
        default:
            assert(init == 0);
            type = DataType::kZero;
            init = bytes;
        }
        m_Slots.emplace_back(type, std::to_string(init));
        m_Size += bytes;
    }

    void AddSlot(const std::string& init) {
        m_Slots.emplace_back(DataType::kString, init);
        m_Size += init.size() + 1;  // assume null-terminated string
    }

    void AddSlot(double init) {
        m_Slots.emplace_back(DataType::kDouble, std::to_string(init));
        m_Size += 8;
    }

    void AddSlot(float init) {
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
    struct Slot {
        DataType Type;

        // TODO: generalize?
        std::string Init;
    };

private:
    std::string m_Name;

    size_t m_Size;

    std::vector<Slot> m_Slots;
};

}  // namespace gen
