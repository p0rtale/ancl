#include <Ancl/CodeGen/Target/AMD64/AMD64Legalizer.hpp>

#include <Ancl/CodeGen/MachineIR/MFunction.hpp>
#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>

#include <Ancl/CodeGen/Target/AMD64/AMD64InstructionSet.hpp>
#include <Ancl/CodeGen/Target/AMD64/AMD64RegisterSet.hpp>


namespace gen::target::amd64 {

AMD64Legalizer::AMD64Legalizer(TargetMachine* targetMachine)
    : Legalizer(targetMachine) {}

bool AMD64Legalizer::IsLegalizationRequired(SelectionNode* node) {
    MInstruction& nodeInstruction = node->GetInstructionRef();
    switch (nodeInstruction.GetOpType()) {
        case MInstruction::OpType::kMul:
        case MInstruction::OpType::kSDiv:
        case MInstruction::OpType::kUDiv:
        case MInstruction::OpType::kSRem:
        case MInstruction::OpType::kURem:
        case MInstruction::OpType::kAdd:
        case MInstruction::OpType::kSub:
        case MInstruction::OpType::kShiftL:
        case MInstruction::OpType::kLShiftR:
        case MInstruction::OpType::kAShiftR:
        case MInstruction::OpType::kAnd:
        case MInstruction::OpType::kXor:
        case MInstruction::OpType::kOr:
        case MInstruction::OpType::kCmp:
        case MInstruction::OpType::kUCmp:
        case MInstruction::OpType::kZExt:
            return true;

        default:
            return false;
    }
}

void AMD64Legalizer::LegalizeMul(SelectionNode* node) {
    moveImmediateToEnd(node);
}

void AMD64Legalizer::LegalizeDiv(SelectionNode* node) {
    legalizeDivRem(node, /*isRem=*/false);
}

void AMD64Legalizer::LegalizeRem(SelectionNode* node) {
    legalizeDivRem(node, /*isRem=*/true);
}

void AMD64Legalizer::LegalizeAdd(SelectionNode* node) {
    moveImmediateToEnd(node);
}

void AMD64Legalizer::LegalizeSub(SelectionNode* node) {
    materializeLeftImmediate(node);
}

void AMD64Legalizer::LegalizeShift(SelectionNode* node) {
    materializeLeftImmediate(node);

    MInstruction shiftInstruction = node->GetInstruction();
    MInstruction::TOperandIt secondUse = shiftInstruction.GetUse(1);
    if (secondUse->IsImmediate()) {
        return;
    }

    MType type = secondUse->GetType();
    auto cxRegNumber = AMD64RegisterSet::ECX;
    if (type.GetBytes() == 8) {
        cxRegNumber = AMD64RegisterSet::RCX;
    }

    target::RegisterSet* registers = m_TargetMachine->GetRegisterSet();
    target::Register cxReg = registers->GetRegister(cxRegNumber);

    MInstruction movToCX{MInstruction::OpType::kMov};
    movToCX.SetTargetInstructionCode(AMD64InstructionSet::MOV_RR);
    movToCX.SetInstructionClass(registers->GetRegisterClass(cxReg));
    movToCX.SetBasicBlock(shiftInstruction.GetBasicBlock());

    movToCX.AddPhysicalRegister(cxReg);
    movToCX.AddOperand(*secondUse);

    node->AddPrologueInstruction(movToCX);

    MInstruction newInstruction{shiftInstruction.GetOpType()};
    newInstruction.SetBasicBlock(shiftInstruction.GetBasicBlock());

    newInstruction.AddOperand(*shiftInstruction.GetDefinition());
    newInstruction.AddOperand(*shiftInstruction.GetUse(0));
    newInstruction.AddPhysicalRegister(registers->GetRegister(AMD64RegisterSet::CL));

    node->SetInstruction(newInstruction);
}

void AMD64Legalizer::LegalizeAnd(SelectionNode* node) {
    moveImmediateToEnd(node);
}

void AMD64Legalizer::LegalizeXor(SelectionNode* node) {
    moveImmediateToEnd(node);
}

void AMD64Legalizer::LegalizeOr(SelectionNode* node) {
    moveImmediateToEnd(node);
}

void AMD64Legalizer::LegalizeCmp(SelectionNode* node) {
    // TODO: Swap operands and invert cmp?
    materializeLeftImmediate(node);
}

void AMD64Legalizer::LegalizeZExt(SelectionNode* node) {
    MInstruction& instruction = node->GetInstructionRef();
    MInstruction::TOperandIt fromOperand = instruction.GetUse(0);
    MInstruction::TOperandIt toOperand = instruction.GetDefinition();

    uint64_t toBytes = 4;

    MType toType = toOperand->GetType();
    if (toType.GetBytes() == toBytes) {
        return;
    }

    MBasicBlock* basicBlock = node->GetBasicBlock();
    MFunction* function = basicBlock->GetFunction();
    uint64_t vregNumber = function->NextVReg();
    auto tempRegister = MOperand::CreateRegister(vregNumber, MType::CreateScalar(toBytes, /*isFloat=*/false),
                                                 /*isVirtual=*/true);

    target::RegisterSet* registers = m_TargetMachine->GetRegisterSet();
    tempRegister.SetRegisterClass(registers->GetRegisterClass(/*bytes=*/toBytes, /*isFloat=*/false));

    MInstruction subregInstruction;
    if (toType.GetBytes() < toBytes) {
        subregInstruction = MInstruction{MInstruction::OpType::kRegToSubreg};
    } else {
        subregInstruction = MInstruction{MInstruction::OpType::kSubregToReg};
    }
    subregInstruction.SetBasicBlock(instruction.GetBasicBlock());

    subregInstruction.AddOperand(*toOperand);
    subregInstruction.AddOperand(tempRegister);

    subregInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_RR);
    node->AddEpilogueInstruction(subregInstruction);

    *toOperand = tempRegister;
}

void AMD64Legalizer::legalizeDivRem(SelectionNode* node, bool isRem) {
    MInstruction& instruction = node->GetInstructionRef();
    MInstruction::TOperandIt resOperand = instruction.GetDefinition();
    MInstruction::TOperandIt dividendOperand = instruction.GetUse(0);
    MInstruction::TOperandIt divisorOperand = instruction.GetUse(1);

    MOperand newDivisorOperand = *divisorOperand;
    if (divisorOperand->IsImmediate()) {
        newDivisorOperand = materializeImmediate(*divisorOperand, node);
    }

    // TODO: Generalize?
    MType type = resOperand->GetType();
    auto axRegNumber = AMD64RegisterSet::EAX;
    auto dxRegNumber = AMD64RegisterSet::EDX;
    if (type.GetBytes() == 8) {
        axRegNumber = AMD64RegisterSet::RAX;
        dxRegNumber = AMD64RegisterSet::RDX;
    }

    target::RegisterSet* registers = m_TargetMachine->GetRegisterSet();
    target::Register axReg = registers->GetRegister(axRegNumber);
    target::Register dxReg = registers->GetRegister(dxRegNumber);

    MInstruction movToAX{MInstruction::OpType::kMov};
    movToAX.SetBasicBlock(instruction.GetBasicBlock());
    movToAX.SetTargetInstructionCode(AMD64InstructionSet::MOV_RR);
    movToAX.SetInstructionClass(registers->GetRegisterClass(axReg));

    movToAX.AddPhysicalRegister(axReg);
    movToAX.AddOperand(*dividendOperand);

    node->AddPrologueInstruction(movToAX);

    if (isRem) {
        MInstruction movFromDX{MInstruction::OpType::kMov};
        movFromDX.SetBasicBlock(instruction.GetBasicBlock());
        movFromDX.SetTargetInstructionCode(AMD64InstructionSet::MOV_RR);
        movFromDX.SetInstructionClass(registers->GetRegisterClass(dxReg));

        movFromDX.AddOperand(*resOperand);
        movFromDX.AddPhysicalRegister(dxReg);

        node->AddEpilogueInstruction(movFromDX);
    } else {
        MInstruction movFromAX{MInstruction::OpType::kMov};
        movFromAX.SetBasicBlock(instruction.GetBasicBlock());
        movFromAX.SetTargetInstructionCode(AMD64InstructionSet::MOV_RR);
        movFromAX.SetInstructionClass(registers->GetRegisterClass(axReg));

        movFromAX.AddOperand(*resOperand);
        movFromAX.AddPhysicalRegister(axReg); 

        node->AddEpilogueInstruction(movFromAX);
    }

    MInstruction newInstruction{instruction.GetOpType()};
    newInstruction.SetBasicBlock(instruction.GetBasicBlock());
    newInstruction.AddOperand(newDivisorOperand);
    
    newInstruction.Undefine();

    newInstruction.AddImplicitRegDefinition(registers->GetRegister(axRegNumber));
    newInstruction.AddImplicitRegDefinition(registers->GetRegister(dxRegNumber));
    newInstruction.AddImplicitRegUse(registers->GetRegister(axRegNumber));
    newInstruction.AddImplicitRegUse(registers->GetRegister(dxRegNumber));

    node->SetInstruction(newInstruction);
}

void AMD64Legalizer::moveImmediateToEnd(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction::TOperandIt firstUse = instruction.GetUse(0);
    MInstruction::TOperandIt secondUse = instruction.GetUse(1);
    if (firstUse->IsImmediate()) {
        std::iter_swap(firstUse, secondUse);
    }
}

void AMD64Legalizer::materializeLeftImmediate(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction newInstruction{instruction.GetOpType()};
    newInstruction.SetBasicBlock(instruction.GetBasicBlock());
    if (instruction.IsDefinition()) {
        newInstruction.AddOperand(*instruction.GetDefinition());
    }

    MInstruction::TOperandIt firstUse = instruction.GetUse(0);
    MInstruction::TOperandIt secondUse = instruction.GetUse(1);
    if (firstUse->IsImmediate()) {
        MOperand regOperand = materializeImmediate(*firstUse, node);
        newInstruction.AddOperand(regOperand);
    } else {
        newInstruction.AddOperand(*firstUse);
    }
    newInstruction.AddOperand(*secondUse);

    node->SetInstruction(newInstruction);
}

MOperand AMD64Legalizer::materializeImmediate(const MOperand& operand, SelectionNode* node) {
    auto movType = MInstruction::OpType::kMov;

    MBasicBlock* basicBlock = node->GetBasicBlock();
    MFunction* function = basicBlock->GetFunction();
    uint64_t vregNumber = function->NextVReg();

    auto vreg = MOperand::CreateRegister(vregNumber, operand.GetType(), /*isVirtual=*/true);

    target::RegisterSet* registers = m_TargetMachine->GetRegisterSet();
    MType opType = vreg.GetType();
    vreg.SetRegisterClass(registers->GetRegisterClass(opType.GetBytes(), opType.IsFloat()));

    MInstruction movInstruction{movType};
    movInstruction.SetBasicBlock(basicBlock);
    movInstruction.AddOperand(vreg);
    movInstruction.AddOperand(operand);

    movInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_RI);
    movInstruction.SetInstructionClass(vreg.GetRegisterClass());

    node->AddPrologueInstruction(movInstruction);

    return vreg;
}

}  // namespace gen::target::amd64
