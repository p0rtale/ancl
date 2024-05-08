#pragma once

#include <Ancl/CodeGen/Target/Base/Machine.hpp>
#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>


namespace gen::target {

class Legalizer {
public:
    Legalizer(TargetMachine* targetMachine)
        : m_TargetMachine(targetMachine) {}

    virtual void LegalizeMul(MInstruction* instruction) = 0;
    virtual void LegalizeDiv(MInstruction* instruction) = 0;
    virtual void LegalizeRem(MInstruction* instruction) = 0;
    virtual void LegalizeAdd(MInstruction* instruction) = 0;
    virtual void LegalizeSub(MInstruction* instruction) = 0;

    virtual void LegalizeShift(MInstruction* instruction) = 0;
    virtual void LegalizeAnd(MInstruction* instruction) = 0;
    virtual void LegalizeXor(MInstruction* instruction) = 0;
    virtual void LegalizeOr(MInstruction* instruction) = 0;

    virtual void LegalizeCmp(MInstruction* instruction) = 0;

    virtual void LegalizeTrunc(MInstruction* instruction) = 0;
    virtual void LegalizeExt(MInstruction* instruction) = 0;

    virtual void LegalizeLoad(MInstruction* instruction) = 0;
    virtual void LegalizeStore(MInstruction* instruction) = 0;

    void Legalize(MInstruction* instruction) {
        // TODO: ...
    }

private:
    TargetMachine* m_TargetMachine;
};

}  // namespace target
