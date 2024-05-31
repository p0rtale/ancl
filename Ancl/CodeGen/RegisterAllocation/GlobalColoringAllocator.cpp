#include <Ancl/CodeGen/RegisterAllocation/GlobalColoringAllocator.hpp>

#include <map>
#include <ranges>

#include <Ancl/CodeGen/Selection/InstructionSelector.hpp>


namespace gen {

GlobalColoringAllocator::GlobalColoringAllocator(MFunction& function,
                                                 target::TargetMachine* targetMachine, bool isFloatClass)
    : m_Function(function),
      m_TargetMachine(targetMachine),
      m_RegisterSet(m_TargetMachine->GetRegisterSet()),
      m_IsFloatClass(isFloatClass),
      m_RegisterSelector(m_RegisterSet) {}

void GlobalColoringAllocator::Allocate(LiveOUTPass& liveOutPass) {
    m_RegisterSelector.Init(m_IsFloatClass);

    CreateNodes();

    target::TargetABI* targetABI = m_TargetMachine->GetABI();
    for (const target::Register& targetReg : targetABI->GetCalleeSavedRegisters()) {
        if (targetReg.IsFloat() == m_IsFloatClass) {
            m_ActiveCalleeSavedRegisters.insert(targetReg.GetNumber());
        }
    }

    bool isAllocated = false;
    while (!isAllocated) {
        BuildGraph(liveOutPass);
        CoalesceCopies();
        Color();
        isAllocated = RenameAndSpill();
    }
}

void GlobalColoringAllocator::BuildGraph(LiveOUTPass& liveOutPass) {
    for (auto& basicBlock : m_Function.GetBasicBlocks()) {
        std::unordered_set<uint64_t> liveNowSet = liveOutPass.GetLiveOUT(basicBlock.get());

        for (MInstruction& instruction : basicBlock->GetInstructions() | std::views::reverse) {
            bool isCopy = false;
            if (instruction.IsRegMov() || instruction.IsRegToSubreg() || instruction.IsSubregToReg()) {
                MType type = instruction.GetDefinition()->GetType();
                if (type.IsFloat() == m_IsFloatClass) {
                    m_CopyList.push_back(&instruction);
                    isCopy = true;
                }
            }

            if (instruction.IsDefinition()) {
                MInstruction::TOperandIt defOperand = instruction.GetDefinition();
                if (defOperand->IsValidVRegister()) {
                    uint64_t defRegNumber = defOperand->GetRegister();
                    liveNowSet.erase(defRegNumber);

                    for (uint64_t activeRegNumber : liveNowSet) {
                        if (isCopy && activeRegNumber == instruction.GetUse(0)->GetRegister()) {
                            continue;
                        }
                        if (m_LiveRanges.contains(defRegNumber)) {
                            m_LiveRanges[defRegNumber].Interferences.insert(activeRegNumber);
                        }
                    }
                }
                if (defOperand->IsRegister()) {
                    // For physical registers too
                    for (uint64_t activeRegNumber : liveNowSet) {
                        if (isCopy && activeRegNumber == instruction.GetUse(0)->GetRegister()) {
                            continue;
                        }
                        if (m_LiveRanges.contains(activeRegNumber)) {
                            m_LiveRanges[activeRegNumber].Interferences.insert(defOperand->GetRegister());
                        }
                    }
                }
            }

            for (size_t i = 0; i < instruction.GetUsesNumber(); ++i) {
                MInstruction::TOperandIt use = instruction.GetUse(i);

                if (use->IsPRegister()) {
                    for (uint64_t activeRegNumber : liveNowSet) {
                        if (isCopy && activeRegNumber == instruction.GetDefinition()->GetRegister()) {
                            continue;
                        }
                        if (m_LiveRanges.contains(activeRegNumber)) {
                            m_LiveRanges[activeRegNumber].Interferences.insert(use->GetRegister());
                        }
                    }
                } else if (use->IsValidVRegister()) {
                    MType regType = use->GetType();
                    if (regType.IsFloat() != m_IsFloatClass) {
                        continue;
                    }

                    uint64_t vregNumber = use->GetRegister();
                    // if (!m_OperandsMap.contains(vregNumber)) {
                    //     m_OperandsMap[vregNumber] = &operand;
                    // }

                    liveNowSet.insert(vregNumber);
                }
            }

            target::TargetABI* targetABI = m_TargetMachine->GetABI();
            for (uint64_t activeRegNumber : liveNowSet) {
                if (instruction.IsCall()) {  // Kill caller-saved registers
                    for (const target::Register& targetReg : targetABI->GetCallerSavedRegisters()) {
                        if (targetReg.IsFloat() == m_IsFloatClass) {
                            m_LiveRanges[activeRegNumber].Interferences.insert(targetReg.GetNumber());
                        }
                    }
                }

                // [EAX, EDX] <- DIV r/m <- [EAX, EDX]
                for (const target::Register& targetReg : instruction.GetImplicitRegDefinitions()) {
                    if (targetReg.IsFloat() == m_IsFloatClass) {
                        m_LiveRanges[activeRegNumber].Interferences.insert(targetReg.GetNumber());
                    }
                }
                for (const target::Register& targetReg : instruction.GetImplicitRegUses()) {
                    if (targetReg.IsFloat() == m_IsFloatClass) {
                        m_LiveRanges[activeRegNumber].Interferences.insert(targetReg.GetNumber());
                    }
                }
            }
        }
    }
}

void GlobalColoringAllocator::CreateNodes() {
    for (auto& basicBlock : m_Function.GetBasicBlocks()) {
        for (MInstruction& instruction : basicBlock->GetInstructions()) {
            for (MOperand& operand : instruction.GetOperands()) {
                MType operandType = operand.GetType();
                if (operand.IsValidVRegister() && operandType.IsFloat() == m_IsFloatClass) {
                    uint64_t vregNumber = operand.GetRegister();
                    if (!m_LiveRanges.contains(vregNumber)) {
                        m_LiveRanges[vregNumber] = LiveRange{
                            .Number = vregNumber,
                            .IsVirtual = true,
                            .VirtualRegister = &operand,
                        };
                    }
                }
            }
        }
    }
}

void GlobalColoringAllocator::CoalesceCopies() {
    for (MInstruction* instruction : m_CopyList) {
        MInstruction::TOperandIt toOperand = instruction->GetOperand(0);
        MInstruction::TOperandIt fromOperand = instruction->GetOperand(1);

        uint64_t toRegNumber = toOperand->GetRegister();
        uint64_t fromRegNumber = fromOperand->GetRegister();

        // if (m_LiveRangeCopyMap.contains(toRegNumber) || m_LiveRangeCopyMap.contains(fromRegNumber)) {
        //     continue;
        // }
        while (m_LiveRangeCopyMap.contains(toRegNumber)) {
            toRegNumber = m_LiveRangeCopyMap[toRegNumber];
        }
        while (m_LiveRangeCopyMap.contains(fromRegNumber)) {
            fromRegNumber = m_LiveRangeCopyMap[fromRegNumber];
        }
        if (fromRegNumber == toRegNumber) {
            continue;
        }

        if (instruction->IsRegMov()) {
            if (toOperand->IsValidVRegister() && fromOperand->IsValidVRegister()) {
                if (!m_LiveRanges.at(toRegNumber).Interferences.contains(fromRegNumber)) {
                    auto& toRegInterferences = m_LiveRanges.at(toRegNumber).Interferences;
                    auto& fromRegInterferences = m_LiveRanges.at(fromRegNumber).Interferences;
                    for (uint64_t reg : fromRegInterferences) {
                        toRegInterferences.insert(reg);
                        if (m_LiveRanges.contains(reg)) {
                            m_LiveRanges.at(reg).Interferences.erase(fromRegNumber);
                            m_LiveRanges.at(reg).Interferences.insert(toRegNumber);
                        }
                    }
                    m_LiveRangeCopyMap[fromRegNumber] = toRegNumber;
                    m_LiveRanges.erase(fromRegNumber);
                }
            } else if (toOperand->IsPRegister() && fromOperand->IsValidVRegister()) {
                if (!m_LiveRanges.at(fromRegNumber).Interferences.contains(toRegNumber)) {
                    auto& fromRegInterferences = m_LiveRanges.at(fromRegNumber).Interferences;
                    for (uint64_t reg : fromRegInterferences) {
                        if (m_LiveRanges.contains(reg)) {
                            m_LiveRanges.at(reg).Interferences.insert(toOperand->GetRegister());
                        }
                    }

                    m_LiveRanges.at(fromRegNumber).Number = toRegNumber;
                    m_LiveRanges.at(fromRegNumber).IsVirtual = false;
                } 
            } else if (toOperand->IsValidVRegister() && fromOperand->IsPRegister()) {
                if (!m_LiveRanges.at(toRegNumber).Interferences.contains(fromRegNumber)) {
                    auto& toRegInterferences = m_LiveRanges.at(toRegNumber).Interferences;
                    for (uint64_t reg : toRegInterferences) {
                        if (m_LiveRanges.contains(reg)) {
                            m_LiveRanges.at(reg).Interferences.insert(fromOperand->GetRegister());
                        }
                    }

                    m_LiveRanges.at(toRegNumber).Number = fromRegNumber;
                    m_LiveRanges.at(toRegNumber).IsVirtual = false;
                }
            }
        } else if (instruction->IsSubregToReg()) {
            m_LiveRanges.at(fromRegNumber).ParentRegCasts.insert(toRegNumber);
            m_LiveRanges.at(toRegNumber).SubRegCasts.insert(fromRegNumber);
        } else if (instruction->IsRegToSubreg()) {
            m_LiveRanges.at(fromRegNumber).SubRegCasts.insert(toRegNumber);
            m_LiveRanges.at(toRegNumber).ParentRegCasts.insert(fromRegNumber);
        }
    }
}

void GlobalColoringAllocator::Color() {
    std::multimap<int64_t, uint64_t> liveRangesColorOrder;
    for (const auto& [number, liveRange] : m_LiveRanges) {
        if (!liveRange.IsVirtual) {
            continue;
        }

        int64_t spillMetric = 1 << 30;
        if (!liveRange.Interferences.empty()) {
            spillMetric = liveRange.SpillCost / (int64_t)liveRange.Interferences.size();
        }
        liveRangesColorOrder.insert({spillMetric, number});
    }

    for (auto [spillMetric, liveRangeNumber] : liveRangesColorOrder) {
        LiveRange& currentliveRange = m_LiveRanges[liveRangeNumber];
        if (!currentliveRange.IsVirtual) {
            continue;
        }

        std::vector<uint64_t> selectedPRegNumbers;
        for (uint64_t interfereNumber : currentliveRange.Interferences) {
            if (!m_LiveRanges.contains(interfereNumber)) {
                continue;
            }
            LiveRange& activeLiveRange = m_LiveRanges[interfereNumber];
            if (!activeLiveRange.IsVirtual) {
                auto selected = m_RegisterSelector.SelectRegister(m_RegisterSet->GetRegister(activeLiveRange.Number));
                if (selected.IsValid()) {
                    selectedPRegNumbers.push_back(activeLiveRange.Number);
                }
            }
        }

        for (uint64_t regNumber : m_ActiveCalleeSavedRegisters) {
            m_RegisterSelector.SelectRegister(m_RegisterSet->GetRegister(regNumber));
        }

        target::Register targetRegister = m_RegisterSelector.SelectRegisterByClass(
                                                currentliveRange.VirtualRegister->GetRegisterClass());

        if (targetRegister.IsValid()) {
            m_RegisterSelector.FreeRegister(targetRegister);
        }

        for (uint64_t regNumber : m_ActiveCalleeSavedRegisters) {
            m_RegisterSelector.FreeRegister(m_RegisterSet->GetRegister(regNumber));
        }

        for (uint64_t pregNumber : selectedPRegNumbers) {
            m_RegisterSelector.FreeRegister(m_RegisterSet->GetRegister(pregNumber));
        }

        bool hasActiveCalleeSavedRegs = !m_ActiveCalleeSavedRegisters.empty();
        if (!targetRegister.IsValid() && !hasActiveCalleeSavedRegs) {
            m_SpillSet.insert(currentliveRange.Number);
        } else {
            if (!targetRegister.IsValid()) {
                targetRegister = m_RegisterSet->GetRegister(*m_ActiveCalleeSavedRegisters.begin());
                m_ActiveCalleeSavedRegisters.erase(targetRegister.GetNumber());
            }

            currentliveRange.IsVirtual = false;
            currentliveRange.Number = targetRegister.GetNumber();

            MOperand* currentVirtualRegister = currentliveRange.VirtualRegister;
            uint64_t currentRegBytes = currentVirtualRegister->GetType().GetBytes();
            for (uint64_t subVRegNumber : currentliveRange.SubRegCasts) {
                if (!m_LiveRanges[subVRegNumber].IsVirtual) {
                    continue;
                }

                MOperand* subRegVirtualRegister = m_LiveRanges[subVRegNumber].VirtualRegister;
                uint64_t subRegBytes = subRegVirtualRegister->GetType().GetBytes();

                while (currentRegBytes > subRegBytes) {
                    targetRegister = m_RegisterSet->GetRegister(targetRegister.GetSubRegNumbers()[0]);
                    currentRegBytes = targetRegister.GetBytes();
                }

                m_LiveRanges[subVRegNumber].Number = targetRegister.GetNumber();
                m_LiveRanges[subVRegNumber].IsVirtual = false;
            }

            for (uint64_t parentVRegNumber : currentliveRange.ParentRegCasts) {
                if (!m_LiveRanges[parentVRegNumber].IsVirtual) {
                    continue;
                }

                targetRegister = m_RegisterSet->GetRegister(targetRegister.GetParentRegNumber());
                m_LiveRanges[parentVRegNumber].Number = targetRegister.GetNumber();
                m_LiveRanges[parentVRegNumber].IsVirtual = false;
            }
        }
    }
}


MBasicBlock::TInstructionIt GlobalColoringAllocator::SpillForInstruction(MBasicBlock::TInstructionIt instrIt) {
    MBasicBlock* basicBlock = instrIt->GetBasicBlock();

    if (instrIt->IsDefinition()) {
        MInstruction::TOperandIt definition = instrIt->GetDefinition();
        if (definition->IsValidVRegister() && m_SpillSet.contains(definition->GetRegister())) {
            m_SpillSet.erase(definition->GetRegister());

            MInstruction storeInstr{MInstruction::OpType::kStore};
            storeInstr.AddStackIndex(definition->GetRegister());
            storeInstr.AddOperand(*definition);
            MInstruction targetStore = InstructionSelector::SelectInstruction(storeInstr, m_TargetMachine);
            return basicBlock->InsertAfter(targetStore, instrIt);
        }
    }

    for (size_t i = 0; i < instrIt->GetUsesNumber(); ++i) {
        MInstruction::TOperandIt use = instrIt->GetUse(i);
        if (use->IsValidVRegister() && m_SpillSet.contains(use->GetRegister())) {
            m_SpillSet.erase(use->GetRegister());

            MInstruction loadInstr{MInstruction::OpType::kLoad};
            loadInstr.AddOperand(*use);
            loadInstr.AddStackIndex(use->GetRegister());
            MInstruction targetLoad = InstructionSelector::SelectInstruction(loadInstr, m_TargetMachine);
            basicBlock->InsertBefore(targetLoad, instrIt);
            return instrIt;
        }
    }

    return instrIt;
}

bool GlobalColoringAllocator::RenameAndSpill() {
    target::TargetABI* targetABI = m_TargetMachine->GetABI();
    for (const target::Register& calleeSavedReg : targetABI->GetCalleeSavedRegisters()) {
        if (calleeSavedReg.IsFloat() != m_IsFloatClass) {
            continue;
        }

        MBasicBlock* firstBlock = m_Function.GetFirstBasicBlock();
        MBasicBlock* lastBlock = m_Function.GetLastBasicBlock();
        if (!m_ActiveCalleeSavedRegisters.contains(calleeSavedReg.GetNumber())) {
            MInstruction pushReg(MInstruction::OpType::kPush);
            pushReg.AddPhysicalRegister(calleeSavedReg);
            MInstruction targetPushReg = InstructionSelector::SelectInstruction(pushReg, m_TargetMachine);
            firstBlock->AddInstructionToBegin(targetPushReg);

            MInstruction popReg(MInstruction::OpType::kPop);
            popReg.AddPhysicalRegister(calleeSavedReg);
            MInstruction targetPopReg = InstructionSelector::SelectInstruction(popReg, m_TargetMachine);
            lastBlock->InsertBeforeLastInstruction(targetPopReg);
        }
    }

    for (auto& basicBlock : m_Function.GetBasicBlocks()) {
        auto& instructions = basicBlock->GetInstructions();
        for (auto instrIt = instructions.begin(); instrIt != instructions.end();) {
            for (auto& operand : instrIt->GetOperands()) {
                if (operand.IsValidVRegister()) {
                    uint64_t regNumber = operand.GetRegister();
                    while (m_LiveRangeCopyMap.contains(regNumber)) {
                        regNumber = m_LiveRangeCopyMap[regNumber];
                    }
                    operand.SetRegister(regNumber);

                    if (m_LiveRanges.contains(regNumber)) {
                        operand.SetVirtual(false);
                        operand.SetRegister(m_LiveRanges[regNumber].Number);
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
                instrIt = SpillForInstruction(instrIt);
            }

            if (toDelete) {
                instrIt = instructions.erase(instrIt);
            } else {
                ++instrIt;
            }
        }
    }

    return m_SpillSet.empty();
}

}  // namespace gen