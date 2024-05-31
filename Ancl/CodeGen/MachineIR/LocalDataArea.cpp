#include <Ancl/CodeGen/MachineIR/LocalDataArea.hpp>

#include <cassert>

#include <Ancl/DataLayout/Alignment.hpp>


namespace gen {

uint64_t LocalDataArea::GetSize() const {
    return m_Size;
}

size_t LocalDataArea::GetSlotsNumber() const {
    return m_Slots.size();
}

std::vector<LocalDataArea::Slot> LocalDataArea::GetSlots() const {
    return m_Slots;
}

bool LocalDataArea::HasSlot(uint64_t vreg) const {
    return m_VRegToSlotPosition.contains(vreg);
}

uint64_t LocalDataArea::GetAreaSize() const {
    return m_Size;
}

uint64_t LocalDataArea::GetSlotSize(uint64_t vreg) const {
    assert(HasSlot(vreg));

    SlotPosition position = m_VRegToSlotPosition.at(vreg);
    Slot slot = m_Slots[position.Index];
    return slot.Size;
}

void LocalDataArea::AddSlot(uint64_t vreg, uint64_t size, uint64_t align) {
    m_Slots.push_back(Slot{vreg, size, align});

    alignStack(size, align);

    m_VRegToSlotPosition[vreg] = SlotPosition{
        .Index = m_Slots.size() - 1,
        .Offset = m_Size,
    };
}

uint64_t LocalDataArea::GetSlotOffset(uint64_t vreg) const {
    assert(HasSlot(vreg));

    SlotPosition position = m_VRegToSlotPosition.at(vreg);
    return position.Offset;
}

void LocalDataArea::alignStack(uint64_t size, uint64_t align) {
    m_Size += size;
    m_Size = ir::Alignment::Align(m_Size, align);
}

}  // namespace gen
