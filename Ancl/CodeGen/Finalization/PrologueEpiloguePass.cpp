#include <Ancl/CodeGen/Finalization/PrologueEpiloguePass.hpp>

#include <ranges>

#include <Ancl/CodeGen/Selection/InstructionSelector.hpp>
#include <Ancl/DataLayout/Alignment.hpp>


namespace gen {

PrologueEpiloguePass::PrologueEpiloguePass(MIRProgram& program, target::TargetMachine* targetMachine)
    : m_Program(program), m_TargetMachine(targetMachine) {}

void PrologueEpiloguePass::Run() {
    for (auto& functionPtr : m_Program.GetFunctions()) {
        MFunction* function = functionPtr.get();

        reset();

        saveARP(function);
        adjustStackCallDown(function);
        adjustStackCallUp(function);
        restoreARP(function);

        generatePrologue(function);
        generateEpilogue(function);
    }
}

void PrologueEpiloguePass::adjustStackCallImpl(MFunction* function, bool isDown) {
    LocalDataArea localData = function->GetLocalDataArea();
    uint64_t stackSize = localData.GetAreaSize();

    target::RegisterSet* registers = m_TargetMachine->GetRegisterSet();
    target::TargetABI* targetABI = m_TargetMachine->GetABI();

    if (!function->IsCaller() && stackSize <= targetABI->GetRedZoneSize()) {
        return;
    }

    uint64_t stackAlignment = targetABI->GetStackAlignment();
    uint64_t alignedSize = ir::Alignment::Align(stackSize, stackAlignment);
    if (alignedSize == 0) {
        return;
    }
    
    auto instrOpType = MInstruction::OpType::kAdd;
    if (isDown) {
        instrOpType = MInstruction::OpType::kSub;
    }

    MInstruction adjustInstr(instrOpType);
    adjustInstr.AddPhysicalRegister(registers->GetSP());
    adjustInstr.AddPhysicalRegister(registers->GetSP());
    adjustInstr.AddImmInteger(alignedSize);
    MInstruction targetAdjust = InstructionSelector::SelectInstruction(adjustInstr, m_TargetMachine);

    if (isDown) {
        insertToPrologue(targetAdjust);
    } else {
        insertToEpilogue(targetAdjust);
    }
}

void PrologueEpiloguePass::adjustStackCallDown(MFunction* function) {
    adjustStackCallImpl(function, /*isDown=*/true);
}

void PrologueEpiloguePass::adjustStackCallUp(MFunction* function) {
    adjustStackCallImpl(function, /*isDown=*/false);
}

void PrologueEpiloguePass::saveARP(MFunction* function) {
    target::RegisterSet* registers = m_TargetMachine->GetRegisterSet();

    MInstruction pushARP(MInstruction::OpType::kPush);
    pushARP.AddPhysicalRegister(registers->GetARP());
    MInstruction targetPushARP = InstructionSelector::SelectInstruction(pushARP, m_TargetMachine);

    MInstruction movARP(MInstruction::OpType::kMov);
    movARP.AddPhysicalRegister(registers->GetARP());
    movARP.AddPhysicalRegister(registers->GetSP());
    MInstruction targetMovARP = InstructionSelector::SelectInstruction(movARP, m_TargetMachine);

    insertToPrologue(targetPushARP);
    insertToPrologue(targetMovARP);
}

void PrologueEpiloguePass::restoreARP(MFunction* function) {
    target::RegisterSet* registers = m_TargetMachine->GetRegisterSet();

    MInstruction popARP(MInstruction::OpType::kPop);
    popARP.AddPhysicalRegister(registers->GetARP());
    MInstruction targetPopARP = InstructionSelector::SelectInstruction(popARP, m_TargetMachine);

    insertToEpilogue(targetPopARP);
}

void PrologueEpiloguePass::insertToPrologue(const MInstruction& instruction) {
    m_PrologueInstructions.push_back(instruction);
}

void PrologueEpiloguePass::insertToEpilogue(const MInstruction& instruction) {
    m_EpilogueInstructions.push_back(instruction);
}

void PrologueEpiloguePass::reset() {
    m_PrologueInstructions.clear();
    m_EpilogueInstructions.clear();
}

void PrologueEpiloguePass::generatePrologue(MFunction* function) {
    MBasicBlock* entryBlock = function->GetFirstBasicBlock();
    for (const MInstruction& instruction : m_PrologueInstructions | std::views::reverse) {
        entryBlock->AddInstructionToBegin(instruction);
    }
}

void PrologueEpiloguePass::generateEpilogue(MFunction* function) {
    MBasicBlock* endBlock = function->GetLastBasicBlock();

    MBasicBlock::TInstructionIt returnInstructionIt = --endBlock->GetInstrEnd();
    assert(returnInstructionIt->IsReturn());

    MBasicBlock::TInstructionIt instrIt = returnInstructionIt;
    for (const MInstruction& instruction : m_EpilogueInstructions | std::views::reverse) {
        instrIt = endBlock->InsertBefore(instruction, instrIt);
    } 
}

}  // namespace gen
