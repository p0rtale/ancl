#include <Ancl/CodeGen/Target/AMD64/AMD64Machine.hpp>


namespace gen::target::amd64 {

void AMD64TargetMachine::Select(SelectionTree& tree) {
    auto& root = tree.GetRoot();
    selectNode(root.get());
}

void AMD64TargetMachine::selectNode(SelectionNode* node) {
    if (!node) {
        return;
    }

    // TODO: Handle already set target instructions
    if (node->IsSelected()) {
        for (SelectionNode* child : node->GetChildren()) {
            selectNode(child);
        }
        return;
    }

    target::Legalizer* legalizer = GetLegalizer();
    legalizer->Legalize(node);

    MInstruction mirInstruction = node->GetInstruction();
    switch (mirInstruction.GetOpType()) {
        case MInstruction::OpType::kMul:
            return selectMul(node);
        case MInstruction::OpType::kFMul:
            return selectFMul(node);

        case MInstruction::OpType::kSDiv:
            return selectSDiv(node);
        case MInstruction::OpType::kUDiv:
            return selectUDiv(node);
        case MInstruction::OpType::kFDiv:
            return selectFDiv(node);

        case MInstruction::OpType::kSRem:
            return selectSRem(node);
        case MInstruction::OpType::kURem:
            return selectURem(node);

        case MInstruction::OpType::kAdd:
            return selectAdd(node);
        case MInstruction::OpType::kFAdd:
            return selectFAdd(node);

        case MInstruction::OpType::kSub:
            return selectSub(node);
        case MInstruction::OpType::kFSub:
            return selectFSub(node);

        case MInstruction::OpType::kShiftL:
            return selectShiftL(node);
        case MInstruction::OpType::kLShiftR:
            return selectLShiftR(node);
        case MInstruction::OpType::kAShiftR:
            return selectAShiftR(node);

        case MInstruction::OpType::kAnd:
            return selectAnd(node);
        case MInstruction::OpType::kOr:
            return selectOr(node);
        case MInstruction::OpType::kXor:
            return selectXor(node);

        case MInstruction::OpType::kCmp:
        case MInstruction::OpType::kUCmp:
        case MInstruction::OpType::kFCmp:
            return selectCmp(node);

        case MInstruction::OpType::kITrunc:
            return selectITrunc(node);
        case MInstruction::OpType::kFTrunc:
            return selectFTrunc(node);
        case MInstruction::OpType::kZExt:
            return selectZExt(node);
        case MInstruction::OpType::kSExt:
            return selectSExt(node);
        case MInstruction::OpType::kFExt:
            return selectFExt(node);
        case MInstruction::OpType::kFToUI:
            return selectFToUI(node);
        case MInstruction::OpType::kFToSI:
            return selectFToSI(node);
        case MInstruction::OpType::kUIToF:
            return selectUIToF(node);
        case MInstruction::OpType::kSIToF:
            return selectSIToF(node);

        case MInstruction::OpType::kCall:
            return selectCall(node);
        case MInstruction::OpType::kJump:
            return selectJump(node);
        case MInstruction::OpType::kBranch:
            return selectBranch(node);
        case MInstruction::OpType::kRet:
            return selectRet(node);

        case MInstruction::OpType::kMov:
            return selectMov(node);
        case MInstruction::OpType::kFMov:
            return selectFMov(node);

        case MInstruction::OpType::kLoad:
            return selectLoad(node);
        case MInstruction::OpType::kStore:
            return selectStore(node);

        case MInstruction::OpType::kStackAddress:
            return selectStackAddress(node);
        case MInstruction::OpType::kGlobalAddress:
            return selectGlobalAddress(node);
        case MInstruction::OpType::kMemberAddress:
            return selectMemberAddress(node);

        case MInstruction::OpType::kPush:
            return selectPush(node);
        case MInstruction::OpType::kPop:
            return selectPop(node);

        case MInstruction::OpType::kPhi:
            return selectPhi(node);

        default:
            ANCL_CRITICAL("AMD64 Selection: instruction was not selected");
            break;
    }
}

std::vector<gen::MOperand> AMD64TargetMachine::mergeMemberAddress(SelectionNode* node) {
    MInstruction nodeInstruction = node->GetInstruction();

    MOperand baseRegister = *nodeInstruction.GetUse(0);
    MOperand scaleImmediate = *nodeInstruction.GetUse(1);
    MOperand indexRegister = *nodeInstruction.GetUse(2);
    MOperand dispImmediate = *nodeInstruction.GetUse(3);

    SelectionNode* childBaseRegisterNode = node->GetChild(0);
    while (childBaseRegisterNode) {
        MInstruction childInstruction = childBaseRegisterNode->GetInstruction();
        if (childInstruction.IsStackAddress() || childInstruction.IsGlobalAddress()) {
            if (childInstruction.IsGlobalAddress() && indexRegister.GetRegister()) {
                break;
            }
            childBaseRegisterNode->MarkAsSelected();
            baseRegister = *childInstruction.GetUse(0);
            break;
        }

        if (!childInstruction.IsMemberAddress()) {
            break;
        }

        baseRegister = *childInstruction.GetUse(0);
        MOperand childScaleImmediate = *childInstruction.GetUse(1);
        MOperand childIndexRegister = *childInstruction.GetUse(2);
        MOperand childDispImmediate = *childInstruction.GetUse(3);

        if (indexRegister.GetRegister() && childIndexRegister.GetRegister()) {
            break;
        }

        if (childIndexRegister.GetRegister()) {
            scaleImmediate = childScaleImmediate;
            indexRegister = childIndexRegister;
        }
        dispImmediate.SetImmInteger(dispImmediate.GetImmInteger() + childDispImmediate.GetImmInteger());

        childBaseRegisterNode->MarkAsSelected();

        childBaseRegisterNode = childBaseRegisterNode->GetChild(0);
    }

    return {baseRegister, scaleImmediate, indexRegister, dispImmediate};
}

void AMD64TargetMachine::legalizeAddressScale(SelectionNode* node,
                                              std::vector<MOperand>& memoryOperands) {
    MOperand indexRegister = memoryOperands[2];
    if (indexRegister.IsInvalidRegister()) {
        return;
    }

    int64_t scale = memoryOperands[1].GetImmInteger();
    if (scale != 1 && scale != 2 && scale != 4 && scale != 8) {
        MOperand indexRegister = memoryOperands[2];
        MOperand scaleImmediate = memoryOperands[1];

        MInstruction scaleMul{MInstruction::OpType::kMul};
        scaleMul.SetBasicBlock(node->GetBasicBlock());
        scaleMul.AddOperand(indexRegister);
        scaleMul.AddOperand(indexRegister);
        scaleMul.AddOperand(scaleImmediate);

        scaleMul.SetTargetInstructionCode(AMD64InstructionSet::IMUL_RRI);
        scaleMul.SetInstructionClass(indexRegister.GetRegisterClass());

        node->AddPrologueInstruction(scaleMul);

        memoryOperands[1] = MOperand::CreateImmInteger(1);
    }
}

std::vector<gen::MOperand> AMD64TargetMachine::selectMemoryOperand(SelectionNode* node) {
    node->MarkAsSelected();

    MInstruction nodeInstruction = node->GetInstruction();

    // BaseReg/BaseSymbol + ScaleImm * IndexReg + DispImm
    if (nodeInstruction.IsStackAddress() || nodeInstruction.IsGlobalAddress()) {
        MOperand baseRegister = *nodeInstruction.GetUse(0);
        auto scaleImmediate = MOperand::CreateImmInteger(0);
        auto indexRegister = MOperand::CreateRegister(0, MType::CreateScalar(GetPointerByteSize()));
        auto dispImmediate = MOperand::CreateImmInteger(0);

        return {baseRegister, scaleImmediate, indexRegister, dispImmediate};
    }

    assert(nodeInstruction.IsMemberAddress());

    std::vector<gen::MOperand> memberAddressOperands = mergeMemberAddress(node);
    legalizeAddressScale(node, memberAddressOperands);
    return memberAddressOperands;
}

std::vector<gen::MOperand> AMD64TargetMachine::trySelectMemory(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    if (instruction.IsLoad()) {
        if (SelectionNode* loadChild = node->GetChild(0)) {
            node->MarkAsSelected();
            return selectMemoryOperand(loadChild);
        }
    }

    return {*instruction.GetDefinition()};
}

void AMD64TargetMachine::finalizeSelect(SelectionNode* node,
                                        std::vector<MInstruction>& targetInstructions) {
    node->MarkAsSelected();
    for (MInstruction& instruction : targetInstructions) {
        node->AddTargetInstruction(instruction);
    }

    for (SelectionNode* child : node->GetChildren()) {
        selectNode(child);
    }
}

void AMD64TargetMachine::finalizeSelect(SelectionNode* node,
                                        MInstruction& targetInstruction) {
    std::vector<MInstruction> instructions = {targetInstruction};
    finalizeSelect(node, instructions);
}

void AMD64TargetMachine::selectMul(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    MInstruction::TOperandIt defOperand = instruction.GetDefinition();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetInstructionClass(defOperand->GetRegisterClass());
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());
    resultInstruction.AddOperand(*defOperand);

    bool isMemory = false;

    MInstruction::TOperandIt firstUse = instruction.GetUse(0);
    MInstruction::TOperandIt secondUse = instruction.GetUse(1);

    if (secondUse->IsImmediate()) {
        if (SelectionNode* child = node->GetChild(0)) {
            std::vector<MOperand> operands = trySelectMemory(child);
            if (operands.size() > 1) {
                isMemory = true;
            }
            for (const MOperand& operand : operands) {
                resultInstruction.AddOperand(operand);
            }
        } else {
            resultInstruction.AddOperand(*firstUse);
        }

        resultInstruction.AddOperand(*secondUse);
    } else {
        resultInstruction.AddOperand(*firstUse);

        if (SelectionNode* child = node->GetChild(1)) {
            std::vector<MOperand> operands = trySelectMemory(child);
            if (operands.size() > 1) {
                isMemory = true;
            }
            for (const MOperand& operand : operands) {
                resultInstruction.AddOperand(operand);
            }
        } else {
            resultInstruction.AddOperand(*secondUse);
        }
    }

    if (secondUse->IsImmediate()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::IMUL_RRI);
        if (isMemory) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::IMUL_RMI);
        }
    } else if (secondUse->IsVRegister()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::IMUL_RR);
        if (isMemory) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::IMUL_RM);
        }
    }

    finalizeSelect(node, resultInstruction);
}

gen::MInstruction AMD64TargetMachine::selectRMImpl(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt definition = instruction.GetDefinition();
    resultInstruction.SetInstructionClass(definition->GetRegisterClass());

    resultInstruction.AddOperand(*definition);

    MInstruction::TOperandIt firstUse = instruction.GetUse(0);
    resultInstruction.AddOperand(*firstUse);

    MInstruction::TOperandIt secondUse = instruction.GetUse(1);
    if (SelectionNode* child = node->GetChild(1)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*secondUse);
    }

    return resultInstruction;
}

void AMD64TargetMachine::selectFMul(SelectionNode* node) {
    MInstruction resultInstruction = selectRMImpl(node);
    MInstruction::TOperandIt definition = resultInstruction.GetDefinition();

    MType defType = definition->GetType();
    bool isDouble = false;
    if (defType.GetBytes() == 8) {
        isDouble = true;
        resultInstruction.SetInstructionClass(FR64);
    } else {
        resultInstruction.SetInstructionClass(FR32);
    }

    bool isMemory = resultInstruction.GetOperandsNumber() > 3;
    if (isMemory) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MULSS_RM);
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MULSD_RM);
        }
    } else {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MULSS_RR);
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MULSD_RR);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectSDivImpl(SelectionNode* node) {
    // SDIV vreg
    // NB: There are still two nodes here. Operand corresponds to the second node

    MInstruction instruction = node->GetInstruction();
    MInstruction::TOperandIt divOperand = instruction.GetOperand(0);

    std::vector<MInstruction> targetInstructions;

    MInstruction convInstr{MInstruction::OpType::kNone};
    convInstr.SetBasicBlock(instruction.GetBasicBlock());
    convInstr.SetTargetInstructionCode(AMD64InstructionSet::CDQ);
    if (divOperand->GetRegisterClass() == GR64) {
        convInstr.SetTargetInstructionCode(AMD64InstructionSet::CQO);
    }
    convInstr.Undefine();

    targetInstructions.push_back(convInstr);

    MInstruction divInstr{MInstruction::OpType::kSDiv};
    divInstr.SetBasicBlock(instruction.GetBasicBlock());

    divInstr.SetInstructionClass(divOperand->GetRegisterClass());

    bool isMemory = false;
    if (SelectionNode* child = node->GetChild(1)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        if (operands.size() > 1) {
            isMemory = true;
        }
        for (const MOperand& operand : operands) {
            divInstr.AddOperand(operand);
        }
    } else {
        divInstr.AddOperand(*divOperand);
    }

    divInstr.SetTargetInstructionCode(AMD64InstructionSet::IDIV_R);
    if (isMemory) {
        divInstr.SetTargetInstructionCode(AMD64InstructionSet::IDIV_M);
    }
    targetInstructions.push_back(divInstr);

    finalizeSelect(node, targetInstructions);
}

void AMD64TargetMachine::selectSDiv(SelectionNode* node) {
    selectSDivImpl(node);
}

void AMD64TargetMachine::selectUDivImpl(SelectionNode* node) {
    // UDIV vreg
    // NB: There are still two nodes here. Operand corresponds to the second node

    MInstruction instruction = node->GetInstruction();
    MInstruction::TOperandIt divOperand = instruction.GetOperand(0);

    std::vector<MInstruction> targetInstructions;

    // Zero edx/rdx using xor
    MInstruction xorInstr{MInstruction::OpType::kXor};
    xorInstr.SetBasicBlock(instruction.GetBasicBlock());
    xorInstr.SetTargetInstructionCode(AMD64InstructionSet::XOR_RR);

    target::Register edx = m_RegisterSet->GetRegister(AMD64RegisterSet::EDX);
    xorInstr.SetInstructionClass(m_RegisterSet->GetRegisterClass(edx));

    xorInstr.AddPhysicalRegister(edx);
    xorInstr.AddPhysicalRegister(edx);
    targetInstructions.push_back(xorInstr);

    MInstruction divInstr{MInstruction::OpType::kUDiv};
    divInstr.SetBasicBlock(instruction.GetBasicBlock());

    divInstr.SetInstructionClass(divOperand->GetRegisterClass());

    bool isMemory = false;
    if (SelectionNode* child = node->GetChild(1)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        if (operands.size() > 1) {
            isMemory = true;
        }
        for (const MOperand& operand : operands) {
            divInstr.AddOperand(operand);
        }
    } else {
        divInstr.AddOperand(*divOperand);
    }

    divInstr.SetTargetInstructionCode(AMD64InstructionSet::DIV_R);
    if (isMemory) {
        divInstr.SetTargetInstructionCode(AMD64InstructionSet::DIV_M);
    }
    targetInstructions.push_back(divInstr);

    finalizeSelect(node, targetInstructions);
}

void AMD64TargetMachine::selectUDiv(SelectionNode* node) {
    selectUDivImpl(node);
}

void AMD64TargetMachine::selectFDiv(SelectionNode* node) {
    MInstruction resultInstruction = selectRMImpl(node);
    MInstruction::TOperandIt definition = resultInstruction.GetDefinition();

    MType defType = definition->GetType();
    bool isDouble = false;
    if (defType.GetBytes() == 8) {
        isDouble = true;
        resultInstruction.SetInstructionClass(FR64);
    } else {
        resultInstruction.SetInstructionClass(FR32);
    }

    bool isMemory = resultInstruction.GetOperandsNumber() > 3;
    if (isMemory) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::DIVSS_RM);
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::DIVSD_RM);
        }
    } else {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::DIVSS_RR);
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::DIVSD_RR);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectSRem(SelectionNode* node) {
    selectSDivImpl(node);
}

void AMD64TargetMachine::selectURem(SelectionNode* node) {
    selectUDivImpl(node);
}

void AMD64TargetMachine::selectAdd(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt definition = instruction.GetDefinition();
    resultInstruction.AddOperand(*definition);

    resultInstruction.SetInstructionClass(definition->GetRegisterClass());

    MInstruction::TOperandIt firstUse = instruction.GetUse(0);
    resultInstruction.AddOperand(*firstUse);

    bool isMemory = false;
    MInstruction::TOperandIt secondUse = instruction.GetUse(1);
    if (SelectionNode* child = node->GetChild(1)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        if (operands.size() > 1) {
            isMemory = true;
        }
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*secondUse);
    }

    if (secondUse->IsImmediate()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::ADD_RI);
    } else if (secondUse->IsVRegister()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::ADD_RR);
        if (isMemory) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::ADD_RM);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectFAdd(SelectionNode* node) {
    MInstruction resultInstruction = selectRMImpl(node);
    MInstruction::TOperandIt definition = resultInstruction.GetDefinition();

    MType defType = definition->GetType();
    bool isDouble = false;
    if (defType.GetBytes() == 8) {
        isDouble = true;
        resultInstruction.SetInstructionClass(FR64);
    } else {
        resultInstruction.SetInstructionClass(FR32);
    }

    bool isMemory = resultInstruction.GetOperandsNumber() > 3;
    if (isMemory) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::ADDSS_RM);
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::ADDSD_RM);
        }
    } else {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::ADDSS_RR);
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::ADDSD_RR);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectSub(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    MInstruction::TOperandIt secondUse = instruction.GetUse(1);

    MInstruction resultInstruction = selectRMImpl(node);

    bool isMemory = resultInstruction.GetOperandsNumber() > 3;
    if (secondUse->IsImmediate()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::SUB_RI);
    } else if (secondUse->IsVRegister()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::SUB_RR);
        if (isMemory) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::SUB_RM);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectFSub(SelectionNode* node) {
    MInstruction resultInstruction = selectRMImpl(node);
    MInstruction::TOperandIt definition = resultInstruction.GetDefinition();

    MType defType = definition->GetType();
    bool isDouble = false;
    if (defType.GetBytes() == 8) {
        isDouble = true;
        resultInstruction.SetInstructionClass(FR64);
    } else {
        resultInstruction.SetInstructionClass(FR32);
    }

    bool isMemory = resultInstruction.GetOperandsNumber() > 3;
    if (isMemory) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::SUBSS_RM);
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::SUBSD_RM);
        }
    } else {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::SUBSS_RR);
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::SUBSD_RR);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectShiftL(SelectionNode* node) {
    // ShiftL vreg1, vreg2, CL/imm

    MInstruction instruction = node->GetInstruction();

    MInstruction::TOperandIt definition = instruction.GetDefinition();
    instruction.SetInstructionClass(definition->GetRegisterClass());

    MInstruction::TOperandIt secondUse = instruction.GetOperand(1);
    if (secondUse->IsImmediate()) {
        instruction.SetTargetInstructionCode(AMD64InstructionSet::SHL_RI);
    } else if (secondUse->IsVRegister()) {
        instruction.SetTargetInstructionCode(AMD64InstructionSet::SHL_RCL);
    }

    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectLShiftR(SelectionNode* node) {
    // LShiftR vreg1, vreg2, CL/imm

    MInstruction instruction = node->GetInstruction();

    MInstruction::TOperandIt definition = instruction.GetDefinition();
    instruction.SetInstructionClass(definition->GetRegisterClass());

    MInstruction::TOperandIt secondUse = instruction.GetOperand(1);
    if (secondUse->IsImmediate()) {
        instruction.SetTargetInstructionCode(AMD64InstructionSet::SHR_RI);
    } else if (secondUse->IsVRegister()) {
        instruction.SetTargetInstructionCode(AMD64InstructionSet::SHR_RCL);
    }

    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectAShiftR(SelectionNode* node) {
    // AShiftR vreg1, vreg2, CL/imm

    MInstruction instruction = node->GetInstruction();

    MInstruction::TOperandIt definition = instruction.GetDefinition();
    instruction.SetInstructionClass(definition->GetRegisterClass());

    MInstruction::TOperandIt secondUse = instruction.GetOperand(1);
    if (secondUse->IsImmediate()) {
        instruction.SetTargetInstructionCode(AMD64InstructionSet::SAR_RI);
    } else if (secondUse->IsVRegister()) {
        instruction.SetTargetInstructionCode(AMD64InstructionSet::SAR_RCL);
    }

    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectAnd(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    MInstruction::TOperandIt secondUse = instruction.GetUse(1);

    MInstruction resultInstruction = selectRMImpl(node);

    bool isMemory = resultInstruction.GetOperandsNumber() > 3;
    if (secondUse->IsImmediate()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::AND_RI);
    } else if (secondUse->IsVRegister()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::AND_RR);
        if (isMemory) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::AND_RM);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectXor(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    MInstruction::TOperandIt secondUse = instruction.GetUse(1);

    MInstruction resultInstruction = selectRMImpl(node);

    bool isMemory = resultInstruction.GetOperandsNumber() > 3;
    if (secondUse->IsImmediate()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::XOR_RI);
    } else if (secondUse->IsVRegister()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::XOR_RR);
        if (isMemory) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::XOR_RM);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectOr(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    MInstruction::TOperandIt secondUse = instruction.GetUse(1);

    MInstruction resultInstruction = selectRMImpl(node);

    bool isMemory = resultInstruction.GetOperandsNumber() > 3;
    if (secondUse->IsImmediate()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::OR_RI);
    } else if (secondUse->IsVRegister()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::OR_RR);
        if (isMemory) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::OR_RM);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectCmp(SelectionNode* node) {
    /*
        CMP mem/reg imm/reg
        SETcc reg
    */

    MInstruction instruction = node->GetInstruction();
    
    std::vector<MInstruction> targetInstructions;
    MInstruction targetCmp{MInstruction::OpType::kCmp};
    targetCmp.Undefine();
    targetCmp.SetBasicBlock(instruction.GetBasicBlock());

    bool isMemory = false;

    MInstruction::TOperandIt firstUse = instruction.GetUse(0);
    MInstruction::TOperandIt secondUse = instruction.GetUse(1);

    targetCmp.SetInstructionClass(firstUse->GetRegisterClass());

    if (instruction.GetOpType() == MInstruction::OpType::kFCmp) {
        targetCmp.AddOperand(*firstUse);

        if (SelectionNode* child = node->GetChild(1)) {
            std::vector<MOperand> operands = trySelectMemory(child);
            if (operands.size() > 1) {
                isMemory = true;
            }
            for (const MOperand& operand : operands) {
                targetCmp.AddOperand(operand);
            }
        } else {
            targetCmp.AddOperand(*secondUse);
        }
    } else {
        if (SelectionNode* child = node->GetChild(0)) {
            std::vector<MOperand> operands = trySelectMemory(child);
            if (operands.size() > 1) {
                isMemory = true;
            }
            for (const MOperand& operand : operands) {
                targetCmp.AddOperand(operand);
            }
        } else {
            targetCmp.AddOperand(*firstUse);
        }

        targetCmp.AddOperand(*secondUse);
    }

    if (instruction.GetOpType() == MInstruction::OpType::kFCmp) {
        targetCmp.SetTargetInstructionCode(AMD64InstructionSet::UCOMISS_RR);
        if (isMemory) {
            targetCmp.SetTargetInstructionCode(AMD64InstructionSet::UCOMISS_RM);
        }
    } else {
        if (secondUse->IsImmediate()) {
            targetCmp.SetTargetInstructionCode(AMD64InstructionSet::CMP_RI);
            if (isMemory) {
                targetCmp.SetTargetInstructionCode(AMD64InstructionSet::CMP_MI);
            }
        } else {
            targetCmp.SetTargetInstructionCode(AMD64InstructionSet::CMP_RR);
            if (isMemory) {
                targetCmp.SetTargetInstructionCode(AMD64InstructionSet::CMP_MR);
            }
        }
    }
    targetInstructions.push_back(targetCmp);

    MInstruction targetSetcc{MInstruction::OpType::kNone};
    targetSetcc.Undefine();
    targetSetcc.SetBasicBlock(instruction.GetBasicBlock());
    targetSetcc.AddOperand(*instruction.GetDefinition());

    bool isSignedCmp = (instruction.GetOpType() == MInstruction::OpType::kCmp);
    switch (instruction.GetCompareKind()) {
    case MInstruction::CompareKind::kEqual:
        targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETE);
        break;
    case MInstruction::CompareKind::kNEqual:
        targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETNE);
        break;
    case MInstruction::CompareKind::kGreater:
        if (isSignedCmp) {
            targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETG);
        } else {
            targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETA);
        }
        break;
    case MInstruction::CompareKind::kLess:
        if (isSignedCmp) {
            targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETL);
        } else {
            targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETB);
        }
        break;
    case MInstruction::CompareKind::kGreaterEq:
        if (isSignedCmp) {
            targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETGE);
        } else {
            targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETAE);
        }
        break;
    case MInstruction::CompareKind::kLessEq:
        if (isSignedCmp) {
            targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETLE);
        } else {
            targetSetcc.SetTargetInstructionCode(AMD64InstructionSet::SETBE);
        }
        break;
    default:
        // TODO: Handle error
        break;
    }
    targetInstructions.push_back(targetSetcc);

    finalizeSelect(node, targetInstructions);
}

void AMD64TargetMachine::selectITrunc(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{MInstruction::OpType::kRegToSubreg};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt toOperand = instruction.GetDefinition();
    resultInstruction.SetInstructionClass(toOperand->GetRegisterClass());
    resultInstruction.AddOperand(*toOperand);

    MInstruction::TOperandIt fromOperand = instruction.GetUse(0);
    resultInstruction.AddOperand(*fromOperand);

    resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_RR);

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectFTrunc(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt toOperand = instruction.GetDefinition();
    resultInstruction.AddOperand(*toOperand);

    MInstruction::TOperandIt fromOperand = instruction.GetUse(0);
    resultInstruction.SetInstructionClass(FR64);

    if (SelectionNode* child = node->GetChild(0)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*fromOperand);
    }

    bool isMemory = resultInstruction.GetOperandsNumber() > 2;
    if (isMemory) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTSD2SS_RM);
    } else {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTSD2SS_RR);
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectZExt(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt toOperand = instruction.GetDefinition();
    resultInstruction.AddOperand(*toOperand);

    MInstruction::TOperandIt fromOperand = instruction.GetUse(0);
    resultInstruction.SetInstructionClass(fromOperand->GetRegisterClass());

    if (SelectionNode* child = node->GetChild(0)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*fromOperand);
    }

    bool fromDoubleWord = (fromOperand->GetRegisterClass() == GR32);
    bool isMemory = resultInstruction.GetOperandsNumber() > 2;
    if (isMemory) {
        if (fromDoubleWord) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_RM);
        } else {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVZX_RM);
        }
    } else {
        if (fromDoubleWord) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_RR);
        } else {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVZX_RR);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectSExt(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt toOperand = instruction.GetDefinition();
    resultInstruction.AddOperand(*toOperand);

    MInstruction::TOperandIt fromOperand = instruction.GetUse(0);
    resultInstruction.SetInstructionClass(fromOperand->GetRegisterClass());

    if (SelectionNode* child = node->GetChild(0)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*fromOperand);
    }

    // TODO: Use movsx32 + subreg for r8 -> r16?

    bool fromDoubleWord = (fromOperand->GetRegisterClass() == GR32);
    bool isMemory = resultInstruction.GetOperandsNumber() > 2;
    if (isMemory) {
        if (fromDoubleWord) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSXD_RM);
        } else {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSX_RM);
        }
    } else {
        if (fromDoubleWord) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSXD_RR);
        } else {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSX_RR);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectFExt(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt toOperand = instruction.GetDefinition();
    resultInstruction.AddOperand(*toOperand);

    MInstruction::TOperandIt fromOperand = instruction.GetUse(0);
    resultInstruction.SetInstructionClass(FR32);

    if (SelectionNode* child = node->GetChild(0)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*fromOperand);
    }

    bool isMemory = resultInstruction.GetOperandsNumber() > 2;
    if (isMemory) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTSS2SD_RM);
    } else {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTSS2SD_RR);
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectFToUI(SelectionNode* node) {
    // TODO: ...
    selectFToSI(node);
}

void AMD64TargetMachine::selectFToSI(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt toOperand = instruction.GetDefinition();
    resultInstruction.AddOperand(*toOperand);

    MInstruction::TOperandIt fromOperand = instruction.GetUse(0);

    if (SelectionNode* child = node->GetChild(0)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*fromOperand);
    }

    MType fromType = fromOperand->GetType();
    bool isDouble = false;
    if (fromType.GetBytes() == 8) {
        isDouble = true;
        resultInstruction.SetInstructionClass(FR64);
    } else {
        resultInstruction.SetInstructionClass(FR32);
    }

    bool isMemory = resultInstruction.GetOperandsNumber() > 2;
    if (isMemory) {
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTTSD2SI_RM);
        } else {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTTSS2SI_RM);
        }
    } else {
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTTSD2SI_RR);
        } else {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTTSS2SI_RR);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectUIToF(SelectionNode* node) {
    // TODO: ...
    selectSIToF(node);
}

void AMD64TargetMachine::selectSIToF(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt toOperand = instruction.GetDefinition();
    resultInstruction.AddOperand(*toOperand);

    MInstruction::TOperandIt fromOperand = instruction.GetUse(0);
    resultInstruction.SetInstructionClass(fromOperand->GetRegisterClass());

    if (SelectionNode* child = node->GetChild(0)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*fromOperand);
    }

    MType toType = toOperand->GetType();
    bool isDouble = false;
    if (toType.GetBytes() == 8) {
        isDouble = true;
    }

    bool isMemory = resultInstruction.GetOperandsNumber() > 2;
    if (isMemory) {
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTSI2SD_RM);
        } else {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTSI2SS_RM);
        }
    } else {
        if (isDouble) {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTSI2SD_RR);
        } else {
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::CVTSI2SS_RR);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectCall(SelectionNode* node) {
    // TODO: Handle mem/reg Call
    MInstruction instruction = node->GetInstruction();
    instruction.SetTargetInstructionCode(AMD64InstructionSet::CALL);
    instruction.SetInstructionClass(GR64);
    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectJump(SelectionNode* node) {
    // TODO: Handle reg Jump
    MInstruction instruction = node->GetInstruction();
    instruction.SetTargetInstructionCode(AMD64InstructionSet::JMP);
    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectBranch(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    
    std::vector<MInstruction> targetInstructions;

    // Must be CMP child
    SelectionNode* conditionNode = node->GetChild(0);
    assert(conditionNode);

    MInstruction conditionInstr = conditionNode->GetInstruction();
    assert(conditionInstr.IsCmp());

    conditionNode->MarkAsSelected();

    /*
        Branch condVR labelTrue labelFalse
        ----------------------------------
        CMP mem/reg imm/reg
        Jcc labelTrue
        JMP labelFalse
    */

    // TODO: Handle fall-through
    // TODO: Invert condition

    bool isMemory = false;

    MInstruction targetCmp{MInstruction::OpType::kCmp};
    targetCmp.Undefine();
    targetCmp.SetBasicBlock(conditionInstr.GetBasicBlock());

    MInstruction::TOperandIt firstUse = conditionInstr.GetUse(0);
    MInstruction::TOperandIt secondUse = conditionInstr.GetUse(1);

    targetCmp.SetInstructionClass(firstUse->GetRegisterClass());

    if (SelectionNode* child = conditionNode->GetChild(0)) {
        std::vector<MOperand> operands = trySelectMemory(child);
        if (operands.size() > 1) {
            isMemory = true;
        }
        for (const MOperand& operand : operands) {
            targetCmp.AddOperand(operand);
        }
    } else {
        targetCmp.AddOperand(*firstUse);
    }

    targetCmp.AddOperand(*secondUse);

    if (secondUse->IsImmediate()) {
        targetCmp.SetTargetInstructionCode(AMD64InstructionSet::CMP_RI);
        if (isMemory) {
            targetCmp.SetTargetInstructionCode(AMD64InstructionSet::CMP_MI);
        }
    } else {
        targetCmp.SetTargetInstructionCode(AMD64InstructionSet::CMP_RR);
        if (isMemory) {
            targetCmp.SetTargetInstructionCode(AMD64InstructionSet::CMP_MR);
        }
    }
    targetInstructions.push_back(targetCmp);

    MInstruction targetJcc{MInstruction::OpType::kBranch};
    targetJcc.SetBasicBlock(conditionInstr.GetBasicBlock());
    targetJcc.AddOperand(*instruction.GetUse(1));

    bool isSignedCmp = (conditionInstr.GetOpType() == MInstruction::OpType::kCmp);
    switch (conditionInstr.GetCompareKind()) {
    case MInstruction::CompareKind::kEqual:
        targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JE);
        break;
    case MInstruction::CompareKind::kNEqual:
        targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JNE);
        break;
    case MInstruction::CompareKind::kGreater:
        if (isSignedCmp) {
            targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JG);
        } else {
            targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JA);
        }
        break;
    case MInstruction::CompareKind::kLess:
        if (isSignedCmp) {
            targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JL);
        } else {
            targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JB);
        }
        break;
    case MInstruction::CompareKind::kGreaterEq:
        if (isSignedCmp) {
            targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JGE);
        } else {
            targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JAE);
        }
        break;
    case MInstruction::CompareKind::kLessEq:
        if (isSignedCmp) {
            targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JLE);
        } else {
            targetJcc.SetTargetInstructionCode(AMD64InstructionSet::JBE);
        }
        break;
    default:
        // TODO: Handle error
        break;
    }
    targetInstructions.push_back(targetJcc);

    MOperand falseBasicBlockOperand = *instruction.GetUse(2);
    if (falseBasicBlockOperand.HasBasicBlock()) {
        MInstruction targetJmp{MInstruction::OpType::kJump};
        targetJmp.SetBasicBlock(conditionInstr.GetBasicBlock());
        targetJmp.AddOperand(falseBasicBlockOperand);
        targetJmp.SetTargetInstructionCode(AMD64InstructionSet::JMP);
        targetInstructions.push_back(targetJmp);
    }

    finalizeSelect(node, targetInstructions);
}

void AMD64TargetMachine::selectRet(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    instruction.SetTargetInstructionCode(AMD64InstructionSet::RET);
    instruction.SetInstructionClass(GR64);
    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectMov(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction::TOperandIt definition = instruction.GetDefinition();
    instruction.SetInstructionClass(definition->GetRegisterClass());

    MInstruction::TOperandIt useOperand = instruction.GetUse(0);
    if (useOperand->IsImmediate()) {
        // if (useOperand->GetImmInteger() == 0) {
        //     // XOR reg reg reg
        //     MInstruction targetXor{MInstruction::OpType::kXor};
        //     targetXor.SetBasicBlock(instruction.GetBasicBlock());
        //     targetXor.AddOperand(*definition);
        //     targetXor.AddOperand(*definition);
        //     targetXor.AddOperand(*definition);

        //     instruction = targetXor;
        //     instruction.SetTargetInstructionCode(AMD64InstructionSet::XOR_RR);
        // } else {
            instruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_RI);
        // }
    } else {
        instruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_RR);
    }

    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectFMov(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction::TOperandIt definition = instruction.GetDefinition();
    MType defType = definition->GetType();
    if (defType.GetBytes() == 8) {
        instruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSD_RR);
    } else {
        instruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSS_RR);
    }
    
    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectLoad(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt definition = instruction.GetDefinition();
    resultInstruction.SetInstructionClass(definition->GetRegisterClass());

    resultInstruction.AddOperand(*definition);

    MInstruction::TOperandIt use = instruction.GetUse(0);
    if (SelectionNode* child = node->GetChild(0)) {
        std::vector<MOperand> operands = selectMemoryOperand(child);
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*use);

        // ScaleImm
        resultInstruction.AddOperand(MOperand::CreateImmInteger(0));
        
        // IndexReg
        resultInstruction.AddOperand(MOperand::CreateRegister(0, MType::CreateScalar(GetPointerByteSize())));

        // DispImm
        resultInstruction.AddOperand(MOperand::CreateImmInteger(0));
    }

    MInstruction::TOperandIt destOperand = instruction.GetOperand(0);
    MType destType = destOperand->GetType();
    resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_RM);
    if (destType.IsFloat()) {
        if (destType.GetBytes() == 4) {
            resultInstruction.SetInstructionClass(FR32);
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSS_RM);
        } else {
            resultInstruction.SetInstructionClass(FR64);
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSD_RM);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectStore(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    MInstruction resultInstruction{instruction.GetOpType()};
    resultInstruction.SetBasicBlock(instruction.GetBasicBlock());

    MInstruction::TOperandIt addressOperand = instruction.GetUse(0);
    if (SelectionNode* child = node->GetChild(0)) {
        std::vector<MOperand> operands = selectMemoryOperand(child);
        for (const MOperand& operand : operands) {
            resultInstruction.AddOperand(operand);
        }
    } else {
        resultInstruction.AddOperand(*addressOperand);

        // ScaleImm
        resultInstruction.AddOperand(MOperand::CreateImmInteger(0));
        
        // IndexReg
        resultInstruction.AddOperand(MOperand::CreateRegister(0, MType::CreateScalar(GetPointerByteSize())));

        // DispImm
        resultInstruction.AddOperand(MOperand::CreateImmInteger(0));
    }

    MInstruction::TOperandIt valueOperand = instruction.GetUse(1);
    resultInstruction.SetInstructionClass(valueOperand->GetRegisterClass());
    resultInstruction.AddOperand(*valueOperand);

    MType destType = valueOperand->GetType();
    if (valueOperand->IsImmediate()) {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_MI);
    } else {
        resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOV_MR);
    }
    if (destType.IsFloat()) {
        if (destType.GetBytes() == 4) {
            resultInstruction.SetInstructionClass(FR32);
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSS_MR);
        } else {
            resultInstruction.SetInstructionClass(FR64);
            resultInstruction.SetTargetInstructionCode(AMD64InstructionSet::MOVSD_MR);
        }
    }

    finalizeSelect(node, resultInstruction);
}

void AMD64TargetMachine::selectStackAddress(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    // ScaleImm
    instruction.AddOperand(MOperand::CreateImmInteger(0));
    
    // IndexReg
    instruction.AddOperand(MOperand::CreateRegister(0, MType::CreateScalar(GetPointerByteSize())));

    // DispImm
    instruction.AddOperand(MOperand::CreateImmInteger(0));

    instruction.SetTargetInstructionCode(AMD64InstructionSet::LEA);
    instruction.SetInstructionClass(GR64);

    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectGlobalAddress(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();

    // ScaleImm
    instruction.AddOperand(MOperand::CreateImmInteger(0));
    
    // IndexReg
    instruction.AddOperand(MOperand::CreateRegister(0, MType::CreateScalar(GetPointerByteSize())));

    // DispImm
    instruction.AddOperand(MOperand::CreateImmInteger(0));

    instruction.SetTargetInstructionCode(AMD64InstructionSet::LEA);
    instruction.SetInstructionClass(GR64);

    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectMemberAddress(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    instruction.SetTargetInstructionCode(AMD64InstructionSet::LEA);
    instruction.SetInstructionClass(GR64);

    MOperand baseRegister = *instruction.GetUse(0);
    MOperand scaleImmediate = *instruction.GetUse(1);
    MOperand indexRegister = *instruction.GetUse(2);
    MOperand dispImmediate = *instruction.GetUse(3);

    std::vector<MOperand> operands = {
                baseRegister, scaleImmediate, indexRegister, dispImmediate};
    legalizeAddressScale(node, operands);
    *instruction.GetUse(1) = operands[1];

    finalizeSelect(node, instruction);
}


// TODO: Immediate and memory PUSH
void AMD64TargetMachine::selectPush(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    MInstruction::TOperandIt operand = instruction.GetOperand(0);
    instruction.SetInstructionClass(operand->GetRegisterClass());
    instruction.SetTargetInstructionCode(AMD64InstructionSet::PUSH_R);
    instruction.SetInstructionClass(GR64);
    finalizeSelect(node, instruction);
}

// TODO: Memory POP
void AMD64TargetMachine::selectPop(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    MInstruction::TOperandIt operand = instruction.GetOperand(0);
    instruction.SetInstructionClass(operand->GetRegisterClass());
    instruction.SetTargetInstructionCode(AMD64InstructionSet::POP_R);
    instruction.SetInstructionClass(GR64);
    finalizeSelect(node, instruction);
}

void AMD64TargetMachine::selectPhi(SelectionNode* node) {
    MInstruction instruction = node->GetInstruction();
    instruction.SetTargetInstructionCode(AMD64InstructionSet::INVALID);
    finalizeSelect(node, instruction);
}

}  // namespace gen::target::amd64
