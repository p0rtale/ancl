#pragma once

#include <Ancl/Base.hpp>
#include <Ancl/CodeGen/Selection/SelectionTree.hpp>
#include <Ancl/CodeGen/Target/Base/ABI.hpp>
#include <Ancl/CodeGen/Target/Base/InstructionSet.hpp>
#include <Ancl/CodeGen/Target/Base/Legalizer.hpp>
#include <Ancl/CodeGen/Target/Base/RegisterSet.hpp>


namespace gen::target {

class TargetMachine {
public:
    TargetMachine() = default;
    virtual ~TargetMachine() = default;

    virtual void Select(SelectionTree& tree) = 0;

    RegisterSet* GetRegisterSet() {
        return m_RegisterSet.get();
    }

    InstructionSet* GetInstructionSet() {
        return m_InstructionSet.get();
    }

    TargetABI* GetABI() {
        return m_ABI.get();
    }

    Legalizer* GetLegalizer() {
        return m_Legalizer.get();
    }

    virtual uint64_t GetPointerByteSize() = 0;

protected:
    TScopePtr<RegisterSet> m_RegisterSet = nullptr;
    TScopePtr<InstructionSet> m_InstructionSet = nullptr;
    TScopePtr<TargetABI> m_ABI = nullptr;
    TScopePtr<Legalizer> m_Legalizer = nullptr;
};

}  // namespace gen::target
