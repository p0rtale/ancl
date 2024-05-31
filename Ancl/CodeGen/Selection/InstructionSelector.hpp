#pragma once

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>
#include <Ancl/CodeGen/Selection/SelectionGraph.hpp>


namespace gen {

class InstructionSelector {
public:
    InstructionSelector(MIRProgram& mirProgram,
                        target::TargetMachine* targetMachine);

    void Select();

    static MInstruction SelectInstruction(MInstruction& instruction,
                                          target::TargetMachine* targetMachine);

private:
    MIRProgram& m_MIRProgram;

    target::TargetMachine* m_TargetMachine = nullptr;
};

}  // namespace gen

