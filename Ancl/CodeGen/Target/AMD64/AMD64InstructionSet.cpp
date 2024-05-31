#include <Ancl/CodeGen/Target/AMD64/AMD64InstructionSet.hpp>

#include <unordered_set>


namespace gen::target::amd64 {

AMD64InstructionSet::AMD64InstructionSet() {
    initInstructionsMap();
}

TargetInstruction AMD64InstructionSet::GetInstruction(TInstrCode code) const {
    return m_TInstructionsMap.at(code);
}

void AMD64InstructionSet::initInstructionsMap() {
    TOperandClass relOp{REL};
    TOperandClass grOp{GR};
    TOperandClass frOp{FR};
    TOperandClass immOp{IMM};

    // Intel: BaseReg + ScaleImm * IndexReg + DispImm
    // AT&T:  DispImm(BaseReg, IndexReg, ScaleImm)
    TOperandClass memOp{/*BaseReg=*/GR, /*ScaleImm=*/IMM, /*IndexReg=*/GR, /*DispImm=*/IMM};

    std::vector<TargetInstruction> instructions{
        {INVALID, {}, "INVALID"},

        {IMUL_RR,  {grOp, grOp}, "imul"}, {IMUL_RM, {grOp, memOp}, "imul"},
        {IMUL_RRI, {grOp, grOp, immOp}, "imul"}, {IMUL_RMI, {grOp, memOp, immOp}, "imul"},

        {MULSS_RR,  {frOp, frOp},  "mulss"}, {MULSS_RM,  {frOp, memOp}, "mulss"},
        {MULSD_RR,  {frOp, frOp},  "mulsd"}, {MULSD_RM,  {frOp, memOp}, "mulsd"},

        {IDIV_R, {grOp},  "idiv"}, {IDIV_M, {memOp}, "idiv"},
        {DIV_R,  {grOp},  "div"}, {DIV_M,  {memOp}, "div"},

        {DIVSS_RR, {frOp, frOp},  "divss"}, {DIVSS_RM, {frOp, memOp}, "divss"},
        {DIVSD_RR, {frOp, frOp},  "divsd"}, {DIVSD_RM, {frOp, memOp}, "divsd"},

        {ADD_RR, {grOp,  grOp},  "add"}, {ADD_RM, {grOp,  memOp}, "add"},
        {ADD_MR, {memOp, grOp},  "add"}, {ADD_RI, {grOp,  immOp}, "add"},
        {ADD_MI, {memOp, immOp}, "add"},

        {ADDSS_RR, {frOp, frOp},  "addss"}, {ADDSS_RM, {frOp, memOp}, "addss"},
        {ADDSD_RR, {frOp, frOp},  "addsd"}, {ADDSD_RM, {frOp, memOp}, "addsd"},

        {SUB_RR, {grOp,  grOp},  "sub"}, {SUB_RM, {grOp,  memOp}, "sub"},
        {SUB_MR, {memOp, grOp},  "sub"}, {SUB_RI, {grOp,  immOp}, "sub"},
        {SUB_MI, {memOp, immOp}, "sub"},

        {SUBSS_RR, {frOp, frOp},  "subss"}, {SUBSS_RM, {frOp, memOp}, "subss"},
        {SUBSD_RR, {frOp, frOp},  "subsd"}, {SUBSD_RM, {frOp, memOp}, "subsd"},

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

        {UCOMISS_RR, {frOp,  frOp},  "ucomiss"}, {UCOMISS_RM, {frOp,  memOp}, "ucomiss"},

        // TODO: Memory operand support
        {SETE,  {grOp}, "sete"}, {SETNE, {grOp}, "setne"},
        {SETL,  {grOp}, "setl"}, {SETLE, {grOp}, "setle"},
        {SETG,  {grOp}, "setg"}, {SETGE, {grOp}, "setge"},
        {SETB,  {grOp}, "setb"}, {SETBE, {grOp}, "setbe"},
        {SETA,  {grOp}, "seta"}, {SETAE, {grOp}, "setae"},

        {MOVZX_RR, {grOp, grOp},  "movz"}, {MOVZX_RM, {grOp, memOp}, "movz"},

        {CDQ, {}, "cdq"}, {CQO, {}, "cqo"},

        {MOVSX_RR, {grOp, grOp},  "movs"}, {MOVSX_RM, {grOp, memOp}, "movs"},
        {MOVSXD_RR, {grOp, grOp},  "movs"}, {MOVSXD_RM, {grOp, memOp}, "movs"},

        {CVTSI2SS_RR, {frOp, grOp},  "cvtsi2ss"}, {CVTSI2SS_RM, {frOp, memOp}, "cvtsi2ss"},
        {CVTSI2SD_RR, {frOp, grOp},  "cvtsi2sd"}, {CVTSI2SD_RM, {frOp, memOp}, "cvtsi2sd"},

        {CVTTSS2SI_RR, {grOp, frOp},  "cvttss2si"}, {CVTTSS2SI_RM, {grOp, memOp}, "cvttss2si"},
        {CVTTSD2SI_RR, {grOp, frOp},  "cvttsd2si"}, {CVTTSD2SI_RM, {grOp, memOp}, "cvttsd2si"},

        {CVTSS2SD_RR, {frOp, frOp},  "cvtss2sd"}, {CVTSS2SD_RM, {frOp, memOp}, "cvtss2sd"},
        {CVTSD2SS_RR, {frOp, frOp},  "cvtsd2ss"}, {CVTSD2SS_RM, {frOp, memOp}, "cvtsd2ss"},

        {CALL, {relOp}, "call"}, {CALL_R, {grOp}, "call"}, {CALL_M, {memOp}, "call"},

        {JMP, {relOp}, "jmp"}, {JMP_R, {grOp}, "jmp"},

        {JE,  {relOp}, "je"},  {JNE, {relOp}, "jne"},
        {JL,  {relOp}, "jl"},  {JLE, {relOp}, "jle"},
        {JG,  {relOp}, "jg"},  {JGE, {relOp}, "jge"},
        {JB,  {relOp}, "jb"},  {JBE, {relOp}, "jbe"},
        {JA,  {relOp}, "ja"},  {JAE, {relOp}, "jae"},

        {RET, {}, "ret"},

        {MOV_RR, {grOp,  grOp},  "mov"}, {MOV_RM, {grOp, memOp}, "mov"},
        {MOV_MR, {memOp, grOp},  "mov"}, {MOV_RI, {grOp, immOp}, "mov"},
        {MOV_MI, {memOp, immOp}, "mov"},

        {MOVSS_RR, {frOp,  frOp}, "movss"}, {MOVSS_RM, {frOp, memOp}, "movss"},
        {MOVSS_MR, {memOp, frOp}, "movss"},
        {MOVSD_RR, {frOp, frOp},  "movsd"}, {MOVSD_RM, {frOp, memOp}, "movsd"},
        {MOVSD_MR, {memOp, frOp}, "movsd"},

        {LEA, {grOp, memOp}, "lea"},

        {PUSH_R, {grOp}, "push"}, {PUSH_M, {memOp}, "push"}, {PUSH_I, {immOp}, "push"},

        {POP_R, {grOp}, "pop"}, {POP_M, {memOp}, "pop"}
    };

    // TODO: Destructiveness conditions? (definition + two ops in target instr)
    const std::unordered_set<TInstrCode> destructiveSet = {
        IMUL_RR, IMUL_RM,

        MULSS_RR, MULSS_RM,
        MULSD_RR, MULSD_RM,

        DIVSS_RR, DIVSS_RM,
        DIVSD_RR, DIVSD_RM,

        ADD_RR, ADD_RM, ADD_RI,

        ADDSS_RR, ADDSS_RM,
        ADDSD_RR, ADDSD_RM,

        SUB_RR, SUB_RM, SUB_RI,

        SUBSS_RR, SUBSS_RM,
        SUBSD_RR, SUBSD_RM,

        SHL_RCL, SHL_RI,
        SHR_RCL, SHR_RI,
        SAR_RCL, SAR_RI,

        AND_RR, AND_RM, AND_RI,

        XOR_RR, XOR_RM, XOR_RI,

        OR_RR, OR_RM, OR_RI,
    };

    for (TargetInstruction& instr : instructions) {
        TInstrCode code = instr.GetInstructionCode();
        if (destructiveSet.contains(code)) {
            instr.SetDestructive();
        }
        m_TInstructionsMap.emplace(code, instr);
    }
}

}  // namespace gen::target::amd64
