#pragma once

#include <set>
#include <unordered_map>

#include <Ancl/CodeGen/Target/Base/RegisterSet.hpp>


namespace gen {

class RegisterSelector {
public:
    RegisterSelector(target::RegisterSet* registerSet);

    void Init(bool isFloatClass);

    target::Register SelectRegister(const target::Register& reg);

    target::Register SelectRegisterByClass(uint64_t regClass);

    void FreeRegister(const target::Register& reg);

    bool IsActiveRegister(const target::Register& reg);

private:
    void activateRegister(const target::Register& reg);
    void activateParentRegisters(const target::Register& reg);
    void acticateSubRegisters(const target::Register& reg);

    void deactivateRegister(const target::Register& reg);
    void deactivateParentRegisters(const target::Register& reg);
    void deacticateSubRegisters(const target::Register& reg);

private:
    target::RegisterSet* m_RegisterSet = nullptr;

    struct RegClassInfo {
        std::set<uint64_t> FreeRegisters;
        std::set<uint64_t> PriorityPairedRegisters;
    };

    std::unordered_map<uint64_t, RegClassInfo> m_RegClassesMap;
};

}  // namespace gen
