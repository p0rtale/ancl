#pragma once

#include <string>
#include <vector>


namespace gen::target {

using TInstrCode = uint;
using TOperandClass = std::vector<uint>;

class TargetInstruction {
public:
    TargetInstruction(TInstrCode instrCode,
                      std::vector<TOperandClass> operandClasses,
                      const std::string& asmName)
        : m_InstrCode(instrCode),
          m_OperandClasses(operandClasses), m_AsmName(asmName) {}

    TInstrCode GetInstructionCode() const {
        return m_InstrCode;
    }

    size_t GetOperandsNumber() const {
        return m_OperandClasses.size();
    }

    TOperandClass GetOperandClass(size_t idx) {
        return m_OperandClasses.at(idx);
    }

    std::string GetAsmName() const {
        return m_AsmName;
    }

private:
    TInstrCode m_InstrCode;

    std::vector<TOperandClass> m_OperandClasses;

    std::string m_AsmName;
};

}  // namespace target
