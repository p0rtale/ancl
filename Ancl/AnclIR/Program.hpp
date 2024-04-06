#pragma once

#include <vector>

#include <Ancl/AnclIR/Constant/GlobalVariable.hpp>
#include <Ancl/AnclIR/Constant/Function.hpp>


namespace ir {

class Program {
public:
    Program() = default;

    Program(const std::vector<GlobalVariable*>& globalVars,
            const std::vector<Function*>& functions)
        : m_GlobalVars(globalVars),
          m_Functions(functions) {}

    void AddGlobalVar(GlobalVariable* globalVar) {
        m_GlobalVars.push_back(globalVar);
    }

    std::vector<GlobalVariable*> GetGlobalVars() const {
        return m_GlobalVars;
    }

    void AddFunction(Function* function) {
        m_Functions.push_back(function);
    }

    std::vector<Function*> GetFunctions() const {
        return m_Functions;
    }

private:
    // TODO: symbol tables
    std::vector<GlobalVariable*> m_GlobalVars;
    std::vector<Function*> m_Functions;
};

}  // namespace ir
