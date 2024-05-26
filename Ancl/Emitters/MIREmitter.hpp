#pragma once

#include <algorithm>
#include <string>
#include <fstream>
#include <format>

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>


namespace gen {

class MIREmitter {
public:
    MIREmitter(const std::string& filename): m_OutputStream(filename) {}

    void Emit(MIRProgram& program, target::TargetMachine* targetMachine) {
        for (TScopePtr<MFunction>& function : program.GetFunctions()) {
            m_OutputStream << std::format("func {} {{\n", function->GetName());

            emitStackFrame(function->GetLocalDataArea());

            std::vector<TScopePtr<MBasicBlock>>& basicBlocks = function->GetBasicBlocks();
            for (size_t i = 0; i < basicBlocks.size(); ++i) {
                emitBasicBlock(basicBlocks[i].get(), targetMachine);

                if (i + 1 < basicBlocks.size()) {
                    m_OutputStream << "\n";
                }
            }
            m_OutputStream << "}\n\n";
        }
    }

private:
    void emitStackFrame(LocalDataArea& localData) {
        m_OutputStream << "Frame:\n";
        for (const LocalDataArea::Slot& slot : localData.GetSlots()) {
            uint offset = localData.GetSlotOffset(slot.VReg);
            uint prettyVRegNumber = slot.VReg - MFunction::kFirstVirtualRegisterNumber;
            m_OutputStream << std::format("\t%{}: size={}, align={}, offset={}\n",
                                          prettyVRegNumber, slot.Size, slot.Align, offset);
        }
        m_OutputStream << "\n";
    }

    std::string getOperandString(MOperand& operand, target::TargetMachine* targetMachine) {
        if (operand.IsImmInteger()) {
            return std::to_string(operand.GetImmInteger());
        }
        if (operand.IsImmFloat()) {
            return std::to_string(operand.GetImmFloat());
        }
        if (operand.IsRegister() || operand.IsMemory()) {
            std::string regName = "null";

            uint prettyVRegNumber = operand.GetRegister();
            if (prettyVRegNumber != 0) {
                prettyVRegNumber -= MFunction::kFirstVirtualRegisterNumber;
                regName = std::to_string(prettyVRegNumber);
            }

            char prefix = '%';
            if (operand.IsPRegister()) {
                prefix = '$';

                target::RegisterSet* registers = targetMachine->GetRegisterSet();
                uint regNumber = operand.GetRegister();
                target::Register preg = registers->GetRegister(regNumber);
                regName = preg.GetName();
            }

            // TODO: Print register class (platform dependent)
            MType type = operand.GetType();
            uint bits = type.GetBytes() * 8;
            std::string typeString = std::format("i{}", bits);
            if (type.IsFloat()) {
                typeString = std::format("fp{}", bits);
            }

            std::string opString = std::format("{}{}:{}", prefix, regName, typeString);
            if (operand.IsMemory()) {
                return "[" + opString + "]";
            }
            return opString;
        }
        if (operand.IsGlobalSymbol() || operand.IsFunction()) {
            return std::format("@{}", operand.GetGlobalSymbol());
        }
        if (operand.IsMBasicBlock()) {
            MBasicBlock* block = operand.GetBasicBlock();
            return std::format("%{}", block->GetName());
        }
        if (operand.IsStackIndex()) {
            uint prettyVRegNumber = operand.GetIndex() - MFunction::kFirstVirtualRegisterNumber;
            return std::format("%stack[{}]", prettyVRegNumber);
        }

        return "";
    }

    void emitInstruction(MInstruction& instruction, target::TargetMachine* targetMachine) {
        std::string instrName;
        if (instruction.HasTargetInstruction()) {
            target::InstructionSet* instructions = targetMachine->GetInstructionSet();
            uint targetInstrCode = instruction.GetTargetInstructionCode();
            target::TargetInstruction targetInstr = instructions->GetInstruction(targetInstrCode);

            instrName = targetInstr.GetAsmName();
        } else {
            instrName = instruction.GetOpTypeString();
        }

        std::transform(instrName.begin(), instrName.end(), instrName.begin(), ::toupper);
        m_OutputStream << instrName << " ";

        // TODO: string stream
        std::string operandsString;
        for (MOperand& operand : instruction.GetOperands()) {
            operandsString += getOperandString(operand, targetMachine) + ", ";
        }
        if (instruction.HasOperands()) {
            operandsString.pop_back();
            operandsString.pop_back();
        }

        m_OutputStream << operandsString;
    }

    void emitBasicBlock(MBasicBlock* basicBlock, target::TargetMachine* targetMachine) {
        m_OutputStream << basicBlock->GetName() << ":\n";

        m_OutputStream << "\tPredecessors: ";
        for (MBasicBlock* predBlock : basicBlock->GetPredecessors()) {
            m_OutputStream << predBlock->GetName() << " ";
        }
        m_OutputStream << "\n";

        m_OutputStream << "\tSuccessors: ";
        for (MBasicBlock* succBlock : basicBlock->GetSuccessors()) {
            m_OutputStream << succBlock->GetName() << " ";
        }
        m_OutputStream << "\n\n";

        for (MInstruction& instruction : basicBlock->GetInstructions()) {
            m_OutputStream << "\t";
            emitInstruction(instruction, targetMachine);
            m_OutputStream << "\n";
        }
    }

private:
    std::ofstream m_OutputStream;
};

}  // namespace gen
