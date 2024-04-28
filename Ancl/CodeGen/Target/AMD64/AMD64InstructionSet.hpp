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
        // TODO: IMUL R/M with implicit AX register?
        IMUL_RR, IMUL_RM, IMUL_RI, IMUL_RMI,

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
        SHL_RCL, SHL_RI, SHR_RCL, SHR_RI, SAR_RCL, SAR_RI,

        // felixcloutier.com/x86/and
        AND_RR, AND_RM, AND_MR, AND_RI, AND_MI,

        // felixcloutier.com/x86/xor
        XOR_RR, XOR_RM, XOR_MR, XOR_RI, XOR_MI,

        // felixcloutier.com/x86/or
        OR_RR, OR_RM, OR_MR, OR_RI, OR_MI,

        // felixcloutier.com/x86/cmp
        // felixcloutier.com/x86/ucomiss  (floating cmp)
        CMP_RR, CMP_RM, CMP_MR, CMP_RI, CMP_MI,  
        UCOMISS_RR, UCOMISS_RM,

        // felixcloutier.com/x86/setcc
        SETE, SETNE,
        SETL, SETLE, SETG, SETGE,  // signed integer
        SETB, SETBE, SETA, SETAE,  // unsigned integer

        // felixcloutier.com/x86/movzx
        // TODO: from 8->64 to 8->32 
        MOVZX_RR, MOVZX_RM,

        // felixcloutier.com/x86/movsx:movsxd
        // TODO: MOVSXD for 32->64
        MOVSX_RR, MOVSX_RM,

        // felixcloutier.com/x86/cvtsi2ss (integer->float)
        // felixcloutier.com/x86/cvtsi2sd (integer->double)
        CVTSI2SS_RR, CVTSI2SS_RM,
        CVTSI2SD_RR, CVTSI2SD_RM,

        // felixcloutier.com/x86/cvttss2si (float->integer)
        // felixcloutier.com/x86/cvttsd2si (double->integer)
        CVTTSS2SI_RR, CVTTSS2SI_RM,
        CVTTSD2SI_RR, CVTTSD2SI_RM,

        // felixcloutier.com/x86/cvtss2sd (float->double)
        // felixcloutier.com/x86/cvtsd2ss (double->float)
        CVTSS2SD_RR, CVTSS2SD_RM,
        CVTSD2SS_RR, CVTSD2SS_RM,

        // felixcloutier.com/x86/call
        CALL, CALL_R, CALL_M,
        
        // felixcloutier.com/x86/jmp
        JMP, JMP_R,

        // felixcloutier.com/x86/jcc
        JE, JNE,
        JL, JLE, JG, JGE,  // signed integer
        JB, JBE, JA, JAE,  // unsigned integer or float

        // felixcloutier.com/x86/ret
        RET,

        // felixcloutier.com/x86/mov
        MOV_RR, MOV_RM, MOV_MR, MOV_RI, MOV_MI,

        // felixcloutier.com/x86/movss  (floating move)
        // felixcloutier.com/x86/movsd  (double floating move)
        MOVSS_RR, MOVSS_RM, MOVSS_MR,
        MOVSD_RR, MOVSD_RM, MOVSD_MR,
    
        // felixcloutier.com/x86/lea
        LEA,

        // felixcloutier.com/x86/push
        PUSH_R, PUSH_M, PUSH_I,

        // felixcloutier.com/x86/pop
        POP_R, POP_M,
    };

public:
    AMD64InstructionSet() {
        
    }

    TargetInstruction GetInstruction(TInstrCode code) const override {
        return m_TInstructionsMap.at(code);
    }

private:
    TInstrMap m_TInstructionsMap;
};

}  // namespace target
