#pragma once

#include <vector>
#include <string>

#include <Ancl/AnclIR/Constant/GlobalValue.hpp>
#include <Ancl/AnclIR/Type/FunctionType.hpp>
#include <Ancl/AnclIR/Parameter.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>


namespace ir {

class Function: public GlobalValue {
public:
    Function(FunctionType* type, LinkageType linkage, const std::string& name)
            : GlobalValue(type, linkage) {
        SetName(name);
    }

    void SetDeclaration() {
        m_IsDeclaration = true;
    }

    bool IsDeclaration() const {
        return m_IsDeclaration;
    }

    void SetInline() {
        m_IsInline = true;
    }

    bool IsInline() const {
        return m_IsInline;
    }

    void AddParameter(Parameter* parameter) {
        m_Parameters.push_back(parameter);
    }

    std::vector<Parameter*> GetParameters() const {
        return m_Parameters;
    }

    void AddBasicBlock(BasicBlock* basicBlock) {
        m_BasicBlocks.push_back(basicBlock);
    }

    BasicBlock* GetEntryBlock() const {
        if (m_BasicBlocks.empty()) {
            return nullptr;
        }
        return m_BasicBlocks[0];
    }

    std::vector<BasicBlock*> GetBasicBlocks() const {
        return m_BasicBlocks;
    }

    void SetReturnValue(Value* value) {
        m_ReturnValue = value;
    }

    bool HasReturnValue() const {
        return m_ReturnValue;
    }

    Value* GetReturnValue() const {
        return m_ReturnValue;
    }

private:
    bool m_IsInline = false;
    bool m_IsDeclaration = false;

    std::vector<BasicBlock*> m_BasicBlocks;

    std::vector<Parameter*> m_Parameters;
    Value* m_ReturnValue = nullptr;
};

}  // namespace ir
