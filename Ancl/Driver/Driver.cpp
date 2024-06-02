#include <Ancl/Driver/Driver.hpp>

#include "CLexer.h"

#include <Ancl/Logger/Logger.hpp>

#include <Ancl/Preprocessor/Preprocessor.hpp>

#include <Ancl/Visitor/AstDotVisitor.hpp>
#include <Ancl/Visitor/BuildAstVisitor.hpp>
#include <Ancl/Visitor/IRGenAstVisitor.hpp>
#include <Ancl/Visitor/SemanticAstVisitor.hpp>

#include <Ancl/SymbolTable/DotConverter.hpp>

#include <Ancl/Optimization/SSAPass.hpp>
#include <Ancl/Optimization/DVNTPass.hpp>
#include <Ancl/Optimization/DCEPass.hpp>
#include <Ancl/Optimization/CleanPass.hpp>

#include <Ancl/CodeGen/Target/AMD64/AMD64Machine.hpp>

#include <Ancl/CodeGen/MIRGenerator.hpp>
#include <Ancl/CodeGen/Selection/InstructionSelector.hpp>
#include <Ancl/CodeGen/Selection/VirtualRegClassSelector.hpp>

#include <Ancl/CodeGen/RegisterAllocation/DestructiveInstructionPass.hpp>
#include <Ancl/CodeGen/RegisterAllocation/GlobalColoringAllocator.hpp>
#include <Ancl/CodeGen/RegisterAllocation/LinearScanAllocator.hpp>
#include <Ancl/CodeGen/RegisterAllocation/PhiEliminationPass.hpp>

#include <Ancl/CodeGen/Finalization/PrologueEpiloguePass.hpp>
#include <Ancl/CodeGen/Finalization/StackAddressPass.hpp>

#include <Ancl/Emitters/IREmitter.hpp>
#include <Ancl/Emitters/MIREmitter.hpp>
#include <Ancl/Emitters/IntelEmitter.hpp>
#include <Ancl/Emitters/GASEmitter.hpp>



void Driver::Init() {
    ancl::Logger::Init();
}

std::string Driver::Preprocess(const std::string& sourceFilename, bool debug) {
    ANCL_INFO("Preprocessing \"{}\"...", sourceFilename);
    preproc::Preprocessor preprocessor;
    return preprocessor.Run(sourceFilename, debug);
}

void Driver::PreprocessToFile(const std::string& sourceFilename, const std::string& outputFilename) {
    std::ofstream outputStream{outputFilename};
    outputStream << Preprocess(sourceFilename);
    outputStream.close();
    ANCL_INFO("Preprocessed file: \"{}\"", outputFilename);
}

Driver::ParseResult Driver::Parse(std::istream& inputStream) {
    antlr4::ANTLRInputStream inputAntlrStream{inputStream};

    anclgrammar::CLexer lexer{&inputAntlrStream};
    antlr4::CommonTokenStream tokens{&lexer};

    std::vector<antlr4::Token*> lineTokens;
    tokens.fill();
    for (antlr4::Token* token : tokens.getTokens()) {
        if (token->getChannel() == anclgrammar::CLexer::LINE) {
            lineTokens.push_back(token);
        }
    }

    ANCL_INFO("Parsing...");
    anclgrammar::CParser parser{&tokens};
    auto* syntaxTreeEntry = parser.translationUnit();

    if (parser.getNumberOfSyntaxErrors()) {
        ANCL_CRITICAL("Syntax errors were found. The program is completed.");
        return ParseResult::kError;
    }

    ANCL_INFO("Creating AST...");
    buildAST(syntaxTreeEntry, lineTokens);

    return ParseResult::kOK;
}

void Driver::RunSemanticPass() {
    ANCL_INFO("Analyzing semantics...");
    ast::SemanticAstVisitor semanticVisitor{m_ASTProgram};
    semanticVisitor.Run();

    if (!m_SemanticDotInfoPath.empty()) {
        semanticVisitor.PrintScopeInfoDot(m_SemanticDotInfoPath.string());
        ANCL_INFO("Semantic scopes in DOT format: \"{}\"", m_SemanticDotInfoPath.string());
    }

    if (!m_ASTDotInfoPath.empty()) {
        ast::AstDotVisitor dotVisitor{m_ASTDotInfoPath.string()};
        dotVisitor.Visit(*m_ASTProgram.GetTranslationUnit());
        ANCL_INFO("AST in DOT format: \"{}\"", m_ASTDotInfoPath.string());
    }
}

void Driver::GenerateAnclIR() {
    ANCL_INFO("Generating Ancl IR...");
    ast::IRGenAstVisitor irGenVisitor{m_IRProgram};
    irGenVisitor.Run(m_ASTProgram);

    if (!m_IREmitterPath.empty()) {
        const auto irPath = m_IREmitterPath / "AnclIR.txt";
        ir::IREmitter irEmitter(irPath.string());
        irEmitter.Emit(m_IRProgram);
        ANCL_INFO("Ancl IR is saved in \"{}\"", irPath.string());
    }
}

void Driver::OptimizeIR() {
    if (!m_UseOptimizations) {
        return;
    }

    ANCL_INFO("SSA Pass...");
    for (ir::Function* function : m_IRProgram.GetFunctions()) {
        if (function->IsDeclaration()) {
            continue;
        }
        ir::SSAPass ssaPass(function);
        ssaPass.Run();
    }
    emitAnclIR("AnclIR_SSA.txt");

    ANCL_INFO("DVNT Pass...");
    for (ir::Function* function : m_IRProgram.GetFunctions()) {
        if (function->IsDeclaration()) {
            continue;
        }
        ir::DVNTPass dvntPass(function);
        dvntPass.Run();
    }
    emitAnclIR("AnclIR_DVNT.txt");

    ANCL_INFO("DCE Pass...");
    for (ir::Function* function : m_IRProgram.GetFunctions()) {
        if (function->IsDeclaration()) {
            continue;
        }
        ir::DCEPass dcePass(function);
        dcePass.Run();
    }
    emitAnclIR("AnclIR_DCE.txt");

    ANCL_INFO("CleanCFG Pass...");
    for (ir::Function* function : m_IRProgram.GetFunctions()) {
        if (function->IsDeclaration()) {
            continue;
        }
        ir::CleanPass cleanPass(function);
        cleanPass.Run();
    }
    emitAnclIR("AnclIR_CleanCFG.txt");

    ANCL_INFO("Optimization results are saved in \"{}\" directory", m_IREmitterPath.string());
}

void Driver::GenerateMachineIR() {
    ANCL_INFO("Generating Machine IR...");
    m_TargetMachine = CreateScope<gen::target::amd64::AMD64TargetMachine>();
    gen::MIRGenerator machineIRGenerator(m_MIRProgram, m_IRProgram, m_TargetMachine.get());
    machineIRGenerator.Generate();
    emitMachineIR("MachineIR.txt");
    if (!m_MIREmitterPath.empty()) {
        ANCL_INFO("Machine IR is saved in \"{}\" directory", m_MIREmitterPath.string());
    }
}

void Driver::RunInstructionSelection() {
    ANCL_INFO("Selecting target instructions...");
    gen::VirtualRegClassSelector vregClassSelector(m_MIRProgram, m_TargetMachine.get());
    vregClassSelector.Select();

    gen::InstructionSelector instructionSelector(m_MIRProgram, m_TargetMachine.get());
    instructionSelector.Select();

    emitMachineIR("TargetMachineIR.txt");
}

void Driver::AllocateRegisters() {
    if (m_UseGraphColorAllocator) {
        ANCL_INFO("Calculating liveness information...");
        m_FunctionsLiveOUT.clear();
        for (auto& function : m_MIRProgram.GetFunctions()) {
            // TODO: Simplify
            std::string functionName = function->GetName();
            m_FunctionsLiveOUT.emplace(functionName, *function);
            m_FunctionsLiveOUT.at(functionName).Run();
        }
    }

    ANCL_INFO("Eliminating Phi functions...");
    gen::PhiEliminationPass phiEliminationPass(m_MIRProgram, m_TargetMachine.get());
    phiEliminationPass.Run();
    emitMachineIR("TargetMachineIR_PhiElim.txt");

    ANCL_INFO("Translation of instructions into two-address form...");
    gen::DestructiveInstructionPass destructivePass(m_MIRProgram, m_TargetMachine.get());
    destructivePass.Run();
    emitMachineIR("TargetMachineIR_TwoAddress.txt");

    if (m_UseGraphColorAllocator) {
        ANCL_INFO("Allocating registers using Graph Coloring Algorithm...");
        for (auto& function : m_MIRProgram.GetFunctions()) {
            std::string functionName = function->GetName();

            gen::GlobalColoringAllocator floatAllocator(*function, m_TargetMachine.get(), /*isFloatClass=*/true);
            floatAllocator.Allocate(m_FunctionsLiveOUT.at(functionName));

            gen::GlobalColoringAllocator generalAllocator(*function, m_TargetMachine.get(), /*isFloatClass=*/false);
            generalAllocator.Allocate(m_FunctionsLiveOUT.at(functionName));
        }
    } else {
        ANCL_INFO("Allocating registers using Linear Scan Algorithm...");
        for (auto& function : m_MIRProgram.GetFunctions()) {
            gen::LinearScanAllocator floatAllocator(*function, m_TargetMachine.get(), /*isFloatClass=*/true);
            floatAllocator.Allocate();

            gen::LinearScanAllocator generalAllocator(*function, m_TargetMachine.get(), /*isFloatClass=*/false);
            generalAllocator.Allocate();
        }
    }

    emitMachineIR("TargetMachineIR_RegAlloc.txt");
}

void Driver::Finalize() {
    ANCL_INFO("Inserting prologue and epilogue...");
    gen::PrologueEpiloguePass prologEpilogPass(m_MIRProgram, m_TargetMachine.get());
    prologEpilogPass.Run();
    emitMachineIR("TargetMachineIR_PrologEpilog.txt");

    ANCL_INFO("Expanding stack addresses...");
    gen::StackAddressPass stackAddressPass(m_MIRProgram, m_TargetMachine.get());
    stackAddressPass.Run();
    emitMachineIR("TargetMachineIR_StackAddress.txt");
}

void Driver::EmitAssembler() {
    if (!m_IntelEmitterPath.empty()) {
        gen::target::amd64::IntelEmitter intelEmitter(m_IntelEmitterPath);
        intelEmitter.Emit(m_MIRProgram, m_TargetMachine.get());
        ANCL_INFO("Intel assembler is saved in \"{}\"", m_IntelEmitterPath.string());
    }
    if (!m_GASEmitterPath.empty()) {
        gen::target::amd64::GASEmitter gasEmitter(m_GASEmitterPath);
        gasEmitter.Emit(m_MIRProgram, m_TargetMachine.get());
        ANCL_INFO("GAS assembler is saved in \"{}\"", m_GASEmitterPath.string());
    }
}

void Driver::SetUseGraphColorAllocatorFlag(bool flag) {
    m_UseGraphColorAllocator = flag;
}

void Driver::SetASTDotInfoPath(const std::string& path) {
    m_ASTDotInfoPath = path;
}

void Driver::SetSemanticDotInfoPath(const std::string& path) {
    m_SemanticDotInfoPath = path;
}

void Driver::SetIREmitterPath(const std::string& path) {
    m_IREmitterPath = path;
}

void Driver::SetUseOptimizations(bool useOptimizations) {
    m_UseOptimizations = useOptimizations;
}

void Driver::SetMIREmitterPath(const std::string& path) {
    m_MIREmitterPath = path;
}

void Driver::SetIntelEmitterPath(const std::string& path) {
    m_IntelEmitterPath = path;
}

void Driver::SetGASEmitterPath(const std::string& path) {
    m_GASEmitterPath = path;
}

void Driver::buildAST(anclgrammar::CParser::TranslationUnitContext* syntaxTreeEntry,
                const std::vector<antlr4::Token*> lineTokens) {
    anclgrammar::BuildAstVisitor buildVisitor{m_ASTProgram};
    buildVisitor.setLineTokens(lineTokens);
    buildVisitor.visitTranslationUnit(syntaxTreeEntry);
}

void Driver::emitAnclIR(const std::string& filename) {
    if (!m_IREmitterPath.empty()) {
        const auto irPath = m_IREmitterPath / filename;
        ir::IREmitter irEmitter(irPath.string());
        irEmitter.Emit(m_IRProgram);
    }   
}

void Driver::emitMachineIR(const std::string& filename) {
    if (!m_MIREmitterPath.empty()) {
        const auto irPath = m_MIREmitterPath / filename;
        gen::MIREmitter mirEmitter(irPath.string());
        mirEmitter.Emit(m_MIRProgram, m_TargetMachine.get());
    }
}
