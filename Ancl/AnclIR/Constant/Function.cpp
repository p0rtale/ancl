#include <Ancl/AnclIR/Constant/Function.hpp>

#include <unordered_map>


namespace ir {

Function::Function(FunctionType* type, LinkageType linkage, const std::string& name)
        : GlobalValue(type, linkage),
          m_InstructionNamingImpl(CreateScope<ValueNaming>()),
          m_BasicBlockNamingImpl(CreateScope<ValueNaming>()) {
    SetName(name);
}

void Function::SetDeclaration() {
    m_IsDeclaration = true;
}

bool Function::IsDeclaration() const {
    return m_IsDeclaration;
}

void Function::SetInline() {
    m_IsInline = true;
}

bool Function::IsInline() const {
    return m_IsInline;
}

void Function::AddParameter(Parameter* parameter) {
    m_Parameters.push_back(parameter);
}

std::vector<Parameter*> Function::GetParameters() const {
    return m_Parameters;
}

void Function::AddBasicBlock(BasicBlock* basicBlock) {
    m_BasicBlocks.push_back(basicBlock);
}

void Function::SetEntryBlock(BasicBlock* basicBlock) {
    basicBlock->SetName(GetName());
    m_BasicBlocks.at(0) = basicBlock;
}

BasicBlock* Function::GetEntryBlock() const {
    if (m_BasicBlocks.empty()) {
        return nullptr;
    }
    return m_BasicBlocks.front();
}

void Function::SetLastBlock(size_t basicBlockIdx) {
    std::swap(m_BasicBlocks[basicBlockIdx], m_BasicBlocks.back());
}

BasicBlock* Function::GetLastBlock() const {
    if (m_BasicBlocks.empty()) {
        return nullptr;
    }
    return m_BasicBlocks.back();
}

void Function::SetBasicBlocks(const std::vector<BasicBlock*>& blocks) {
    m_BasicBlocks = blocks;
}

std::vector<BasicBlock*> Function::GetBasicBlocks() const {
    return m_BasicBlocks;
}

void Function::SetReturnValue(Value* value) {
    m_ReturnValue = value;
}

bool Function::HasReturnValue() const {
    return m_ReturnValue;
}

Value* Function::GetReturnValue() const {
    return m_ReturnValue;
}

class Function::ValueNaming {
public:
    ValueNaming() = default;

    std::string GetNewName(const std::string& name) {
        if (!m_ValueNumbers.contains(name)) {
            m_ValueNumbers[name] = 1;
            if (!name.empty()) {
                return name;
            }
            return "0";
        }

        std::string newName = name;
        while (m_ValueNumbers.contains(newName)) {
            newName = name + std::to_string(m_ValueNumbers[name]++);
        }

        m_ValueNumbers[newName] = 1;

        return newName;
    }

private:
    std::unordered_map<std::string, unsigned int> m_ValueNumbers;
};

std::string Function::GetNewInstructionName(const std::string& name) {
    return m_InstructionNamingImpl->GetNewName(name);
}

std::string Function::GetNewBasicBlockName(const std::string& name) {
    return m_BasicBlockNamingImpl->GetNewName(name);
}

}  // namespace ir
