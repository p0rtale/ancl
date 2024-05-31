#pragma once

#include <Ancl/CodeGen/Target/Base/Legalizer.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen::target::amd64 {

class AMD64Legalizer: public Legalizer {
public:
    AMD64Legalizer(TargetMachine* targetMachine);

    bool IsLegalizationRequired(SelectionNode* node) override;

    void LegalizeMul(SelectionNode* node) override;
    void LegalizeDiv(SelectionNode* node) override;
    void LegalizeRem(SelectionNode* node) override;
    void LegalizeAdd(SelectionNode* node) override;
    void LegalizeSub(SelectionNode* node) override;
    void LegalizeShift(SelectionNode* node) override;
    void LegalizeAnd(SelectionNode* node) override;
    void LegalizeXor(SelectionNode* node) override;
    void LegalizeOr(SelectionNode* node) override;
    void LegalizeCmp(SelectionNode* node) override;
    void LegalizeZExt(SelectionNode* node) override;

private:
    void legalizeDivRem(SelectionNode* node, bool isRem = false);

    void moveImmediateToEnd(SelectionNode* node);
    void materializeLeftImmediate(SelectionNode* node);

    MOperand materializeImmediate(const MOperand& operand, SelectionNode* node);
};

}  // namespace gen::target::amd64
