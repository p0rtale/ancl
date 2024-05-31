#pragma once

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

// It does not cover the Swap Problem yet
// TODO: Serialize copy groups
class PhiEliminationPass {
public:
    PhiEliminationPass(MIRProgram& program, target::TargetMachine* targetMachine);

    void Run();

private:
    MIRProgram& m_Program;
    target::TargetMachine* m_TargetMachine = nullptr;
};

}  // namespace gen
