#pragma once

#include <vector>

#include <Ancl/AnclIR/Constant/GlobalVariable.hpp>
#include <Ancl/AnclIR/Constant/Function.hpp>
#include <Ancl/Tracker.hpp>


namespace ir {

class IRProgram {
public:
    IRProgram() = default;

    IRProgram(const std::vector<GlobalVariable*>& globalVars,
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

    template<typename T, typename... Args>
    T* CreateValue(Args&&... args) {
        return m_ValueTracker.Allocate<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T* CreateType(Args&&... args) {
        return m_TypeTracker.Allocate<T>(std::forward<Args>(args)...);
    }

private:
    // TODO: symbol tables
    std::vector<GlobalVariable*> m_GlobalVars;
    std::vector<Function*> m_Functions;

    Tracker<Value> m_ValueTracker;
    Tracker<Type> m_TypeTracker;
};

}  // namespace ir
