#pragma once

#include <vector>

#include <Ancl/CodeGen/MachineIR/MOperand.hpp>
#include <Ancl/CodeGen/Selection/SelectionTree.hpp>

#include <Ancl/CodeGen/Target/AMD64/AMD64ABI.hpp>
#include <Ancl/CodeGen/Target/AMD64/AMD64InstructionSet.hpp>
#include <Ancl/CodeGen/Target/AMD64/AMD64Legalizer.hpp>
#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen::target::amd64 {

class AMD64TargetMachine: public TargetMachine {
public:
    AMD64TargetMachine() {
        m_RegisterSet = CreateScope<AMD64RegisterSet>();
        m_InstructionSet = CreateScope<AMD64InstructionSet>();
        m_ABI = CreateScope<AMD64TargetABI>(GetPointerByteSize(), m_RegisterSet.get());
        m_Legalizer = CreateScope<AMD64Legalizer>(this);
    }

    void Select(SelectionTree& tree) override;

    uint64_t GetPointerByteSize() override {
        return 8;
    }

private:
    void selectNode(SelectionNode* node);

    void selectMul(SelectionNode* node);
    void selectFMul(SelectionNode* node);

    void selectSDiv(SelectionNode* node);
    void selectUDiv(SelectionNode* node);
    void selectFDiv(SelectionNode* node);

    void selectSRem(SelectionNode* node);
    void selectURem(SelectionNode* node);

    void selectAdd(SelectionNode* node);
    void selectFAdd(SelectionNode* node);

    void selectSub(SelectionNode* node);
    void selectFSub(SelectionNode* node);

    void selectShiftL(SelectionNode* node);
    void selectLShiftR(SelectionNode* node);
    void selectAShiftR(SelectionNode* node);

    void selectAnd(SelectionNode* node);
    void selectXor(SelectionNode* node);
    void selectOr(SelectionNode* node);

    void selectCmp(SelectionNode* node);

    void selectITrunc(SelectionNode* node);
    void selectFTrunc(SelectionNode* node);
    void selectZExt(SelectionNode* node);
    void selectSExt(SelectionNode* node);
    void selectFExt(SelectionNode* node);
    void selectFToUI(SelectionNode* node);
    void selectFToSI(SelectionNode* node);
    void selectUIToF(SelectionNode* node);
    void selectSIToF(SelectionNode* node);

    void selectCall(SelectionNode* node);
    void selectJump(SelectionNode* node);
    void selectBranch(SelectionNode* node);
    void selectRet(SelectionNode* node);

    void selectMov(SelectionNode* node);
    void selectFMov(SelectionNode* node);

    void selectLoad(SelectionNode* node);
    void selectStore(SelectionNode* node);
    void selectStackAddress(SelectionNode* node);
    void selectGlobalAddress(SelectionNode* node);

    void selectMemberAddress(SelectionNode* node);

    void selectPush(SelectionNode* node);
    void selectPop(SelectionNode* node);

    void selectPhi(SelectionNode* node);

private:
    void selectSDivImpl(SelectionNode* node);
    void selectUDivImpl(SelectionNode* node);

    MInstruction selectRMImpl(SelectionNode* node);

    std::vector<MOperand> mergeMemberAddress(SelectionNode* node);
    void legalizeAddressScale(SelectionNode* node,
                              std::vector<gen::MOperand>& memoryOperands);
    std::vector<MOperand> selectMemoryOperand(SelectionNode* node);
    std::vector<MOperand> trySelectMemory(SelectionNode* node);

    void finalizeSelect(SelectionNode* node, std::vector<MInstruction>& targetInstructions);
    void finalizeSelect(SelectionNode* node, MInstruction& targetInstruction);
};

}  // namespace gen::target::amd64
