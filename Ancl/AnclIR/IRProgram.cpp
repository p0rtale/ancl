#include <Ancl/AnclIR/IRProgram.hpp>


namespace ir {

IRProgram::IRProgram(const std::vector<GlobalVariable*>& globalVars,
            const std::vector<Function*>& functions) {
    init(globalVars, functions);
}

void IRProgram::AddGlobalVar(GlobalVariable* globalVar) {
    m_GlobalVarList.push_back(globalVar);
    m_GlobalVarMap[globalVar->GetName()] = globalVar;
}

bool IRProgram::HasGlobalVar(const std::string& name) const {
    return m_GlobalVarMap.contains(name);
}

std::vector<GlobalVariable*> IRProgram::GetGlobalVars() const {
    return m_GlobalVarList;
}

GlobalVariable* IRProgram::GetGlobalVar(const std::string& name) const {
    return m_GlobalVarMap.at(name);
}

void IRProgram::AddFunction(Function* function) {
    m_FunctionList.push_back(function);
    m_FunctionMap[function->GetName()] = function;
}

bool IRProgram::HasFunction(const std::string& name) const {
    return m_FunctionMap.contains(name);
}

std::vector<Function*> IRProgram::GetFunctions() const {
    return m_FunctionList;
}

Function* IRProgram::GetFunction(const std::string& name) const {
    return m_FunctionMap.at(name);
}

void IRProgram::init(const std::vector<GlobalVariable*>& globalVars,
                     const std::vector<Function*>& functions) {
    for (auto* globalVar : globalVars) {
        AddGlobalVar(globalVar);
    }
    for (auto* function : functions) {
        AddFunction(function);
    }
}

}  // namespace ir
