#pragma once

#include <string>

#include <Ancl/Emitters/AssemblyEmitter.hpp>


namespace gen::target::amd64 {

class GASEmitter: public AssemblyEmitter {
public:
    GASEmitter(const std::string& filename);

    void EmitHeader() override;

    void EmitOperand(MInstruction::TOperandIt& operandIt, target::TOperandClass opClass,
                     uint64_t instrClass) override;

    void EmitInstruction(MInstruction& instruction) override;

private:
    std::string getInstructionName(target::TargetInstruction targetInstr);

    std::string getInstructionSuffix(MInstruction& instruction, uint64_t targetInstrCode);
};

}  // namespace gen::target::amd64
