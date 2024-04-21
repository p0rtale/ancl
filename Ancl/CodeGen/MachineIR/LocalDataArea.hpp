#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include <unordered_map>

#include <Ancl/DataLayout/Alignment.hpp>


namespace gen {

class LocalDataArea {
public:
    LocalDataArea() = default;

    uint GetSize() const {
        return m_Size;
    }

    size_t GetSlotsNumber() const {
        return m_Slots.size();
    }

    bool HasSlot(uint vreg) const {
        return m_VRegToSlotPosition.contains(vreg);
    }

    uint GetSlotSize(uint vreg) const {
        assert(HasSlot(vreg));

        auto position = m_VRegToSlotPosition.at(vreg);
        auto slot = m_Slots[position.Index];
        return slot.Size;
    }

    void AddSlot(uint vreg, uint size, uint align) {
        m_Slots.push_back(Slot{size, align});
        m_VRegToSlotPosition[vreg] = SlotPosition{
            .Index = m_Slots.size() - 1,
            .Offset = m_Size,
        };
        alignStack(size, align);
    }

    uint GetSlotOffset(uint vreg) const {
        assert(HasSlot(vreg));

        auto position = m_VRegToSlotPosition.at(vreg);
        return position.Offset;
    }

private:
    void alignStack(uint size, uint align) {
        m_Size = ir::Alignment::Align(m_Size, align);
        m_Size += size;
    }

    struct Slot {
        uint Size;
        uint Align;
    };

    struct SlotPosition {
        size_t Index;
        uint Offset;
    };

private:
    uint m_Size = 0;

    std::vector<Slot> m_Slots;
    std::unordered_map<uint, SlotPosition> m_VRegToSlotPosition;
};

}  // namespace gen
