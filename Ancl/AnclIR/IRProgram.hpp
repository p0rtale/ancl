#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <Ancl/AnclIR/Constant/GlobalVariable.hpp>
#include <Ancl/AnclIR/Constant/Function.hpp>
#include <Ancl/Tracker.hpp>


namespace ir {

class IRProgram {
public:
    IRProgram() = default;

    IRProgram(const std::vector<GlobalVariable*>& globalVars,
              const std::vector<Function*>& functions) {
        init(globalVars, functions);
    }

    void AddGlobalVar(GlobalVariable* globalVar) {
        m_GlobalVarList.push_back(globalVar);
        m_GlobalVarMap[globalVar->GetName()] = globalVar;
    }

    std::vector<GlobalVariable*> GetGlobalVars() const {
        return m_GlobalVarList;
    }

    bool HasGlobalVar(const std::string& name) const {
        return m_GlobalVarMap.contains(name);
    }

    GlobalVariable* GetGlobalVar(const std::string& name) const {
        return m_GlobalVarMap.at(name);
    }

    void AddFunction(Function* function) {
        m_FunctionList.push_back(function);
        m_FunctionMap[function->GetName()] = function;
    }

    std::vector<Function*> GetFunctions() const {
        return m_FunctionList;
    }

    bool HasFunction(const std::string& name) const {
        return m_FunctionMap.contains(name);
    }

    Function* GetFunction(const std::string& name) const {
        return m_FunctionMap.at(name);
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
    void init(const std::vector<GlobalVariable*>& globalVars,
              const std::vector<Function*>& functions) {
        for (auto* globalVar : globalVars) {
            AddGlobalVar(globalVar);
        }
        for (auto* function : functions) {
            AddFunction(function);
        }
    }

private:
    // TODO: simplify
    std::vector<GlobalVariable*> m_GlobalVarList;
    std::unordered_map<std::string, GlobalVariable*> m_GlobalVarMap;
    std::vector<Function*> m_FunctionList;
    std::unordered_map<std::string, Function*> m_FunctionMap;

    Tracker<Value> m_ValueTracker;
    Tracker<Type> m_TypeTracker;
};

}  // namespace ir
