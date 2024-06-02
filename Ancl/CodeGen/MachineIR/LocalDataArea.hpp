#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>


namespace gen {

class LocalDataArea {
public:
    struct Slot {
        uint64_t VReg;
        uint64_t Size;
        uint64_t Align;
    };

public:
    LocalDataArea() = default;

    uint64_t GetSize() const;

    size_t GetSlotsNumber() const;
    std::vector<Slot> GetSlots() const;

    bool HasSlot(uint64_t vreg) const;

    uint64_t GetAreaSize() const;

    uint64_t GetSlotSize(uint64_t vreg) const;

    void AddSlot(uint64_t vreg, uint64_t size, uint64_t align);

    // TODO: Negative offsets (spilled parameters)
    uint64_t GetSlotOffset(uint64_t vreg) const;

    void HandleNewSpilledCalleeSaved() {
        ++m_SpilledCalleeSavedNumber;
    }

    uint64_t GetSpilledCalleeSavedNumber() const {
        return m_SpilledCalleeSavedNumber;
    }

private:
    void alignStack(uint64_t size, uint64_t align);

    struct SlotPosition {
        size_t Index;
        uint64_t Offset;
    };

private:
    uint64_t m_Size = 0;

    uint64_t m_SpilledCalleeSavedNumber = 0;

    std::vector<Slot> m_Slots;
    std::unordered_map<uint64_t, SlotPosition> m_VRegToSlotPosition;
};

}  // namespace gen
