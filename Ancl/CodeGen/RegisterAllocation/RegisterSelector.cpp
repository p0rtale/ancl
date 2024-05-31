#include <Ancl/CodeGen/RegisterAllocation/RegisterSelector.hpp>

#include <cassert>
#include <vector>


namespace gen {

RegisterSelector::RegisterSelector(target::RegisterSet* registerSet)
    : m_RegisterSet(registerSet) {}

void RegisterSelector::Init(bool isFloatClass) {
    m_RegClassesMap.clear();

    std::vector<uint64_t> registerClasses;
    if (isFloatClass) {
        registerClasses = m_RegisterSet->GetFPClasses();
    } else {
        registerClasses = m_RegisterSet->GetGPClasses();
    }

    for (uint64_t regClass : registerClasses) {
        RegClassInfo regClassInfo;
        for (const target::Register& targetReg : m_RegisterSet->GetRegisters(regClass)) {
            regClassInfo.FreeRegisters.insert(targetReg.GetNumber());
        }
        m_RegClassesMap[regClass] = std::move(regClassInfo);
    }
}

target::Register RegisterSelector::SelectRegister(const target::Register& reg) {
    if (IsActiveRegister(reg)) {
        return target::Register{};  // Return invalid register
    }

    activateRegister(reg);
    activateParentRegisters(reg);
    acticateSubRegisters(reg);

    return reg;
}

target::Register RegisterSelector::SelectRegisterByClass(uint64_t regClass) {
    RegClassInfo& regClassInfo = m_RegClassesMap[regClass];
    if (!regClassInfo.PriorityPairedRegisters.empty()) {
        uint64_t regNumber = *regClassInfo.PriorityPairedRegisters.begin();
        return SelectRegister(m_RegisterSet->GetRegister(regNumber));
    }
    if (!regClassInfo.FreeRegisters.empty()) {
        uint64_t regNumber = *regClassInfo.FreeRegisters.begin();
        return SelectRegister(m_RegisterSet->GetRegister(regNumber));
    }
    return target::Register{};  // Return invalid register
}

void RegisterSelector::FreeRegister(const target::Register& reg) {
    assert(IsActiveRegister(reg));

    deactivateRegister(reg);
    deactivateParentRegisters(reg);
    deacticateSubRegisters(reg);
}

bool RegisterSelector::IsActiveRegister(const target::Register& reg) {
    uint64_t regClass = m_RegisterSet->GetRegisterClass(reg);
    return !m_RegClassesMap[regClass].FreeRegisters.contains(reg.GetNumber());
}

void RegisterSelector::activateRegister(const target::Register& reg) {
    uint64_t regClass = m_RegisterSet->GetRegisterClass(reg);
    uint64_t regNumber = reg.GetNumber();

    if (reg.HasPairedRegister()) {
        uint64_t pairedRegNumber = reg.GetPairedRegNumber();
        if (m_RegClassesMap[regClass].FreeRegisters.contains(pairedRegNumber)) {
            m_RegClassesMap[regClass].PriorityPairedRegisters.insert(pairedRegNumber);
        } else {  // Otherwise the paired register is already in priority
            // Not anymore
            m_RegClassesMap[regClass].PriorityPairedRegisters.erase(regNumber);
        }
    }

    m_RegClassesMap[regClass].FreeRegisters.erase(regNumber);
}

void RegisterSelector::activateParentRegisters(const target::Register& reg) {
    uint64_t parentRegNumber = reg.GetParentRegNumber();
    while (parentRegNumber) {
        target::Register parentReg = m_RegisterSet->GetRegister(parentRegNumber);
        activateRegister(parentReg);
        parentRegNumber = parentReg.GetParentRegNumber();
    }
}

void RegisterSelector::acticateSubRegisters(const target::Register& reg) {
    for (uint64_t subRegNumber : reg.GetSubRegNumbers()) {
        const target::Register& subReg = m_RegisterSet->GetRegister(subRegNumber);
        activateRegister(subReg);
        acticateSubRegisters(m_RegisterSet->GetRegister(subRegNumber));
    }
}

void RegisterSelector::deactivateRegister(const target::Register& reg) {
    uint64_t regClass = m_RegisterSet->GetRegisterClass(reg);
    uint64_t regNumber = reg.GetNumber();

    if (reg.HasPairedRegister()) {
        uint64_t pairedRegNumber = reg.GetPairedRegNumber();
        if (m_RegClassesMap[regClass].FreeRegisters.contains(pairedRegNumber)) {
            // Paired register is already in priority
            m_RegClassesMap[regClass].PriorityPairedRegisters.erase(pairedRegNumber);
        } else {  // Otherwise we give it priority
            m_RegClassesMap[regClass].PriorityPairedRegisters.insert(regNumber);
        }
    }

    m_RegClassesMap[regClass].FreeRegisters.insert(regNumber);
}

void RegisterSelector::deactivateParentRegisters(const target::Register& reg) {
    uint64_t parentRegNumber = reg.GetParentRegNumber();
    while (parentRegNumber) {
        target::Register parentReg = m_RegisterSet->GetRegister(parentRegNumber);
        deactivateRegister(parentReg);
        parentRegNumber = parentReg.GetParentRegNumber();
    }
}

void RegisterSelector::deacticateSubRegisters(const target::Register& reg) {
    for (uint64_t subRegNumber : reg.GetSubRegNumbers()) {
        const target::Register& subReg = m_RegisterSet->GetRegister(subRegNumber);
        deactivateRegister(subReg);
        deacticateSubRegisters(m_RegisterSet->GetRegister(subRegNumber));
    }
}

}  // namespace gen
