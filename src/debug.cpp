#include <sstream>

#include <Ancl/Driver/Driver.hpp>


int main(int argc, char** argv) {
    Driver anclDriver;
    anclDriver.Init();

    anclDriver.SetASTDotInfoPath("ast.txt");
    anclDriver.SetSemanticDotInfoPath("scope.txt");

    anclDriver.SetIREmitterPath("AnclIR");
    anclDriver.SetMIREmitterPath("MachineIR");

    anclDriver.SetUseOptimizations(true);
    anclDriver.SetUseGraphColorAllocatorFlag(true);

    anclDriver.SetIntelEmitterPath("intel.s");
    anclDriver.SetGASEmitterPath("gas.s");

    anclDriver.PreprocessToFile("main.c", "preproc.c");
    std::ifstream stream{"preproc.c"};
    Driver::ParseResult result = anclDriver.Parse(stream);
    if (result != Driver::ParseResult::kOK) {
        return EXIT_FAILURE;
    }

    anclDriver.RunSemanticPass();
    anclDriver.GenerateAnclIR();

    anclDriver.OptimizeIR();

    anclDriver.GenerateMachineIR();
    anclDriver.RunInstructionSelection();
    anclDriver.AllocateRegisters();

    anclDriver.Finalize();
    anclDriver.EmitAssembler();

    return EXIT_SUCCESS;
}
