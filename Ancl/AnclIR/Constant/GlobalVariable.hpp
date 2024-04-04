#pragma once

#include <Ancl/AnclIR/Constant/GlobalValue.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Constant/Constant.hpp>


namespace ir {

class GlobalVariable: public GlobalValue {
public:
    GlobalVariable(Type* type, LinkageType linkage, const std::string& name)
            : GlobalValue(type, linkage) {
        SetName(name);
    }

    bool HasInit() const {
        return m_Init;
    }

    Constant* GetInit() const {
        return m_Init;
    }

    void SetInit(Constant* init) {
        m_Init = init;
    }

    bool IsConst() const {
        return m_IsConst;
    }

    void SetConst(bool isConst) {
        m_IsConst = isConst;
    }

private:
    Constant* m_Init = nullptr;
    bool m_IsConst = false;
};

}  // namespace ir
