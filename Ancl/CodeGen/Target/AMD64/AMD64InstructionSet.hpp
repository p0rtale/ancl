#pragma once

#include <unordered_map>

#include <Ancl/CodeGen/Target/Base/InstructionSet.hpp>


namespace target {

// https://www.felixcloutier.com/x86/

class AMD64InstructionSet: public InstructionSet {
public:
    using TInstrMap = std::unordered_map<TInstrCode, TargetInstruction>;

    // TODO: extend i8 and i16 to i32 in AnclIR
    // TODO: AVX instructions
    enum InstrCode: TInstrCode {
        INVALID = 0,

        // felixcloutier.com/x86/imul
        // TODO: IMUL R/M with implicit AX register
        IMUL_RR, IMUL_RM, IMUL_RI,

        // felixcloutier.com/x86/mulss
        // felixcloutier.com/x86/mulsd
        MULSS_RR, MULSS_RM,
        MULSD_RR, MULSD_RM,

        // felixcloutier.com/x86/idiv
        // felixcloutier.com/x86/div
        IDIV_RM, DIV_RM,

        // felixcloutier.com/x86/divss
        // felixcloutier.com/x86/divsd
        DIVSS_RR, DIVSS_RM,
        DIVSD_RR, DIVSD_RM,

        // felixcloutier.com/x86/add
        ADD_RR, ADD_RM, ADD_MR, ADD_RI, ADD_MI,

        // felixcloutier.com/x86/addss
        // felixcloutier.com/x86/addsd
        ADDSS_RR, ADDSS_RM,
        ADDSD_RR, ADDSD_RM,

        // felixcloutier.com/x86/sub
        SUB_RR, SUB_RM, SUB_MR, SUB_RI, SUB_MI,

        // felixcloutier.com/x86/subss
        // felixcloutier.com/x86/subsd
        SUBSS_RR, SUBSS_RM,
        SUBSD_RR, SUBSD_RM,

        // felixcloutier.com/x86/sal:sar:shl:shr
        // TODO: SHL, SHR, SAR,

        // felixcloutier.com/x86/and
        AND_RR, AND_RM, AND_MR, AND_RI, AND_MI,

        // felixcloutier.com/x86/xor
        XOR_RR, XOR_RM, XOR_MR, XOR_RI, XOR_MI,

        // felixcloutier.com/x86/or
        OR_RR, OR_RM, OR_MR, OR_RI, OR_MI,

        // felixcloutier.com/x86/cmp
        // TODO: ...
        // kCmp, kUCmp, kFCmp,

        // kITrunc, kFTrunc,
        // kZExt, kSExt, kFExt,
        // kFToUI, kFToSI,
        // kUIToF, kSIToF,
        // kPtrToI, kIToPtr,

        // kCall, kJump, kBranch, kRet,

        // kImmLoad, kMov, kFMov,

        // kLoad, kStore,
        // kStackAddress, kGlobalAddress,

        // kSextLoad, kZextLoad,
    };

public:
    AMD64InstructionSet() {}

    TargetInstruction GetInstruction(TInstrCode code) const override {
        return m_TInstructionsMap.at(code);
    }

private:
    TInstrMap m_TInstructionsMap;
};

}  // namespace target
