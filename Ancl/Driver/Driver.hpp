#pragma once

#include <filesystem>
#include <unordered_map>

#include "antlr4-runtime.h"
#include "CParser.h"

#include <Ancl/Base.hpp>

#include <Ancl/CodeGen/RegisterAllocation/LiveOutPass.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>

#include <Ancl/AnclIR/IRProgram.hpp>
#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/Grammar/AST/ASTProgram.hpp>


class Driver {
public:
    enum class ParseResult {
        kOK = 0,
        kError,
    };

public:
    Driver() = default;

    void Init();

    std::string Preprocess(const std::string& sourceFilename, bool debug = false);
    void PreprocessToFile(const std::string& sourceFilename, const std::string& outputFilename);

    ParseResult Parse(const std::string& sourceFilename);

    void RunSemanticPass();

    void GenerateAnclIR();
    void OptimizeIR();

    void GenerateMachineIR();
    void RunInstructionSelection();

    void AllocateRegisters();

    void Finalize();

    void EmitAssembler();

    void SetUseGraphColorAllocatorFlag(bool flag);
    void SetASTDotInfoPath(const std::string& path);
    void SetSemanticDotInfoPath(const std::string& path);
    void SetIREmitterPath(const std::string& path);
    void SetUseOptimizations(bool useOptimizations);
    void SetMIREmitterPath(const std::string& path);
    void SetIntelEmitterPath(const std::string& path);
    void SetGASEmitterPath(const std::string& path);

private:
    void buildAST(anclgrammar::CParser::TranslationUnitContext* syntaxTreeEntry,
                  const std::vector<antlr4::Token*> lineTokens);

    void emitAnclIR(const std::string& filename);
    void emitMachineIR(const std::string& filename);

private:
    ast::ASTProgram m_ASTProgram;
    ir::IRProgram m_IRProgram;
    gen::MIRProgram m_MIRProgram;

    std::unordered_map<std::string, gen::LiveOUTPass> m_FunctionsLiveOUT;
    bool m_UseGraphColorAllocator = true;

    bool m_UseOptimizations = false;

    TScopePtr<gen::target::TargetMachine> m_TargetMachine;

    std::filesystem::path m_SemanticDotInfoPath;
    std::filesystem::path m_ASTDotInfoPath;

    std::filesystem::path m_IREmitterPath;
    std::filesystem::path m_MIREmitterPath;

    std::filesystem::path m_IntelEmitterPath;
    std::filesystem::path m_GASEmitterPath;
};
