#pragma once

#include <vector>

#include <Ancl/Base.hpp>
#include <Ancl/CodeGen/MachineIR/GlobalDataArea.hpp>
#include <Ancl/CodeGen/MachineIR/MFunction.hpp>


namespace gen {

class MIRProgram {
public:
    MIRProgram() = default;

    void AddGlobalDataArea(const GlobalDataArea& dataArea) {
        m_GlobalData.push_back(dataArea);
    }

    std::vector<GlobalDataArea>& GetGlobalData() {
        return m_GlobalData;
    }

    void AddFunction(TScopePtr<MFunction> function) {
        m_Functions.push_back(std::move(function));
    }

    std::vector<TScopePtr<MFunction>>& GetFunctions() {
        return m_Functions;
    }

    MFunction* GetLastFunction() {
        auto& function = m_Functions.back();
        return function.get();
    }

private:
    std::vector<GlobalDataArea> m_GlobalData;
    std::vector<TScopePtr<MFunction>> m_Functions;
};

}  // namespace gen
