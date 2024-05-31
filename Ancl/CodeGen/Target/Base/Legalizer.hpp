#pragma once

#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>
#include <Ancl/CodeGen/Selection/SelectionTree.hpp>


namespace gen::target {

class TargetMachine;

class Legalizer {
public:
    Legalizer(TargetMachine* targetMachine)
        : m_TargetMachine(targetMachine) {}

    virtual ~Legalizer() = default;

    virtual bool IsLegalizationRequired(SelectionNode *instruction) = 0;

    virtual void LegalizeMul(SelectionNode* instruction) = 0;
    virtual void LegalizeDiv(SelectionNode* instruction) = 0;
    virtual void LegalizeRem(SelectionNode* instruction) = 0;
    virtual void LegalizeAdd(SelectionNode* instruction) = 0;
    virtual void LegalizeSub(SelectionNode* instruction) = 0;
    virtual void LegalizeShift(SelectionNode* instruction) = 0;
    virtual void LegalizeAnd(SelectionNode* instruction) = 0;
    virtual void LegalizeXor(SelectionNode* instruction) = 0;
    virtual void LegalizeOr(SelectionNode* instruction) = 0;
    virtual void LegalizeCmp(SelectionNode* instruction) = 0;
    virtual void LegalizeZExt(SelectionNode* instruction) = 0;

    void Legalize(SelectionNode* node) {
        if (!IsLegalizationRequired(node)) {
            return;
        }

        MInstruction& nodeInstruction = node->GetInstructionRef();
        switch (nodeInstruction.GetOpType()) {
            case MInstruction::OpType::kMul:
                return LegalizeMul(node); 

            case MInstruction::OpType::kSDiv:
            case MInstruction::OpType::kUDiv:
                return LegalizeDiv(node);

            case MInstruction::OpType::kSRem:
            case MInstruction::OpType::kURem:
                return LegalizeRem(node);

            case MInstruction::OpType::kAdd:
                return LegalizeAdd(node);

            case MInstruction::OpType::kSub:
                return LegalizeSub(node);

            case MInstruction::OpType::kShiftL:
            case MInstruction::OpType::kLShiftR:
            case MInstruction::OpType::kAShiftR:
                return LegalizeShift(node);
    
            case MInstruction::OpType::kAnd:
                return LegalizeAnd(node);

            case MInstruction::OpType::kXor:
                return LegalizeXor(node);

            case MInstruction::OpType::kOr:
                return LegalizeOr(node);

            case MInstruction::OpType::kCmp:
                return LegalizeCmp(node);

            case MInstruction::OpType::kZExt:
                return LegalizeZExt(node);

            default:
                ANCL_CRITICAL("It is impossible to legalize instruction");
                break;
        }
    }

protected:
    TargetMachine* m_TargetMachine;
};

}  // namespace gen::target
