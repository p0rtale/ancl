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

    bool HasInit() const {
        return !m_Init.empty();
    }

    std::string GetInitString() const {
        return m_InitString;
    }

    bool IsStringInit() const {
        return m_IsStringInit;
    }

    Constant* GetInit() const {
        return m_Init.at(0);
    }

    std::vector<Constant*> GetInitList() const {
        return m_Init;
    }

    void SetInit(const std::string& str) {
        m_IsStringInit = true;
        m_InitString = str;
    }

    void SetInit(Constant* init) {
        m_Init = std::vector{init};
    }

    void SetInit(const std::vector<Constant*>& init) {
        m_Init = init;
    }

    bool IsConst() const {
        return m_IsConst;
    }

    void SetConst(bool isConst) {
        m_IsConst = isConst;
    }

private:
    std::vector<Constant*> m_Init;

    std::string m_InitString;
    bool m_IsStringInit = false;

    bool m_IsConst = false;
};

}  // namespace ir
