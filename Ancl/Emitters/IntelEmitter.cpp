#include <Ancl/Emitters/IntelEmitter.hpp>

#include <format>

#include <Ancl/CodeGen/Target/AMD64/AMD64InstructionSet.hpp>
#include <Ancl/Logger/Logger.hpp>


namespace gen::target::amd64 {

IntelEmitter::IntelEmitter(const std::string& filename)
    : AssemblyEmitter(filename) {}

void IntelEmitter::EmitHeader() {
    m_OutputStream << "\t.text\n";
    m_OutputStream << "\t.intel_syntax noprefix\n";
    m_OutputStream << "\n";
}

void IntelEmitter::EmitOperand(MInstruction::TOperandIt& operandIt,
                               target::TOperandClass opClass, uint64_t instrClass) {
    if (opClass.size() > 1) {
        assert(opClass.size() == 4);

        RegisterSet* regSet = m_TargetMachine->GetRegisterSet();

        MOperand baseReg = *operandIt++;
        MOperand scaleImm = *operandIt++;
        MOperand indexReg = *operandIt++;
        MOperand dispImm = *operandIt++;

        std::string addressExpr;

        if (baseReg.IsFunction() || baseReg.IsGlobalSymbol()) {
            std::string labelName;
            if (baseReg.IsFunction()) {
                labelName = baseReg.GetFunctionSymbol();
            } else {
                labelName = baseReg.GetGlobalSymbol();
            }

            Register instrPointerReg = regSet->GetIP();
            addressExpr = std::format("{} + {}", instrPointerReg.GetName(), labelName);
        } else {
            uint64_t baseRegNumber = baseReg.GetRegister();
            Register basePReg = regSet->GetRegister(baseRegNumber);
            // Register basePReg = regSet->GetARP();
            addressExpr = basePReg.GetName();
        }

        uint64_t indexRegNumber = indexReg.GetRegister();
        if (regSet->IsValidRegister(indexRegNumber)) {
            Register indexReg = regSet->GetRegister(indexRegNumber);
            int64_t scale = scaleImm.GetImmInteger();
            if (scale != 1) {
                addressExpr += std::format(" + {}*{}", scale, indexReg.GetName());
            } else {
                addressExpr += std::format(" + {}", indexReg.GetName());
            }
        }

        int64_t dispImmInteger = dispImm.GetImmInteger();
        if (dispImmInteger > 0) {
            addressExpr += " + " + std::to_string(dispImmInteger);
        } else if (dispImmInteger < 0) {
            addressExpr += " - " + std::to_string(-dispImmInteger);
        }

        std::string sizePrefix;
        if (instrClass == GR8) {
            sizePrefix = "byte";
        } else if (instrClass == GR16) {
            sizePrefix = "word";
        } else if (instrClass == GR32 || instrClass == FR32) {
            sizePrefix = "dword";
        } else if (instrClass == GR64 || instrClass == FR64) {
            sizePrefix = "qword";
        }

        m_OutputStream << std::format("{} ptr [{}]", sizePrefix, addressExpr);

        return;
    }
    
    if (opClass.at(0) == IMM) {
        if (operandIt->IsImmInteger()) {
            m_OutputStream << std::to_string(operandIt->GetImmInteger());
        } else {
            ANCL_CRITICAL("IntelEmitter: Immediate float is forbidden (must be in the constant pool)");
        }

        ++operandIt;
        return;
    }

    if (opClass.at(0) == REL) {
        if (operandIt->IsFunction()) {
            m_OutputStream << operandIt->GetFunctionSymbol();
        } else if (operandIt->IsMBasicBlock()) {
            MBasicBlock* block = operandIt->GetBasicBlock();
            m_OutputStream << "." << block->GetName();
        } else {
            ANCL_CRITICAL("IntelEmitter: Invalid relative operand");
        }

        ++operandIt;
        return;
    }

    if (opClass.at(0) == GR || opClass.at(0) == FR) {
        RegisterSet* regSet = m_TargetMachine->GetRegisterSet();
        uint64_t regNumber = operandIt->GetRegister();
        Register reg = regSet->GetRegister(regNumber);
        // Register reg = regSet->GetRegister(1);

        m_OutputStream << reg.GetName();

        ++operandIt;
        return;
    }
}

void IntelEmitter::EmitInstruction(MInstruction& instruction) {
    target::InstructionSet* targetInstrSet = m_TargetMachine->GetInstructionSet();

    uint64_t targetInstrCode = instruction.GetTargetInstructionCode();
    target::TargetInstruction targetInstr = targetInstrSet->GetInstruction(targetInstrCode);

    std::string suffix;
    switch (targetInstr.GetInstructionCode()) {
        case AMD64InstructionSet::MOVZX_RR:
            suffix = "x";
            break;
        case AMD64InstructionSet::MOVZX_RM:
            suffix = "x";
            break;
        case AMD64InstructionSet::MOVSX_RR:
            suffix = "x";
            break;
        case AMD64InstructionSet::MOVSX_RM:
            suffix = "x";
            break;
        case AMD64InstructionSet::MOVSXD_RR:
            suffix = "xd";
            break;
        case AMD64InstructionSet::MOVSXD_RM:
            suffix = "xd";
            break;
        default:
            break;
    }

    m_OutputStream << targetInstr.GetAsmName() + suffix << "\t";

    // TODO: string stream
    std::string operandsString;
    MInstruction::TOperandIt operandIt = instruction.GetOpBegin();
    for (size_t i = 0; i < targetInstr.GetOperandsNumber(); ++i) {
        if (i == 0 && targetInstr.IsDestructive()) {
            ++operandIt;
        }

        target::TOperandClass opClass = targetInstr.GetOperandClass(i);
        EmitOperand(operandIt, opClass, instruction.GetInstructionClass());

        if (i + 1 < targetInstr.GetOperandsNumber()) {
            m_OutputStream << ", ";
        }
    }
}

}  // namespace gen::target::amd64 
