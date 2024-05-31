#pragma once

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen::target {

class AssemblyEmitter {
public:
    AssemblyEmitter(const std::string& filename);

    virtual ~AssemblyEmitter() = default;

    void Emit(MIRProgram& program, target::TargetMachine* targetMachine);

    void EmitDataAreas(const std::vector<GlobalDataArea>& dataAreas);
    void EmitGlobalData(const std::vector<GlobalDataArea>& globalData);

    virtual void EmitHeader() = 0;

    void EmitFunctionInfo(MFunction* function);

    virtual void EmitOperand(MInstruction::TOperandIt& operandIt, target::TOperandClass opClass,
                             uint64_t instrClass) = 0;

    virtual void EmitInstruction(MInstruction& instruction) = 0;

    void EmitBasicBlock(MBasicBlock* basicBlock, bool isFirst);
    void EmitFunction(MFunction* function);

protected:
    std::ofstream m_OutputStream;

    target::TargetMachine* m_TargetMachine = nullptr;
};

}  // namespace gen::target
