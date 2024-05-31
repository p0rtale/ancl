#pragma once

#include <fstream>
#include <string>

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

class MIREmitter {
public:
    MIREmitter(const std::string& filename);

    void Emit(MIRProgram& program, target::TargetMachine* targetMachine);

private:
    void emitStackFrame(LocalDataArea& localData);

    std::string getOperandString(MOperand& operand, target::TargetMachine* targetMachine);

    void emitInstruction(MInstruction& instruction, target::TargetMachine* targetMachine);
    void emitBasicBlock(MBasicBlock* basicBlock, target::TargetMachine* targetMachine);

private:
    std::ofstream m_OutputStream;
};

}  // namespace gen
