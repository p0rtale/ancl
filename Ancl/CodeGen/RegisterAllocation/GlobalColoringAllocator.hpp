#pragma once

#include <unordered_set>
#include <unordered_map>
#include <set>
#include <vector>

#include <Ancl/CodeGen/RegisterAllocation/LiveOutPass.hpp>
#include <Ancl/CodeGen/RegisterAllocation/RegisterSelector.hpp>

#include <Ancl/CodeGen/MachineIR/MFunction.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

class GlobalColoringAllocator {
public:
    GlobalColoringAllocator(MFunction& function,
                            target::TargetMachine* targetMachine, bool isFloatClass);

    void Allocate(LiveOUTPass& liveOutPass);

    void BuildGraph(LiveOUTPass& liveOutPass);
    void CreateNodes();
    void CoalesceCopies();
    void Color();

    MBasicBlock::TInstructionIt SpillForInstruction(MBasicBlock::TInstructionIt instrIt);
    bool RenameAndSpill();

private:
    void insertPRegisterInterference(uint64_t vregNumber, const target::Register& preg);
    void insertSubPRegsInterference(uint64_t vregNumber, const target::Register& preg);

    void handleSpillSlot(MInstruction::TOperandIt spillOperand);

    struct LiveRange {
        uint64_t Number = 0;
        std::unordered_set<uint64_t> Interferences;

        std::unordered_set<uint64_t> ParentRegCasts;
        std::unordered_set<uint64_t> SubRegCasts;

        int64_t SpillCost = 1;
        bool IsVirtual = true;

        MOperand* VirtualRegister = nullptr;
    };

    std::set<uint64_t> m_ActiveCalleeSavedRegisters;
    std::unordered_set<uint64_t> m_CalleeSavedSpillSet;

    std::unordered_set<uint64_t> m_SpillSet;
    std::unordered_map<uint64_t, uint64_t> m_LiveRangeCopyMap;

    std::unordered_map<uint64_t, LiveRange> m_LiveRanges;

    std::vector<MInstruction*> m_CopyList;

private:
    MFunction& m_Function;
    target::TargetMachine* m_TargetMachine = nullptr;
    target::RegisterSet* m_RegisterSet = nullptr;

    bool m_IsFloatClass = false;
    RegisterSelector m_RegisterSelector;
};

}  // namespace gen
