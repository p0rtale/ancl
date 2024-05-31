#pragma once

#include <cstdlib>

#include <Ancl/CodeGen/Target/Base/ABI.hpp>
#include <Ancl/CodeGen/Target/Base/RegisterSet.hpp>


namespace gen::target::amd64 {

// https://uclibc.org/docs/psABI-x86_64.pdf

class AMD64TargetABI: public TargetABI {
public:
    AMD64TargetABI(uint64_t pointerSize, RegisterSet* regSet);

private:
    void setStackAlignment(uint64_t align);

    void setRedZoneSize(uint64_t size);

    void setMaxStructParamSize(uint64_t size);

    void setArgumentRegisters();

    void setReturnRegisters();

    void setCalleeSavedRegisters();
    void setCallerSavedRegisters();

    void setVectorNumberInfoRegister();

private:
    RegisterSet* m_RegSet = nullptr;
};

}  // namespace amd64
