#pragma once

#include <unordered_set>
#include <fstream>
#include <string>

#include <Ancl/AnclIR/IRProgram.hpp>


namespace ir {

class IREmitter {
public:
    IREmitter(const std::string& filename);

    void Emit(const IRProgram& program);

private:
    std::string getValueString(const Value* value);

    void emitInstruction(const Instruction* instruction);

    void emitBasicBlock(const BasicBlock* basicBlock);

    std::string getFunctionSignatureString(const Function* function);

    std::string getTypeString(const Type* type);

private:
    std::ofstream m_OutputStream;

    std::unordered_set<std::string> m_StructNames;
};

}  // namespace ir
