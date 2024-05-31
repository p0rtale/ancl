#include <Ancl/Emitters/AssemblyEmitter.hpp>

#include <fstream>
#include <format>

#include <Ancl/CodeGen/Target/AMD64/AMD64InstructionSet.hpp>
#include <Ancl/Logger/Logger.hpp>


namespace gen::target {

AssemblyEmitter::AssemblyEmitter(const std::string& filename)
    : m_OutputStream(filename) {}

void AssemblyEmitter::Emit(MIRProgram& program, target::TargetMachine* targetMachine) {
    m_TargetMachine = targetMachine;

    EmitHeader();

    for (TScopePtr<MFunction>& function : program.GetFunctions()) {
        EmitFunction(function.get());
    }

    EmitGlobalData(program.GetGlobalData());

    // Only for Linux
    m_OutputStream << "\n.section\t\".note.GNU-stack\",\"\",@progbits\n";
}

void AssemblyEmitter::EmitDataAreas(const std::vector<GlobalDataArea>& dataAreas) {
    for (const GlobalDataArea& area : dataAreas) {
        if (!area.IsLocal()) {
            m_OutputStream << std::format("\t.globl\t{}\n", area.GetName());
        }

        m_OutputStream << area.GetName() << ":\n";

        for (const GlobalDataArea::Slot& slot : area.GetSlots()) {
            if (slot.Type == GlobalDataArea::DataType::kString) {
                m_OutputStream << std::format("\t.asciz\t\"{}\"\n", slot.Init);
                continue;
            }

            std::string directive;
            if (slot.Type == GlobalDataArea::DataType::kZero) {
                directive = "zero";
            } else if (slot.Type == GlobalDataArea::DataType::kByte) {
                directive = "byte";
            } else if (slot.Type == GlobalDataArea::DataType::kShort) {
                directive = "short";
            } else if (slot.Type == GlobalDataArea::DataType::kInt) {
                directive = "long";
            } else if (slot.Type == GlobalDataArea::DataType::kQuad) {
                directive = "quad";
            }

            m_OutputStream << std::format("\t.{}\t{}\n", directive, slot.Init);
        }
    }
    m_OutputStream << "\n";
}

void AssemblyEmitter::EmitGlobalData(const std::vector<GlobalDataArea>& globalData) {
    std::vector<GlobalDataArea> initDataAreas;
    std::vector<GlobalDataArea> zeroDataAreas;
    std::vector<GlobalDataArea> constDataAreas;

    for (const GlobalDataArea& data : globalData) {
        if (data.IsConst()) {
            constDataAreas.push_back(data);
        } else if (data.IsInitialized()) {
            initDataAreas.push_back(data);
        } else {
            zeroDataAreas.push_back(data);
        }
    }

    if (!initDataAreas.empty()) {
        m_OutputStream << "\t.data\n";
        EmitDataAreas(initDataAreas);
    }

    if (!zeroDataAreas.empty()) {
        m_OutputStream << "\t.bss\n";
        EmitDataAreas(zeroDataAreas);
    }

    if (!constDataAreas.empty()) {
        m_OutputStream << "\t.section\t.rodata\n";
        EmitDataAreas(constDataAreas);  
    }
}

void AssemblyEmitter::EmitFunctionInfo(MFunction* function) {
    m_OutputStream << std::format("\t.globl\t{}\n", function->GetName());
}

void AssemblyEmitter::EmitBasicBlock(MBasicBlock* basicBlock, bool isFirst) {
    // TODO: LFB, LFE, LBB, LBE?
    std::string prefix;
    if (!isFirst) {
        prefix = ".";
    }

    m_OutputStream << prefix << basicBlock->GetName() << ":\n";

    for (MInstruction& instruction : basicBlock->GetInstructions()) {
        m_OutputStream << "\t";
        EmitInstruction(instruction);
        m_OutputStream << "\n";
    }
}

void AssemblyEmitter::EmitFunction(MFunction* function) {
    EmitFunctionInfo(function);
    std::vector<TScopePtr<MBasicBlock>>& basicBlocks = function->GetBasicBlocks();
    for (size_t i = 0; i < basicBlocks.size(); ++i) {
        EmitBasicBlock(basicBlocks[i].get(), i == 0);
    }
    m_OutputStream << "\n";
}

}  // namespace gen::target
