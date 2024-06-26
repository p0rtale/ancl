#pragma once

#include <string>
#include <vector>

#include <Ancl/Base.hpp>

#include <Ancl/CodeGen/MachineIR/MBasicBlock.hpp>
#include <Ancl/CodeGen/MachineIR/LocalDataArea.hpp>
#include <Ancl/CodeGen/MachineIR/MType.hpp>


namespace gen {

class MFunction {
public:
    static constexpr uint64_t kFirstVirtualRegisterNumber = 1 << 16;

public:
    MFunction(const std::string& name, bool isStatic = false)
        : m_Name(name), m_IsStatic(isStatic) {}

    std::string GetName() const {
        return m_Name;
    }

    bool IsStatic() const {
        return m_IsStatic;
    }

    void SetStatic() {
        m_IsStatic = true;
    }

    void AddBasicBlock(TScopePtr<MBasicBlock> MBB) {
        m_BasicBlocks.push_back(std::move(MBB));
    }

    MBasicBlock* GetBasicBlock(size_t idx) {
        return m_BasicBlocks[idx].get();
    }

    std::vector<TScopePtr<MBasicBlock>>& GetBasicBlocks() {
        return m_BasicBlocks;
    }

    MBasicBlock* GetFirstBasicBlock() {
        return m_BasicBlocks.front().get();
    }

    MBasicBlock* GetLastBasicBlock() {
        return m_BasicBlocks.back().get();
    }

    LocalDataArea& GetLocalDataArea() {
        return m_LocalDataArea;
    }

    bool HasSlot(uint64_t vreg) const {
        return m_LocalDataArea.HasSlot(vreg);
    }

    void AddLocalData(uint64_t vreg, uint64_t size, uint64_t align) {
        m_LocalDataArea.AddSlot(vreg, size, align);
    }

    bool IsCaller() const {
        return m_IsCaller;
    }

    void SetCaller() {
        m_IsCaller = true;
    }

    bool IsVariadic() const {
        return m_IsVariadic;
    }

    void SetVariadic() {
        m_IsVariadic = true;
    }

    uint64_t NextVReg() {
        return m_NextVReg++;
    }

    bool IsVRegNumber(uint64_t number) const {
        return m_NextVReg >= kFirstVirtualRegisterNumber;
    }

private:
    std::string m_Name;

    bool m_IsStatic = false;
    bool m_IsCaller = false;

    bool m_IsVariadic = false;

    LocalDataArea m_LocalDataArea;
    std::vector<TScopePtr<MBasicBlock>> m_BasicBlocks;

    uint64_t m_NextVReg = kFirstVirtualRegisterNumber;
};

}  // namespace gen
