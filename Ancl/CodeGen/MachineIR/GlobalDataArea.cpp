#include <Ancl/CodeGen/MachineIR/GlobalDataArea.hpp>

#include <cassert>


namespace gen {

GlobalDataArea::GlobalDataArea(const std::string& name)
    : m_Name(name) {}

std::vector<GlobalDataArea::Slot> GlobalDataArea::GetSlots() const {
    return m_Slots;
}

void GlobalDataArea::SetConst() {
    m_IsConst = true;
}

bool GlobalDataArea::IsConst() const {
    return m_IsConst;
}

bool GlobalDataArea::IsLocal() const {
    return m_IsLocal;
}

void GlobalDataArea::SetLocal() {
    m_IsLocal = true;
}

bool GlobalDataArea::IsInitialized() const {
    for (const Slot& slot : m_Slots) {
        if (slot.Type != DataType::kZero && slot.Init != "0") {
            return true;
        }
    }
    return false;
}

void GlobalDataArea::AddIntegerSlot(uint64_t bytes, uint64_t init) {
    DataType type = getIntegerTypeFromBytes(bytes);
    if (type == DataType::kNone) {
        assert(init == 0);
        type = DataType::kZero;
        init = bytes;
    }
    m_Slots.emplace_back(type, std::to_string(init));
    m_Size += bytes;
}

void GlobalDataArea::AddLabelSlot(uint64_t bytes, const std::string& label) {
    DataType type = getIntegerTypeFromBytes(bytes);
    assert(type != DataType::kNone);
    m_Slots.emplace_back(type, label);
    m_Size += bytes;
}

void GlobalDataArea::AddStringSlot(const std::string& init) {
    m_Slots.emplace_back(DataType::kString, init);
    m_Size += init.size() + 1;  // assume null-terminated string
}

void GlobalDataArea::AddDoubleSlot(double init) {
    m_Slots.emplace_back(DataType::kQuad, std::to_string(reinterpret_cast<uint64_t&>(init)));
    m_Size += 8;
}

void GlobalDataArea::AddFloatSlot(float init) {
    m_Slots.emplace_back(DataType::kInt, std::to_string(reinterpret_cast<uint32_t&>(init)));
    m_Size += 4;
}

std::string GlobalDataArea::GetName() const {
    return m_Name;
}

size_t GlobalDataArea::GetSize() const {
    return m_Size;
}

GlobalDataArea::DataType GlobalDataArea::getIntegerTypeFromBytes(uint64_t bytes) {
    switch (bytes) {
        case 1:
            return DataType::kByte;
        case 2:
            return DataType::kShort;
        case 4:
            return DataType::kInt;
        case 8:
            return DataType::kQuad;

        default:
            return DataType::kNone;
    }
}

}  // namespace gen
