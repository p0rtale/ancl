#pragma once

#include <unordered_set>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <ranges>

#include <Ancl/CodeGen/RegisterAllocation/RegisterSelector.hpp>

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

// http://web.cs.ucla.edu/~palsberg/course/cs132/linearscan.pdf
class LinearScanAllocator {
public:
    LinearScanAllocator(MFunction& function,
                        target::TargetMachine* targetMachine, bool isFloatClass);

    void Allocate();

private:
    struct LiveRange {
        uint64_t Start = 0;
        uint64_t End = 0;

        uint64_t Number = 0;
        bool IsVirtual = true;

        std::unordered_set<uint64_t> PRConflictsSet;
    };

private:
    void computeLiveRanges();

    void computeInterferencesWithPhysicalRegisters();

    void coalesceLiveRanges();

    void assignLocalData(LiveRange* spill);

    void spillLiveRange(LiveRange* liveRange,
                        std::multimap<uint64_t, LiveRange*>& activeLiveRanges);

    MBasicBlock::TInstructionIt spillForInstruction(MBasicBlock::TInstructionIt instrIt);

    target::Register selectForLiveRange(const LiveRange& liveRange);

    void spillPhysicalRegisters();

    void allocateRegisters();
    void allocateForFunction();

    void addPhysicalLiveRange(const target::Register& preg, uint64_t start, uint64_t end);
    void addPhysicalLiveRange(uint64_t pregNumber, uint64_t start, uint64_t end);

    target::Register getAssignedRegister(uint64_t liveRangeNumber) {
        return m_PRegistersMap.at(liveRangeNumber);
    }

    bool isRegisterAssigned(uint64_t liveRangeNumber) {
        return m_PRegistersMap.contains(liveRangeNumber);
    }

    void assignPhysicalRegister(uint64_t liveRangeNumber, uint64_t pregNumber) {
        m_PRegistersMap[liveRangeNumber] = m_RegisterSet->GetRegister(pregNumber);
    }

    void assignPhysicalRegister(uint64_t liveRangeNumber, const target::Register& preg) {
        m_PRegistersMap[liveRangeNumber] = preg;
    }

    bool isEntireFunctionLR(const LiveRange& liveRange) {
        return liveRange.Start == 0 && liveRange.End == kMaxLine;
    }

    MInstruction* getCopyInstruction(uint64_t line) {
        return m_LineToCopyInstruction.at(line);
    }

    bool isCopyInstruction(uint64_t line) const {
        return m_LineToCopyInstruction.contains(line);
    }

    bool isSubregToRegInstruction(uint64_t line) const {
        if (!isCopyInstruction(line)) {
            return false;
        }
        return m_LineToCopyInstruction.at(line)->IsSubregToReg();
    }

    bool isRegToSubregInstruction(uint64_t line) const {
        if (!isCopyInstruction(line)) {
            return false;
        }
        return m_LineToCopyInstruction.at(line)->IsRegToSubreg();
    }

    bool isMovInstruction(uint64_t line) const {
        if (!isCopyInstruction(line)) {
            return false;
        }
        return m_LineToCopyInstruction.at(line)->IsRegMov();
    }

private:
    static constexpr unsigned int kMaxLine = 1e9; 

    // Ordered map from LR Start to LR
    std::multimap<uint64_t, LiveRange> m_OrderedLiveRanges;

    std::unordered_map<uint64_t, uint64_t> m_LiveRangeCopyMap;
    std::unordered_map<uint64_t, target::Register> m_PRegistersMap;

    // Map from VR Number to the corresponding operand
    std::unordered_map<uint64_t, MOperand*> m_OperandsMap;

    // Spill info
    std::unordered_set<uint64_t> m_LiveRangeSpills;
    std::vector<target::Register> m_PRegisterSpills;

    std::unordered_map<uint64_t, MInstruction*> m_LineToCopyInstruction;

    MFunction& m_Function;
    target::TargetMachine* m_TargetMachine = nullptr;
    target::RegisterSet* m_RegisterSet = nullptr;
    
    bool m_IsFloatClass = false;
    RegisterSelector m_RegisterSelector;
};

}  // namespace gen
