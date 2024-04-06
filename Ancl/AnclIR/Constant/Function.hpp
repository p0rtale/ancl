#pragma once

#include <vector>
#include <string>

#include <Ancl/AnclIR/Constant/GlobalValue.hpp>
#include <Ancl/AnclIR/Type/FunctionType.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class Function: public GlobalValue {
public:
    Function(FunctionType* type, LinkageType linkage, const std::string& name)
            : GlobalValue(type, linkage) {
        SetName(name);
    }

    void SetInline() {
        m_IsInline = true;
    }

    bool IsInline() const {
        return m_IsInline;
    }

    void AddBasicBlock(BasicBlock* basicBlock) {
        m_BasicBlocks.push_back(basicBlock);
    }

    std::vector<BasicBlock*> GetBasicBlocks() const {
        return m_BasicBlocks;
    }

private:
    bool m_IsInline = false;

    std::vector<BasicBlock*> m_BasicBlocks;
};

}  // namespace ir
