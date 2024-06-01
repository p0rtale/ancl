#include <string>
#include <sstream>

#include <Ancl/Driver/CLI11.hpp>
#include <Ancl/Driver/Driver.hpp>


int main(int argc, char** argv) {
    CLI::App app{"Ancl"};
    argv = app.ensure_utf8(argv);

    std::string sourceFile;
    app.add_option("-f,--file", sourceFile, "Input C file")->required();

    std::string preprocFilename;
    app.add_option("-p,--preproc", preprocFilename, "Preprocessor output filename");

    std::string astDotFilename;
    app.add_option("-a,--ast", astDotFilename, "Filename to output AST in DOT format");

    std::string scopeDotFilename;
    app.add_option("-s,--scope", scopeDotFilename, "Filename to output semantic scopes in DOT format");

    std::string anclIRPath;
    app.add_option("-r,--ir", anclIRPath, "Ancl IR output directory")->check(CLI::ExistingDirectory);

    std::string machineIRPath;
    app.add_option("-m,--mir", machineIRPath, "Machine IR output directory")->check(CLI::ExistingDirectory);

    auto* asmGroup = app.add_option_group("asm");

    std::string intelPath;
    asmGroup->add_option("-n,--intel-asm", intelPath, "Intel assembler output filename");

    std::string gasPath;
    asmGroup->add_option("-g,--gas-asm", gasPath, "GAS assembler output filename");

    asmGroup->require_option(1);

    bool useOptimizations = false;
    app.add_flag("-O,--optim", useOptimizations, "Use optimizations");

    bool isLinearScan = false;
    app.add_flag("--linscan", isLinearScan, "Use Linear Scan Allocator");

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

    Driver::ParseResult result = Driver::ParseResult::kOK;
    if (preprocFilename.empty()) {
        std::string preprocessed = anclDriver.Preprocess(sourceFile);
        std::istringstream stream{preprocessed};
        result = anclDriver.Parse(stream);
    } else {
        anclDriver.PreprocessToFile(sourceFile, preprocFilename);
        std::ifstream stream{preprocFilename};
        result = anclDriver.Parse(stream);
    }
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
