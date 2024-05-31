#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <Ancl/AnclIR/Constant/Function.hpp>
#include <Ancl/AnclIR/Constant/GlobalVariable.hpp>
#include <Ancl/Tracker.hpp>


namespace ir {

class IRProgram {
public:
    IRProgram() = default;

    IRProgram(const std::vector<GlobalVariable*>& globalVars,
              const std::vector<Function*>& functions);

    void AddGlobalVar(GlobalVariable* globalVar);
    bool HasGlobalVar(const std::string& name) const;

    std::vector<GlobalVariable*> GetGlobalVars() const;
    GlobalVariable* GetGlobalVar(const std::string& name) const;

    void AddFunction(Function* function);
    bool HasFunction(const std::string& name) const;

    std::vector<Function*> GetFunctions() const;
    Function* GetFunction(const std::string& name) const;

    template <typename T, typename... Args>
    T* CreateValue(Args&&... args) {
        return m_ValueTracker.Allocate<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T* CreateType(Args&&... args) {
        return m_TypeTracker.Allocate<T>(std::forward<Args>(args)...);
    }

private:
    void init(const std::vector<GlobalVariable*>& globalVars,
              const std::vector<Function*>& functions);

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
