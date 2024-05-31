#pragma once

#include <string>
#include <vector>

#include <Ancl/AnclIR/Constant/Constant.hpp>
#include <Ancl/AnclIR/Constant/GlobalValue.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class GlobalVariable: public GlobalValue {
public:
    GlobalVariable(Type* type, LinkageType linkage, const std::string& name);

    void SetInit(Constant* init);
    Constant* GetInit() const;
    bool HasInit() const;

    void SetInitString(const std::string& str);
    std::string GetInitString() const;
    bool IsInitString() const;

    void SetInitList(const std::vector<Constant*>& init);
    std::vector<Constant*> GetInitList() const;
    bool IsInitList() const;

    void SetInitVariable(GlobalVariable* variable);
    GlobalVariable* GetInitVariable();
    bool IsInitVariable() const;

    bool IsConst() const;
    void SetConst(bool isConst);

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
