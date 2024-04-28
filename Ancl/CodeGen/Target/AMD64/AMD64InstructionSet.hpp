#pragma once

#include <unordered_map>

#include <Ancl/CodeGen/Target/Base/InstructionSet.hpp>
#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.hpp>


namespace target::amd64 {

enum OpClass: uint {
    INVALID = 0,
    GR, GR8, GR16, GR32, GR64,
    FR,
    IMM, IMM8, IMM16, IMM32, IMM64,
};

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
        IDIV_R, IDIV_M, DIV_R, DIV_M,

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
        initInstructionsMap();
    }

    TargetInstruction GetInstruction(TInstrCode code) const override {
        return m_TInstructionsMap.at(code);
    }

private:
    void initInstructionsMap() {
        auto grOp = TOperandClass{GR};
        auto frOp = TOperandClass{FR};
        auto immOp = TOperandClass{IMM};

        // Intel: BaseReg + ScaleImm * IndexReg + DispImm
        // AT&T:  DispImm(BaseReg, IndexReg, ScaleImm)
        auto memOp = TOperandClass{/*BaseReg=*/GR, /*ScaleImm=*/IMM, /*IndexReg=*/GR, /*DispImm=*/IMM};

        auto instructions = std::vector<TargetInstruction>{
            {IMUL_RR,  {grOp, grOp},  "imul"}, {IMUL_RM,  {grOp, memOp}, "imul"},
            {IMUL_RI,  {grOp, immOp}, "imul"},
            // {IMUL_RMI, {grOp, memOp, immOp},  "imul"},

            {MULSS_RR,  {grOp, grOp},  "mulss"}, {MULSS_RM,  {grOp, memOp}, "mulss"},
            {MULSD_RR,  {grOp, grOp},  "mulsd"}, {MULSD_RM,  {grOp, memOp}, "mulsd"},

            {IDIV_R, {grOp},  "idiv"}, {IDIV_M, {memOp}, "idiv"},
            {DIV_R,  {grOp},  "div"}, {DIV_M,  {memOp}, "div"},

            {DIVSS_RR, {grOp, grOp},  "divss"}, {DIVSS_RM, {grOp, memOp}, "divss"},
            {DIVSD_RR, {grOp, grOp},  "divsd"}, {DIVSD_RM, {grOp, memOp}, "divsd"},

            {ADD_RR, {grOp,  grOp},  "add"}, {ADD_RM, {grOp,  memOp}, "add"},
            {ADD_MR, {memOp, grOp},  "add"}, {ADD_RI, {grOp,  immOp}, "add"},
            {ADD_MI, {memOp, immOp}, "add"},

            {ADDSS_RR, {grOp, grOp},  "addss"}, {ADDSS_RM, {grOp, memOp}, "addss"},
            {ADDSD_RR, {grOp, grOp},  "addsd"}, {ADDSD_RM, {grOp, memOp}, "addsd"},

            {SUB_RR, {grOp,  grOp},  "sub"}, {SUB_RM, {grOp,  memOp}, "sub"},
            {SUB_MR, {memOp, grOp},  "sub"}, {SUB_RI, {grOp,  immOp}, "sub"},
            {SUB_MI, {memOp, immOp}, "sub"},

            {SUBSS_RR, {grOp, grOp},  "subss"}, {SUBSS_RM, {grOp, memOp}, "subss"},
            {SUBSD_RR, {grOp, grOp},  "subsd"}, {SUBSD_RM, {grOp, memOp}, "subsd"},

            // TODO: Consider CL register when selecting instructions
            {SHL_RCL, {grOp, grOp},  "shl"}, {SHL_RI,  {grOp, immOp}, "shl"},
            {SHR_RCL, {grOp, grOp},  "shr"}, {SHR_RI,  {grOp, immOp}, "shr"},
            {SAR_RCL, {grOp, grOp},  "sar"}, {SAR_RI,  {grOp, immOp}, "sar"},

            {AND_RR, {grOp,  grOp},  "and"}, {AND_RM, {grOp,  memOp}, "and"},
            {AND_MR, {memOp, grOp},  "and"}, {AND_RI, {grOp,  immOp}, "and"},
            {AND_MI, {memOp, immOp}, "and"},

            {XOR_RR, {grOp,  grOp},  "xor"}, {XOR_RM, {grOp,  memOp}, "xor"},
            {XOR_MR, {memOp, grOp},  "xor"}, {XOR_RI, {grOp,  immOp}, "xor"},
            {XOR_MI, {memOp, immOp}, "xor"},

            {OR_RR, {grOp,  grOp},  "or"}, {OR_RM, {grOp,  memOp}, "or"},
            {OR_MR, {memOp, grOp},  "or"}, {OR_RI, {grOp,  immOp}, "or"},
            {OR_MI, {memOp, immOp}, "or"},

            {CMP_RR, {grOp,  grOp},  "cmp"}, {CMP_RM, {grOp,  memOp}, "cmp"},
            {CMP_MR, {memOp, grOp},  "cmp"}, {CMP_RI, {grOp,  immOp}, "cmp"},
            {CMP_MI, {memOp, immOp}, "cmp"},

            {UCOMISS_RR, {grOp,  grOp},  "ucomiss"}, {UCOMISS_RM, {grOp,  memOp}, "ucomiss"},

            // TODO: Memory operand support
            {SETE,  {grOp}, "sete"}, {SETNE, {grOp}, "setne"},
            {SETL,  {grOp}, "setl"}, {SETLE, {grOp}, "setle"},
            {SETG,  {grOp}, "setg"}, {SETGE, {grOp}, "setge"},
            {SETB,  {grOp}, "setb"}, {SETBE, {grOp}, "setbe"},
            {SETA,  {grOp}, "seta"}, {SETAE, {grOp}, "setae"},

            {MOVZX_RR, {grOp, grOp},  "movz"}, {MOVZX_RM, {grOp, memOp}, "movz"},
            {MOVSX_RR, {grOp, grOp},  "movs"}, {MOVSX_RM, {grOp, memOp}, "movs"},

            {CVTSI2SS_RR, {grOp, grOp},  "cvtsi2ss"}, {CVTSI2SS_RM, {grOp, memOp}, "cvtsi2ss"},
            {CVTSI2SD_RR, {grOp, grOp},  "cvtsi2sd"}, {CVTSI2SD_RM, {grOp, memOp}, "cvtsi2sd"},

            {CVTTSS2SI_RR, {grOp, grOp},  "cvttss2si"}, {CVTTSS2SI_RM, {grOp, memOp}, "cvttss2si"},
            {CVTTSD2SI_RR, {grOp, grOp},  "cvttsd2si"}, {CVTTSD2SI_RM, {grOp, memOp}, "cvttsd2si"},

            {CVTSS2SD_RR, {grOp, grOp},  "cvtss2sd"}, {CVTSS2SD_RM, {grOp, memOp}, "cvtss2sd"},
            {CVTSD2SS_RR, {grOp, grOp},  "cvtsd2ss"}, {CVTSD2SS_RM, {grOp, memOp}, "cvtsd2ss"},

            {CALL, {}, "call"}, {CALL_R, {grOp}, "call"}, {CALL_M, {memOp}, "call"},

            {JMP, {}, "jmp"}, {JMP_R, {grOp}, "jmp"},

            {JE,  {}, "je"},  {JNE, {}, "jne"}, {JL,  {}, "jl"},  {JLE, {}, "jle"}, {JG,  {}, "jg"},
            {JGE, {}, "jge"}, {JB,  {}, "jb"},  {JBE, {}, "jbe"}, {JA,  {}, "ja"},  {JAE, {}, "jae"},

            {RET, {}, "ret"},

            {MOV_RR, {grOp,  grOp},  "mov"}, {MOV_RM, {grOp, memOp}, "mov"},
            {MOV_MR, {memOp, grOp},  "mov"}, {MOV_RI, {grOp, }, "mov"},
            {MOV_MI, {memOp, immOp}, "mov"},
    
            {MOVSS_RR, {grOp,  grOp}, "movss"}, {MOVSS_RM, {grOp, memOp}, "movss"},
            {MOVSS_MR, {memOp, grOp}, "movss"},
            {MOVSD_RR, {grOp, grOp},  "movsd"}, {MOVSD_RM, {grOp, memOp}, "movsd"},
            {MOVSD_MR, {memOp, grOp}, "movsd"},

            {LEA, {grOp, memOp}, "lea"},

            {PUSH_R, {grOp}, "push"}, {PUSH_M, {memOp}, "push"}, {PUSH_I, {immOp}, "push"},

            {POP_R, {grOp}, "pop"}, {POP_M, {memOp}, "pop"}
        };

        for (const auto& instr : instructions) {
            m_TInstructionsMap[instr.GetInstructionCode()] = instr;
        }
    }

private:
    TInstrMap m_TInstructionsMap;
};

}  // namespace amd64
