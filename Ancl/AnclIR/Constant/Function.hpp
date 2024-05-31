#pragma once

#include <string>
#include <vector>

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Constant/GlobalValue.hpp>
#include <Ancl/AnclIR/Parameter.hpp>
#include <Ancl/AnclIR/Type/FunctionType.hpp>
#include <Ancl/Base.hpp>


namespace ir {

class Function: public GlobalValue {
public:
    Function(FunctionType* type, LinkageType linkage, const std::string& name);

    void SetDeclaration();
    bool IsDeclaration() const;

    void SetInline();
    bool IsInline() const;

    void AddParameter(Parameter* parameter);
    std::vector<Parameter*> GetParameters() const;

    void AddBasicBlock(BasicBlock* basicBlock);

    void SetEntryBlock(BasicBlock* basicBlock);
    BasicBlock* GetEntryBlock() const;

    void SetLastBlock(size_t basicBlockIdx);
    BasicBlock* GetLastBlock() const;

    void SetBasicBlocks(const std::vector<BasicBlock*>& blocks);
    std::vector<BasicBlock*> GetBasicBlocks() const;

    void SetReturnValue(Value* value);
    bool HasReturnValue() const;
    Value* GetReturnValue() const;

    std::string GetNewInstructionName(const std::string& name);
    std::string GetNewBasicBlockName(const std::string& name);

private:
    bool m_IsInline = false;
    bool m_IsDeclaration = false;

    std::vector<BasicBlock*> m_BasicBlocks;

    std::vector<Parameter*> m_Parameters;

    Value* m_ReturnValue = nullptr;

    class ValueNaming;
    TScopePtr<ValueNaming> m_InstructionNamingImpl;
    TScopePtr<ValueNaming> m_BasicBlockNamingImpl;
};

}  // namespace ir
