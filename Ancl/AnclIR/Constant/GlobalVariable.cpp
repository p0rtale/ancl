#include <Ancl/AnclIR/Constant/GlobalVariable.hpp>


namespace ir {

GlobalVariable::GlobalVariable(Type* type, LinkageType linkage,
                               const std::string& name)
        : GlobalValue(type, linkage) {
    SetName(name);
}

void GlobalVariable::SetInit(Constant* init) {
    m_Init = std::vector{init};
}

Constant* GlobalVariable::GetInit() const {
    return m_Init.at(0);
}

bool GlobalVariable::HasInit() const {
    return !m_Init.empty();
}

void GlobalVariable::SetInitString(const std::string& str) {
    m_IsInitString = true;
    m_InitString = str;
}

std::string GlobalVariable::GetInitString() const {
    return m_InitString;
}

bool GlobalVariable::IsInitString() const {
    return m_IsInitString;
}

void GlobalVariable::SetInitList(const std::vector<Constant*>& init) {
    m_IsInitList = true;
    m_Init = init;
}

std::vector<Constant*> GlobalVariable::GetInitList() const {
    return m_Init;
}

bool GlobalVariable::IsInitList() const {
    return m_IsInitList;
}

void GlobalVariable::SetInitVariable(GlobalVariable* variable) {
    m_InitVariable = variable;
    m_IsInitVariable = true;
}

GlobalVariable* GlobalVariable::GetInitVariable() {
    return m_InitVariable;
}

bool GlobalVariable::IsInitVariable() const {
    return m_IsInitVariable;
}

bool GlobalVariable::IsConst() const {
    return m_IsConst;
}

void GlobalVariable::SetConst(bool isConst) {
    m_IsConst = isConst;
}

}  // namespace ir
