#pragma once

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

class PrologueEpiloguePass {
public:
    PrologueEpiloguePass(MIRProgram& program, target::TargetMachine* targetMachine);

    void Run();

private:
    void adjustStackCallImpl(MFunction* function, bool isDown);

    void adjustStackCallDown(MFunction* function);

    void adjustStackCallUp(MFunction* function);

    void saveARP(MFunction* function);
    void restoreARP(MFunction* function);

    void insertToPrologue(const MInstruction& instruction);
    void insertToEpilogue(const MInstruction& instruction);

    void reset();

    void generatePrologue(MFunction* function);
    void generateEpilogue(MFunction* function);

private:
    MIRProgram& m_Program;
    target::TargetMachine* m_TargetMachine = nullptr;

    std::vector<MInstruction> m_PrologueInstructions;
    std::vector<MInstruction> m_EpilogueInstructions;
};

}  // namespace gen
