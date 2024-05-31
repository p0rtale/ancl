#include <Ancl/Driver/CLI11.hpp>
#include <Ancl/Driver/Driver.hpp>


int main(int argc, char** argv) {
    CLI::App app{"Ancl"};
    argv = app.ensure_utf8(argv);

    std::string sourceFile = "main.c";
    app.add_option("-f,--file", sourceFile, "Input C file");

    std::string preprocFilename = "preproc.c";
    app.add_option("-p,--preproc", preprocFilename, "Preprocessor output filename");

    std::string astDotFilename = "ast.dot";
    app.add_option("-a,--ast", astDotFilename, "Filename to output AST in DOT format");

    std::string scopeDotFilename = "scope.dot";
    app.add_option("-s,--scope", scopeDotFilename, "Filename to output semantic scopes in DOT format");

    std::string anclIRPath = ".";
    app.add_option("-r,--ir", anclIRPath, "Ancl IR output directory")->check(CLI::ExistingDirectory);

    std::string machineIRPath = ".";
    app.add_option("-m,--mir", machineIRPath, "Machine IR output directory")->check(CLI::ExistingDirectory);

    std::string intelPath = "intel.s";
    app.add_option("-n,--intel-asm", intelPath, "Intel assembler output filename");

    std::string gasPath = "gas.s";
    app.add_option("-g,--gas-asm", gasPath, "GAS assembler output filename");

    bool useOptimizations = false;
    app.add_option("-O,--optim", useOptimizations, "Use optimizations");

    bool isLinearScan = false;
    app.add_option("--linscan", isLinearScan, "Use Linear Scan Allocator");

    CLI11_PARSE(app, argc, argv);

    Driver anclDriver;
    anclDriver.Init();

    anclDriver.SetASTDotInfoPath(astDotFilename);
    anclDriver.SetSemanticDotInfoPath(scopeDotFilename);

    anclDriver.SetIREmitterPath(anclIRPath);
    anclDriver.SetMIREmitterPath(machineIRPath);

    anclDriver.SetUseOptimizations(useOptimizations);
    anclDriver.SetUseGraphColorAllocatorFlag(!isLinearScan);

    anclDriver.SetIntelEmitterPath(intelPath);
    anclDriver.SetGASEmitterPath(gasPath);

    anclDriver.PreprocessToFile(sourceFile, preprocFilename);

    Driver::ParseResult result = anclDriver.Parse(preprocFilename);
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
