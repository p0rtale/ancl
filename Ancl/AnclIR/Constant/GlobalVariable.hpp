#pragma once

#include <string>
#include <vector>

#include <Ancl/AnclIR/Constant/Constant.hpp>
#include <Ancl/AnclIR/Constant/GlobalValue.hpp>

#include <Ancl/AnclIR/Type/Type.hpp>



namespace ir {

class GlobalVariable: public GlobalValue {
public:
    GlobalVariable(Type* type, LinkageType linkage, const std::string& name)
            : GlobalValue(type, linkage) {
        SetName(name);
    }

    void SetInit(Constant* init) {
        m_Init = std::vector{init};
    }

    Constant* GetInit() const {
        return m_Init.at(0);
    }

    bool HasInit() const {
        return !m_Init.empty();
    }

    void SetInitString(const std::string& str) {
        m_IsInitString = true;
        m_InitString = str;
    }

    std::string GetInitString() const {
        return m_InitString;
    }

    bool IsInitString() const {
        return m_IsInitString;
    }

    void SetInitList(const std::vector<Constant*>& init) {
        m_IsInitList = true;
        m_Init = init;
    }

    std::vector<Constant*> GetInitList() const {
        return m_Init;
    }

    bool IsInitList() const {
        return m_IsInitList;
    }

    void SetInitVariable(GlobalVariable* variable) {
        m_InitVariable = variable;
        m_IsInitVariable = true;
    }

    GlobalVariable* GetInitVariable() {
        return m_InitVariable;
    }

    bool IsInitVariable() const {
        return m_IsInitVariable;
    }

    bool IsConst() const {
        return m_IsConst;
    }

    void SetConst(bool isConst) {
        m_IsConst = isConst;
    }

private:
    std::vector<Constant*> m_Init;
    bool m_IsInitList = false;

    std::string m_InitString;
    bool m_IsInitString = false;

    GlobalVariable* m_InitVariable;
    bool m_IsInitVariable = false;

    bool m_IsConst = false;
};

}  // namespace ir
