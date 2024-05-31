#include <Ancl/CodeGen/Target/AMD64/AMD64ABI.hpp>

#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.hpp>


namespace gen::target::amd64 {

// https://uclibc.org/docs/psABI-x86_64.pdf

AMD64TargetABI::AMD64TargetABI(uint64_t pointerSize, RegisterSet* regSet)
        : m_RegSet(regSet)  {
    setStackAlignment(16);
    setRedZoneSize(128);
    setMaxStructParamSize(pointerSize * 2);

    setArgumentRegisters();
    setReturnRegisters();

    setCalleeSavedRegisters();
    setCallerSavedRegisters();

    setVectorNumberInfoRegister();
}

void AMD64TargetABI::setStackAlignment(uint64_t align) {
    m_StackAlignment = align;
}

void AMD64TargetABI::setRedZoneSize(uint64_t size) {
    m_RedZoneSize = size;
}

void AMD64TargetABI::setMaxStructParamSize(uint64_t size) {
    m_MaxStructParamSize = size;
}

void AMD64TargetABI::setArgumentRegisters() {
    // TODO: rewrite using std array
    auto rdi = m_RegSet->GetRegister(AMD64RegisterSet::RDI);
    auto rsi = m_RegSet->GetRegister(AMD64RegisterSet::RSI);
    auto rdx = m_RegSet->GetRegister(AMD64RegisterSet::RDX);
    auto rcx = m_RegSet->GetRegister(AMD64RegisterSet::RCX);
    auto r8 = m_RegSet->GetRegister(AMD64RegisterSet::R8);
    auto r9 = m_RegSet->GetRegister(AMD64RegisterSet::R9);
    m_IntArgumentRegisters = {rdi, rsi, rdx, rcx, r8, r9};

    auto xmm0 = m_RegSet->GetRegister(AMD64RegisterSet::XMM0);
    auto xmm1 = m_RegSet->GetRegister(AMD64RegisterSet::XMM1);
    auto xmm2 = m_RegSet->GetRegister(AMD64RegisterSet::XMM2);
    auto xmm3 = m_RegSet->GetRegister(AMD64RegisterSet::XMM3);
    auto xmm4 = m_RegSet->GetRegister(AMD64RegisterSet::XMM4);
    auto xmm5 = m_RegSet->GetRegister(AMD64RegisterSet::XMM5);
    auto xmm6 = m_RegSet->GetRegister(AMD64RegisterSet::XMM6);
    auto xmm7 = m_RegSet->GetRegister(AMD64RegisterSet::XMM7);
    m_FloatArgumentRegisters = {
        xmm0, xmm1, xmm2, xmm3,
        xmm4, xmm5, xmm6, xmm7,
    };
}

void AMD64TargetABI::setReturnRegisters() {
    auto rax = m_RegSet->GetRegister(AMD64RegisterSet::RAX);
    auto rdx = m_RegSet->GetRegister(AMD64RegisterSet::RDX);
    m_IntReturnRegisters = {rax, rdx};

    // TODO: Support YMM and ZMM registers
    auto xmm0 = m_RegSet->GetRegister(AMD64RegisterSet::XMM0);
    auto xmm1 = m_RegSet->GetRegister(AMD64RegisterSet::XMM1);
    m_FloatReturnRegisters = {xmm0, xmm1};
}

void AMD64TargetABI::setCalleeSavedRegisters() {
    auto rbx =  m_RegSet->GetRegister(AMD64RegisterSet::RBX);
    auto r12 =  m_RegSet->GetRegister(AMD64RegisterSet::R12);
    auto r13 =  m_RegSet->GetRegister(AMD64RegisterSet::R13);
    auto r14 =  m_RegSet->GetRegister(AMD64RegisterSet::R14);
    auto r15 =  m_RegSet->GetRegister(AMD64RegisterSet::R15);
    m_CalleeSavedRegisters = {rbx, r12, r13, r14, r15};
}

void AMD64TargetABI::setCallerSavedRegisters() {
    auto rax = m_RegSet->GetRegister(AMD64RegisterSet::RAX);
    auto rcx = m_RegSet->GetRegister(AMD64RegisterSet::RCX);
    auto rdx = m_RegSet->GetRegister(AMD64RegisterSet::RDX);
    auto rsi = m_RegSet->GetRegister(AMD64RegisterSet::RSI);
    auto rdi = m_RegSet->GetRegister(AMD64RegisterSet::RDI);
    m_CallerSavedRegisters = {rax, rcx, rdx, rsi, rdi};

    for (uint64_t reg = AMD64RegisterSet::XMM0; reg <= AMD64RegisterSet::XMM15; ++reg) {
        m_CallerSavedRegisters.push_back(m_RegSet->GetRegister(reg));
    }

    for (uint64_t reg = AMD64RegisterSet::R8; reg <= AMD64RegisterSet::R11; ++reg) {
        m_CallerSavedRegisters.push_back(m_RegSet->GetRegister(reg));
    }
}

void AMD64TargetABI::setVectorNumberInfoRegister() {
    m_VectorNumberInfoRegister = m_RegSet->GetRegister(AMD64RegisterSet::AL);
}

}  // namespace gen::target::amd64
