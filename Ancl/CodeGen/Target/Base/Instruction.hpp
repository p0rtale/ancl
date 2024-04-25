#pragma once

#include <string>
#include <vector>

#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>


namespace target {

using TInstrCode = uint;
using TOperandClass = uint;

class TargetInstruction {
public:
    TargetInstruction(gen::MInstruction::OpType mInstrType, 
                      TInstrCode instrCode,
                      std::vector<TOperandClass> operandClasses,
                      const std::string& asmTemplate)
        : m_MInstrType(mInstrType), m_InstrCode(instrCode),
          m_OperandClasses(operandClasses), m_AsmTemplate(asmTemplate) {}

    gen::MInstruction::OpType GetMInstructionType() const {
        return m_MInstrType;
    }

    TInstrCode GetInstructionCode() const {
        return m_InstrCode;
    }

    size_t GetOperandsNumber() const {
        return m_OperandClasses.size();
    }

    TOperandClass GetOperandClass(size_t idx) {
        return m_OperandClasses.at(idx);
    }

    std::string GetAsmTemplate() const {
        return m_AsmTemplate;
    }

private:
    gen::MInstruction::OpType m_MInstrType;
    TInstrCode m_InstrCode;

    std::vector<TOperandClass> m_OperandClasses;

    std::string m_AsmTemplate;
};

}  // namespace target
