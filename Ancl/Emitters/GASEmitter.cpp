#include <Ancl/Emitters/GASEmitter.hpp>

#include <format>

#include <Ancl/CodeGen/Target/AMD64/AMD64InstructionSet.hpp>
#include <Ancl/Logger/Logger.hpp>


namespace gen::target::amd64 {

GASEmitter::GASEmitter(const std::string& filename)
    : AssemblyEmitter(filename) {}

void GASEmitter::EmitHeader() {
    m_OutputStream << "\t.text\n";
    m_OutputStream << "\n";
}

void GASEmitter::EmitOperand(MInstruction::TOperandIt& operandIt,
                             target::TOperandClass opClass, uint64_t) {
    if (opClass.size() > 1) {
        assert(opClass.size() == 4);

        RegisterSet* regSet = m_TargetMachine->GetRegisterSet();

        // Reverse order
        MOperand dispImm = *operandIt--;
        MOperand indexReg = *operandIt--;
        MOperand scaleImm = *operandIt--;
        MOperand baseReg = *operandIt--;

        std::string addressExpr;

        std::string labelDisp;
        if (baseReg.IsFunction() || baseReg.IsGlobalSymbol()) {
            if (baseReg.IsFunction()) {
                labelDisp = baseReg.GetFunctionSymbol();
            } else {
                labelDisp = baseReg.GetGlobalSymbol();
            }

            Register instrPointerReg = regSet->GetIP();
            addressExpr = std::format("%{}", instrPointerReg.GetName());
        } else {
            uint64_t baseRegNumber = baseReg.GetRegister();
            Register basePReg = regSet->GetRegister(baseRegNumber);
            addressExpr = std::format("%{}", basePReg.GetName());
        }

        uint64_t indexRegNumber = indexReg.GetRegister();
        if (regSet->IsValidRegister(indexRegNumber)) {
            Register indexReg = regSet->GetRegister(indexRegNumber);
            addressExpr += std::format(",%{},{}", indexReg.GetName(), scaleImm.GetImmInteger());
        }

        addressExpr = "(" + addressExpr + ")";

        int64_t dispImmInteger = dispImm.GetImmInteger();
        if (dispImmInteger > 0) {
            addressExpr = std::format("{}{}", dispImmInteger, addressExpr);
        } else if (dispImmInteger < 0) {
            addressExpr = std::format("-{}{}", -dispImmInteger, addressExpr);
        }

        if (labelDisp.empty()) {
            m_OutputStream << addressExpr;
        } else {
            m_OutputStream << labelDisp + addressExpr;
        }

        return;
    }
    
    if (opClass.at(0) == IMM) {
        if (operandIt->IsImmInteger()) {
            m_OutputStream << std::format("${}", operandIt->GetImmInteger());
        } else {
            ANCL_CRITICAL("IntelEmitter: Immediate float is forbidden (must be in the constant pool)");
        }

        --operandIt;
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

        --operandIt;
        return;
    }

    if (opClass.at(0) == GR || opClass.at(0) == FR) {
        RegisterSet* regSet = m_TargetMachine->GetRegisterSet();
        uint64_t regNumber = operandIt->GetRegister();
        Register reg = regSet->GetRegister(regNumber);

        m_OutputStream << std::format("%{}", reg.GetName());

        --operandIt;
        return;
    }
}

void GASEmitter::EmitInstruction(MInstruction& instruction) {
    target::InstructionSet* targetInstrSet = m_TargetMachine->GetInstructionSet();
    uint64_t targetInstrCode = instruction.GetTargetInstructionCode();
    target::TargetInstruction targetInstr = targetInstrSet->GetInstruction(targetInstrCode);

    std::string sizeSuffix = getInstructionSuffix(instruction, targetInstrCode);
    m_OutputStream << getInstructionName(targetInstr) + sizeSuffix << "\t";

    // TODO: string stream
    std::string operandsString;
    MInstruction::TOperandIt operandIt = --instruction.GetOpEnd();
    for (int64_t i = targetInstr.GetOperandsNumber() - 1; i >= 0; --i) {
        target::TOperandClass opClass = targetInstr.GetOperandClass(i);
        EmitOperand(operandIt, opClass, 0);

        if (i > 0) {
            m_OutputStream << ", ";
        }
    }
}

std::string GASEmitter::getInstructionName(target::TargetInstruction targetInstr) {
    // https://sourceware.org/binutils/docs-2.19/as/i386_002dMnemonics.html
    switch (targetInstr.GetInstructionCode()) {
        case AMD64InstructionSet::CDQ:
            return "cltd";
        case AMD64InstructionSet::CQO:
            return "cqto";
        default:
            return targetInstr.GetAsmName();
    }
}

std::string GASEmitter::getInstructionSuffix(MInstruction& instruction, uint64_t targetInstrCode) {
    switch (targetInstrCode) {
        case AMD64InstructionSet::CVTSI2SS_RR:
        case AMD64InstructionSet::CVTSI2SS_RM:
        case AMD64InstructionSet::CVTSI2SD_RR:
        case AMD64InstructionSet::CVTSI2SD_RM:
            return "";

        default:
            break;
    }

    unsigned int instrClass = instruction.GetInstructionClass();

    std::string sizeSuffix;
    if (instrClass == GR8) {
        sizeSuffix = "b";
    } else if (instrClass == GR16) {
        sizeSuffix = "w";
    } else if (instrClass == GR32) {
        sizeSuffix = "l";
    } else if (instrClass == GR64) {
        sizeSuffix = "q";
    } else {
        return "";
    }

    switch (targetInstrCode) {
        case AMD64InstructionSet::MOVZX_RR:
        case AMD64InstructionSet::MOVZX_RM:
        case AMD64InstructionSet::MOVSX_RR:
        case AMD64InstructionSet::MOVSX_RM:
        case AMD64InstructionSet::MOVSXD_RR:
        case AMD64InstructionSet::MOVSXD_RM: {
            MInstruction::TOperandIt defOperand = instruction.GetDefinition();
            unsigned defClass = defOperand->GetRegisterClass();
            if (defClass == GR8) {
                sizeSuffix += "b";
            } else if (defClass == GR16) {
                sizeSuffix += "w";
            } else if (defClass == GR32) {
                sizeSuffix += "l";
            } else if (defClass == GR64) {
                sizeSuffix += "q";
            }
            break;
        }
        default:
            break;
    }

    return sizeSuffix;
}

}  // namespace gen::target::amd64 
