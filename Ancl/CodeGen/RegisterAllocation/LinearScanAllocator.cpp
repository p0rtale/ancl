#include <Ancl/CodeGen/RegisterAllocation/LinearScanAllocator.hpp>

#include <Ancl/CodeGen/Selection/InstructionSelector.hpp>

#include <cassert>
#include <ranges>


namespace gen {

LinearScanAllocator::LinearScanAllocator(MFunction& function,
                                         target::TargetMachine* targetMachine,
                                         bool isFloatClass)
    : m_Function(function),
      m_TargetMachine(targetMachine),
      m_RegisterSet(m_TargetMachine->GetRegisterSet()),
      m_IsFloatClass(isFloatClass),
      m_RegisterSelector(m_RegisterSet) {}

void LinearScanAllocator::Allocate() {
    m_RegisterSelector.Init(m_IsFloatClass);
    allocateForFunction();
}

void LinearScanAllocator::computeLiveRanges() {
    target::TargetABI* targetABI = m_TargetMachine->GetABI();
    
    for (const target::Register& targetReg : targetABI->GetCalleeSavedRegisters()) {
        addPhysicalLiveRange(targetReg, /*start=*/0, /*end=*/kMaxLine);
    }

    std::unordered_map<uint64_t, LiveRange> virtualLiveRangesInfo;

    uint64_t line = 1;
    for (auto& basicBlock : m_Function.GetBasicBlocks()) {
        for (MInstruction& instruction : basicBlock->GetInstructions()) {
            if (instruction.IsCall()) {  // Kill caller-saved registers
                for (const target::Register& targetReg : targetABI->GetCallerSavedRegisters()) {
                    addPhysicalLiveRange(targetReg, /*start=*/line, /*end=*/line);
                }
            }

            if (instruction.IsRegMov() || instruction.IsRegToSubreg() || instruction.IsSubregToReg()) {
                m_LineToCopyInstruction[line] = &instruction;
            }

            // [EAX, EDX] <- DIV r/m <- [EAX, EDX]
            for (const target::Register& targetReg : instruction.GetImplicitRegDefinitions()) {
                addPhysicalLiveRange(targetReg, /*start=*/line, /*end=*/line);
            }
            for (const target::Register& targetReg : instruction.GetImplicitRegUses()) {
                addPhysicalLiveRange(targetReg, /*start=*/line, /*end=*/line);
            }

            for (MOperand& operand : instruction.GetOperands()) {
                if (operand.IsPRegister()) {
                    addPhysicalLiveRange(operand.GetRegister(), /*start=*/line, /*end=*/line);
                    continue;
                }

                if (!operand.IsVRegister() || !operand.GetRegister()) {
                    continue;
                }

                MType regType = operand.GetType();
                if (regType.IsFloat() != m_IsFloatClass) {
                    continue;
                }

                uint64_t vregNumber = operand.GetRegister();
                if (!m_OperandsMap.contains(vregNumber)) {
                    m_OperandsMap[vregNumber] = &operand;
                }

                if (virtualLiveRangesInfo.contains(vregNumber)) {
                    LiveRange& liveRange = virtualLiveRangesInfo[vregNumber];
                    liveRange.End = line;
                } else {
                    virtualLiveRangesInfo[vregNumber] = LiveRange{
                        .Start = line,
                        .End = line,
                        .Number = vregNumber,
                        .IsVirtual = true,
                    };
                }
            }

            ++line;
        }
    }

    for (const LiveRange& liveRange : std::views::values(virtualLiveRangesInfo)) {
        m_OrderedLiveRanges.insert({liveRange.Start, liveRange});
    }
}

void LinearScanAllocator::computeInterferencesWithPhysicalRegisters() {
    std::multimap<uint64_t, LiveRange*> activeLiveRanges;  // Ordered by End

    for (LiveRange& liveRange : std::views::values(m_OrderedLiveRanges)) {
        for (auto it = activeLiveRanges.begin(); it != activeLiveRanges.end();) {
            LiveRange* active = it->second;
            if (active->End >= liveRange.Start) {
                break;
            }
            it = activeLiveRanges.erase(it);
        }

        // All have intersections with LR for callee saved registers.
        // This will be taken into account later.
        if (!liveRange.IsVirtual && !isEntireFunctionLR(liveRange)) {
            for (auto& pair : activeLiveRanges) {
                LiveRange* active = pair.second;
                active->PRConflictsSet.insert(liveRange.Number);
            }
        }

        activeLiveRanges.insert({liveRange.End, &liveRange});
    }
}

void LinearScanAllocator::coalesceLiveRanges() {
    using TLiveRangeIt = std::multimap<uint64_t, LiveRange>::iterator;

    std::multimap<uint64_t, TLiveRangeIt> activePhysLiveRanges;
    std::multimap<uint64_t, TLiveRangeIt> activeVirtualLiveRanges;
    for (auto liveRangeIt = m_OrderedLiveRanges.begin(); liveRangeIt != m_OrderedLiveRanges.end(); ++liveRangeIt) {
        auto& liveRange = liveRangeIt->second;

        // Update active physical Live Ranges with coalescing
        for (auto activeIt = activePhysLiveRanges.begin(); activeIt != activePhysLiveRanges.end();) {
            auto& active = activeIt->second->second;
            if (active.End > liveRange.Start) {
                break;
            }
            if (active.End == liveRange.Start && isMovInstruction(liveRange.Start)) {
                assert(liveRange.IsVirtual);

                uint64_t point = liveRange.Start;
                if (!liveRange.PRConflictsSet.contains(active.Number)) {
                    // PLR | VLR  -->  PLR
                    liveRange.Start = active.Start;
                    liveRange.IsVirtual = false;

                    assignPhysicalRegister(liveRange.Number, active.Number);
                    liveRange.Number = active.Number;

                    m_OrderedLiveRanges.erase(activeIt->second);
                    // activePhysLiveRanges.insert({liveRange.End, liveRangeIt});
                }
            }
            activeIt = activePhysLiveRanges.erase(activeIt);
        }

        // Update active virtual Live Ranges with coalescing
        for (auto activeIt = activeVirtualLiveRanges.begin(); activeIt != activeVirtualLiveRanges.end();) {
            auto& active = activeIt->second->second;
            if (active.End > liveRange.Start) {
                break;
            }
            if (active.End == liveRange.Start && isMovInstruction(liveRange.Start)) {
                uint64_t point = liveRange.Start;

                if (liveRange.IsVirtual) {
                    // VLR | VLR  -->  VLR
                    liveRange.Start = active.Start;

                    auto& activeConflicts = active.PRConflictsSet;
                    liveRange.PRConflictsSet.insert(activeConflicts.begin(), activeConflicts.end());

                    m_LiveRangeCopyMap[active.Number] = liveRange.Number;
                    m_OrderedLiveRanges.erase(activeIt->second);
                } else if (!active.PRConflictsSet.contains(active.Number)) {
                    // VLR | PLR  -->  PLR

                    liveRange.Start = active.Start;
                    assignPhysicalRegister(active.Number, liveRange.Number);

                    m_OrderedLiveRanges.erase(activeIt->second);
                    // activePhysLiveRanges.insert({liveRange.End, liveRangeIt});
                }
            }
            activeIt = activeVirtualLiveRanges.erase(activeIt);
        }

        if (liveRange.IsVirtual) {
            for (auto pair : activePhysLiveRanges) {
                LiveRange& active = pair.second->second;
                liveRange.PRConflictsSet.insert(active.Number);
            }
            activeVirtualLiveRanges.insert({liveRange.End, liveRangeIt});
        } else if (!isEntireFunctionLR(liveRange)) {
            activePhysLiveRanges.insert({liveRange.End, liveRangeIt});
        }
    }
}

void LinearScanAllocator::assignLocalData(LiveRange* spill) {
    uint64_t bytes = 0;
    if (spill->IsVirtual) {
        MOperand* spillOperand = m_OperandsMap[spill->Number];
        MType spillType = spillOperand->GetType();
        bytes = spillType.GetBytes();
        m_LiveRangeSpills.insert(spill->Number);
    } else {
        target::RegisterSet* registers = m_TargetMachine->GetRegisterSet();
        target::Register reg = registers->GetRegister(spill->Number);
        bytes = reg.GetBytes();
        m_PRegisterSpills.push_back(reg);
    }
    if (!m_Function.HasSlot(spill->Number)) {
        m_Function.AddLocalData(spill->Number, bytes, bytes);
    }
}

void LinearScanAllocator::spillLiveRange(LiveRange* liveRange,
                                         std::multimap<uint64_t, LiveRange*>& activeLiveRanges) {
    auto spillIt = --activeLiveRanges.end();
    LiveRange* spill = spillIt->second;
    if (spill->End > liveRange->End) {
        target::Register targetRegister = getAssignedRegister(spill->Number);
        MOperand* liveRangeOperand = m_OperandsMap[liveRange->Number];
        MType liveRangeType = liveRangeOperand->GetType();
        while (targetRegister.GetBytes() > liveRangeType.GetBytes()) {
            targetRegister = m_RegisterSet->GetRegister(targetRegister.GetSubRegNumbers().at(0));
        }
        assignPhysicalRegister(liveRange->Number, targetRegister);

        assignLocalData(spill);
        activeLiveRanges.erase(spillIt);
        activeLiveRanges.insert({liveRange->End, liveRange});
    } else {
        assignLocalData(liveRange);
    }
}

MBasicBlock::TInstructionIt LinearScanAllocator::spillForInstruction(MBasicBlock::TInstructionIt instrIt) {
    MBasicBlock* basicBlock = instrIt->GetBasicBlock();

    if (instrIt->IsDefinition()) {
        MInstruction::TOperandIt definition = instrIt->GetDefinition();
        uint64_t defRegNumber = definition->GetRegister();
        if (m_LiveRangeSpills.contains(defRegNumber)) {
            ANCL_CRITICAL("LinearScanAllocator: Virtual LR spill is not implemented");

            // TODO: ...
            // MInstruction storeInstr{MInstruction::OpType::kStore};
            // storeInstr.AddStackIndex(defRegNumber);
            // storeInstr.AddPhysicalRegister();
            // MInstruction targetStore = InstructionSelector::SelectInstruction(storeInstr, m_TargetMachine);
            // return basicBlock->InsertAfter(targetStore, instrIt);
        }
    }

    for (size_t i = 0; i < instrIt->GetUsesNumber(); ++i) {
        MInstruction::TOperandIt useOperand = instrIt->GetUse(i);
        uint64_t useRegNumber = useOperand->GetRegister();
        if (m_LiveRangeSpills.contains(useRegNumber)) {
            ANCL_CRITICAL("LinearScanAllocator: Virtual LR spill is not implemented");

            // TODO: ...
            // MInstruction loadInstr{MInstruction::OpType::kLoad};
            // loadInstr.AddPhysicalRegister();
            // loadInstr.AddStackIndex(useRegNumber);
            // MInstruction targetLoad = InstructionSelector::SelectInstruction(loadInstr, m_TargetMachine);
            // basicBlock->InsertBefore(targetLoad, instrIt);
            // return instrIt;
        }
    }

    return instrIt;
}

target::Register LinearScanAllocator::selectForLiveRange(const LiveRange& liveRange) {
    if (liveRange.IsVirtual) {
        // For each interference with PR occupy and restore the register
        std::vector<uint64_t> selectedPRegNumbers;
        for (uint64_t pregNumber : liveRange.PRConflictsSet) {
            target::Register preg = m_RegisterSet->GetRegister(pregNumber);
            if (!m_RegisterSelector.IsActiveRegister(preg)) {
                m_RegisterSelector.SelectRegister(preg);
                selectedPRegNumbers.push_back(pregNumber);
            }
        }

        MOperand* liveRangeOperand = m_OperandsMap[liveRange.Number];
        target::Register liveRangeRegister = m_RegisterSelector.SelectRegisterByClass(
                                                        liveRangeOperand->GetRegisterClass());

        for (uint64_t pregNumber : selectedPRegNumbers) {
            target::Register preg = m_RegisterSet->GetRegister(pregNumber);
            m_RegisterSelector.FreeRegister(preg);
        }

        return liveRangeRegister;
    }

    target::Register liveRangeRegister = m_RegisterSet->GetRegister(liveRange.Number);
    return m_RegisterSelector.SelectRegister(liveRangeRegister);
}

void LinearScanAllocator::allocateRegisters() {
    // TODO: Resolve possible conflict between callee-saved and direct PRs (Spill) 
    std::multimap<uint64_t, LiveRange*> activeLiveRanges;
    for (auto& liveRange : std::views::values(m_OrderedLiveRanges)) {
        // Interferences with physical LRs have already been taken into account
        if (!liveRange.IsVirtual && !isEntireFunctionLR(liveRange)) {
            continue;
        }

        target::Register liveRangeTargetRegister;
        bool isSelected = false;

        for (auto it = activeLiveRanges.begin(); it != activeLiveRanges.end();) {
            LiveRange* active = it->second;
            if (active->End > liveRange.Start) {
                break;
            }

            if (active->End == liveRange.Start) {
                uint64_t point = liveRange.Start;
                if (isSubregToRegInstruction(point)) {
                    // TODO: ...
                    MInstruction* subregToReg = getCopyInstruction(point);
                    const target::Register& subreg = getAssignedRegister(active->Number);
                    const target::Register& reg = m_RegisterSet->GetRegister(subreg.GetParentRegNumber());

                    liveRangeTargetRegister = reg;
                    isSelected = true;
                } else if (isRegToSubregInstruction(point)) {
                    MInstruction* regToSubreg = getCopyInstruction(point);
                    target::Register reg = getAssignedRegister(active->Number);
                    uint64_t regBytes = reg.GetBytes();

                    MInstruction::TOperandIt subregOperand = regToSubreg->GetDefinition();
                    MType subregType = subregOperand->GetType();
                    uint64_t subregBytes = subregType.GetBytes();

                    while (regBytes > subregBytes) {
                        // TODO: Select more carefully
                        reg = m_RegisterSet->GetRegister(reg.GetSubRegNumbers()[0]);
                        regBytes = reg.GetBytes();
                    }
                    assert(regBytes == subregBytes);

                    liveRangeTargetRegister = reg;
                    isSelected = true;
                }
            }

            it = activeLiveRanges.erase(it);
            m_RegisterSelector.FreeRegister(m_PRegistersMap[active->Number]);
        }

        if (isSelected) {
            liveRangeTargetRegister = m_RegisterSelector.SelectRegister(liveRangeTargetRegister);
        } else {
            liveRangeTargetRegister = selectForLiveRange(liveRange);
        }
        if (liveRangeTargetRegister.IsValid()) {
            assignPhysicalRegister(liveRange.Number, liveRangeTargetRegister);
            activeLiveRanges.insert({liveRange.End, &liveRange});
        } else {
            spillLiveRange(&liveRange, activeLiveRanges);
        }
    }
}

void LinearScanAllocator::spillPhysicalRegisters() {
    // For now we rely on the fact that the callee saved LRs are spilled before the virtual LRs
    // TODO: Handle spilling of other physical Live Ranges
    MBasicBlock* firstBlock = m_Function.GetFirstBasicBlock();
    MBasicBlock* lastBlock = m_Function.GetLastBasicBlock();
    assert(lastBlock->GetLastInstruction()->IsReturn());

    for (target::Register& reg : m_PRegisterSpills) {
        uint64_t regNumber = reg.GetNumber();

        MInstruction pushReg(MInstruction::OpType::kPush);
        pushReg.AddPhysicalRegister(reg);
        MInstruction targetPushReg = InstructionSelector::SelectInstruction(pushReg, m_TargetMachine);
        // MInstruction storeInstr{MInstruction::OpType::kStore};
        // storeInstr.AddStackIndex(regNumber);
        // storeInstr.AddPhysicalRegister(reg);
        firstBlock->AddInstructionToBegin(targetPushReg);

        MInstruction popReg(MInstruction::OpType::kPop);
        popReg.AddPhysicalRegister(reg);
        MInstruction targetPopReg = InstructionSelector::SelectInstruction(popReg, m_TargetMachine);
        // MInstruction loadInstr{MInstruction::OpType::kLoad};
        // loadInstr.AddPhysicalRegister(reg);
        // loadInstr.AddStackIndex(regNumber);
        lastBlock->InsertBeforeLastInstruction(targetPopReg);
    }
}

void LinearScanAllocator::allocateForFunction() {
    computeLiveRanges();
    computeInterferencesWithPhysicalRegisters();

    coalesceLiveRanges();

    allocateRegisters();

    spillPhysicalRegisters();

    for (auto& basicBlock : m_Function.GetBasicBlocks()) {
        auto& instructions = basicBlock->GetInstructions();
        for (auto instrIt = instructions.begin(); instrIt != instructions.end();) {
            for (auto& operand : instrIt->GetOperands()) {
                if (operand.IsVRegister()) {
                    uint64_t regNumber = operand.GetRegister();
                    // TODO: Simplify?
                    while (m_LiveRangeCopyMap.contains(regNumber)) {
                        regNumber = m_LiveRangeCopyMap[regNumber];
                    }
                    operand.SetRegister(regNumber);

                    if (m_PRegistersMap.contains(regNumber)) {
                        operand.SetVirtual(false);
                        operand.SetRegister(m_PRegistersMap[regNumber].GetNumber());
                    }
                }
            }

            bool toDelete = false;

            if (instrIt->IsRegToSubreg() || instrIt->IsSubregToReg()) {
                toDelete = true;
            } else if (instrIt->IsRegMov()) {
                MInstruction::TOperandIt toOperand = instrIt->GetOperand(0);
                MInstruction::TOperandIt fromOperand = instrIt->GetOperand(1);
                if (fromOperand->GetRegister() == toOperand->GetRegister()) {
                    toDelete = true;
                }
            } else {
                instrIt = spillForInstruction(instrIt);
            }

            if (toDelete) {
                instrIt = instructions.erase(instrIt);
            } else {
                ++instrIt;
            }
        }
    }
}

void LinearScanAllocator::addPhysicalLiveRange(const target::Register& preg, uint64_t start, uint64_t end) {
    addPhysicalLiveRange(preg.GetNumber(), start, end);
}

void LinearScanAllocator::addPhysicalLiveRange(uint64_t pregNumber, uint64_t start, uint64_t end) {
    const target::Register& preg = m_RegisterSet->GetRegister(pregNumber);
    if (preg.IsFloat() != m_IsFloatClass) {
        return;
    }

    m_OrderedLiveRanges.insert({start, LiveRange{
        .Start=start,
        .End=end,
        .Number=pregNumber,
        .IsVirtual=false,
    }});
}

}  // namespace gen
