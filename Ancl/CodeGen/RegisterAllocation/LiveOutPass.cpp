#include <Ancl/CodeGen/RegisterAllocation/LiveOutPass.hpp>


namespace gen {

LiveOUTPass::LiveOUTPass(MFunction& function)
    : m_Function(function) {}

void LiveOUTPass::Run() {
    initSets();

    while (m_IsChanged) {
        m_IsChanged = false;
        m_Visited.clear();
        preorderPass(m_Function.GetFirstBasicBlock());
    }
}

std::unordered_set<uint64_t> LiveOUTPass::GetLiveOUT(MBasicBlock* basicBlock) {
    return m_BlocksLiveOUT[basicBlock]; 
}

void LiveOUTPass::initSets() {
    for (auto& basicBlock : m_Function.GetBasicBlocks()) {
        auto& upwardExposedSet = m_BlocksUEVar[basicBlock.get()];
        auto& varKillSet = m_BlocksVarKill[basicBlock.get()];

        for (MInstruction& instruction : basicBlock->GetInstructions()) {
            for (size_t i = 0; i < instruction.GetUsesNumber(); ++i) {
                if (instruction.IsPhi()) {
                    break;
                }
                MInstruction::TOperandIt useOperand = instruction.GetUse(i);
                if (!useOperand->IsVRegister()) {
                    continue;
                }
                if (!varKillSet.contains(useOperand->GetRegister())) {
                    upwardExposedSet.insert(useOperand->GetRegister());
                }
            }

            if (instruction.IsDefinition()) {
                MInstruction::TOperandIt defOperand = instruction.GetDefinition();
                if (defOperand->IsVRegister()) {
                    varKillSet.insert(defOperand->GetRegister());
                }
            }
        }
    }
}

void LiveOUTPass::preorderPass(MBasicBlock* basicBlock) {
    auto& liveOutSet = m_BlocksLiveOUT[basicBlock];

    m_Visited.insert(basicBlock);
    for (MBasicBlock* successor : basicBlock->GetSuccessors()) {
        size_t predIdx = 0;
        while (successor->GetPredecessor(predIdx) != basicBlock) {
            ++predIdx;
        }
        for (MInstruction& instruction : successor->GetInstructions()) {
            if (instruction.IsPhi()) {
                MInstruction::TOperandIt useOperand = instruction.GetUse(predIdx);
                if (useOperand->IsVRegister() && !useOperand->IsInvalidRegister()) {
                    liveOutSet.insert(useOperand->GetRegister());
                }
            }
        }

        if (!m_Visited.contains(successor)) {
            preorderPass(successor);
        }

        // TODO: Make it more efficient
        for (uint64_t number : m_BlocksUEVar[successor]) {
            if (!liveOutSet.contains(number)) {
                m_IsChanged = true;
                liveOutSet.insert(number);
            }
        }

        for (uint64_t number : m_BlocksLiveOUT[successor]) {
            if (!m_BlocksVarKill[successor].contains(number)) {
                if (!liveOutSet.contains(number)) {
                    m_IsChanged = true;
                    liveOutSet.insert(number);
                }
            }
        }
    }
}

}  // namespace gen