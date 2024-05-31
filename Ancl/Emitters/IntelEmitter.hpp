#pragma once

#include <string>

#include <Ancl/Emitters/AssemblyEmitter.hpp>


namespace gen::target::amd64 {

class IntelEmitter: public AssemblyEmitter {
public:
    IntelEmitter(const std::string& filename);

    void EmitHeader() override;

    void EmitOperand(MInstruction::TOperandIt& operandIt, target::TOperandClass opClass,
                     uint64_t instrClass) override;

    void EmitInstruction(MInstruction& instruction) override;
};

}  // namespace gen::target::amd64
