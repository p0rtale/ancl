#include <Ancl/Optimization/DVNTPass.hpp>


namespace ir {

DVNTPass::DVNTPass(Function* function)
    : m_Function(function),
      m_DomTree(function),
      m_Constexpr(function->GetProgram()) {}

void DVNTPass::Run() {
    runPreorderDVNT(m_Function->GetEntryBlock());
}

void DVNTPass::runPreorderDVNT(BasicBlock* basicBlock) {
    allocateScope();

    std::list<Instruction*>& instructions = basicBlock->GetInstructionsRef();
    for (auto instrIt = instructions.begin(); instrIt != instructions.end();) {
        Instruction* instruction = *instrIt;

        bool toDelete = false;

        if (auto* phiInstr = dynamic_cast<PhiInstruction*>(instruction)) {
            if (isMeaninglessPhi(phiInstr)) {
                m_AssignedDefinitions[phiInstr->GetName()] = phiInstr->GetIncomingValue(0);
                toDelete = true;
            } else {
                m_AssignedDefinitions[phiInstr->GetName()] = phiInstr;
            }
        } else {
            std::vector<Value*> operands = instruction->GetOperands();
            for (size_t i = 0; i < operands.size(); ++i) {
                Value* operand = operands[i];
                if (dynamic_cast<ir::Instruction*>(operand)) {
                    Value* assignedDef = m_AssignedDefinitions.at(operand->GetName());
                    instruction->SetOperand(assignedDef, i);
                }
            }

            // TODO: Algebraic simplification

            Constant* result = m_Constexpr.TryToEvaluate(instruction);
            if (result) {
                m_AssignedDefinitions[instruction->GetName()] = result;
                toDelete = true;
            } else if (auto* callInstr = dynamic_cast<CallInstruction*>(instruction)) { 
                Function* callee = callInstr->GetCallee();
                if (callee->HasReturnValue() && isNumberConstant(callee->GetReturnValue())) {
                    m_AssignedDefinitions[callInstr->GetName()] = callee->GetReturnValue();
                } else {
                    m_AssignedDefinitions[callInstr->GetName()] = callInstr;
                }
            } else if (isReducible(instruction)) {
                toDelete = handleReducible(instruction);
            } else if (auto* branch = dynamic_cast<BranchInstruction*>(instruction)) {
                if (branch->IsConditional() && isNumberConstant(branch->GetCondition())) {
                    Value* condition = branch->GetCondition();
                    auto* intConstant = static_cast<IntConstant*>(condition);
                    IntValue intValue = intConstant->GetValue();
                    if (intValue.GetUnsignedValue()) {
                        branch->ToUnconditionalTrue();
                    } else {
                        branch->ToUnconditionalFalse();
                    }
                }
            } else if (!dynamic_cast<ir::VoidType*>(instruction->GetType())) {
                m_AssignedDefinitions[instruction->GetName()] = instruction;
            }
        }

        if (toDelete) {
            instrIt = instructions.erase(instrIt);
        } else {
            ++instrIt;
        }
    }

    for (BasicBlock* successor : basicBlock->GetSuccessors()) {
        std::vector<BasicBlock*> preds = successor->GetPredecessors();
        size_t predIndex = 0;
        for (; predIndex < preds.size(); ++predIndex) {
            if (preds[predIndex] == basicBlock) {
                break;
            }
        }

        for (PhiInstruction* phiInstr : successor->GetPhiFunctions()) {
            Value* argValue = phiInstr->GetIncomingValue(predIndex);
            if (dynamic_cast<ir::Instruction*>(argValue)) {
                phiInstr->SetIncomingValue(predIndex, m_AssignedDefinitions.at(argValue->GetName()));
            }
        }
    }

    for (BasicBlock* child : m_DomTree.GetChildren(basicBlock)) {
        runPreorderDVNT(child);
    }

    freeScope();
}

bool DVNTPass::handleReducible(Instruction* instruction) {
    Value* leftOperand = instruction->GetOperand(0);
    Value* rightOperand = instruction->GetOperand(1);

    if (auto* binary = dynamic_cast<BinaryInstruction*>(instruction)) {
        if (binary->IsCommutative() && leftOperand > rightOperand) {
            std::swap(leftOperand, rightOperand);
        }
    }

    triplet key = constructKey(leftOperand, rightOperand,
                                getReducibleInstructionName(instruction));

    auto definitionOpt = getDefinition(key);
    if (definitionOpt) {
        m_AssignedDefinitions[instruction->GetName()] = *definitionOpt;
        return true;
    }

    m_AssignedDefinitions[instruction->GetName()] = instruction;
    addDefinition(key, instruction);

    return false;
}

void DVNTPass::allocateScope() {
    m_ExprDefinitions.push_back(std::unordered_map<triplet, Value*>{});
}

void DVNTPass::freeScope() {
    m_ExprDefinitions.pop_back();
}

void DVNTPass::addDefinition(const triplet& key, Instruction* instruction) {
    auto& currentScope = m_ExprDefinitions.back();
    currentScope[key] = instruction;
}

std::optional<Value*> DVNTPass::getDefinition(const triplet& key) {
    for (auto it = m_ExprDefinitions.rbegin(); it != m_ExprDefinitions.rend(); ++it) {
        if (it->contains(key)) {
            return (*it)[key];
        }
    }
    return {};
}

bool DVNTPass::isMeaninglessConstPhi(PhiInstruction* phiInstr, const IntValue& intValue) {
    for (size_t i = 0; i < phiInstr->GetArgumentsNumber(); ++i) {
        Value* argValue = phiInstr->GetIncomingValue(i);
        auto* argConst = dynamic_cast<IntConstant*>(argValue);
        if (!argConst) {
            return false;
        }

        IntValue argIntValue = argConst->GetValue();
        if (argIntValue != intValue) {
            return false;
        }
    }

    return true;
}

bool DVNTPass::isMeaninglessPhi(PhiInstruction* phiInstr) {
    Value* firstDef = phiInstr->GetIncomingValue(0);
    if (auto* intConst = dynamic_cast<IntConstant*>(firstDef)) {
        return isMeaninglessConstPhi(phiInstr, intConst->GetValue());
    }
    if (!dynamic_cast<ir::Instruction*>(firstDef)) {
        return false;
    }

    // TODO: Check global variables
    for (size_t i = 1; i < phiInstr->GetArgumentsNumber(); ++i) {
        Value* argValue = phiInstr->GetIncomingValue(i);
        if (!argValue || !dynamic_cast<ir::Instruction*>(argValue)) {
            return false;
        }

        if (firstDef->GetName() != argValue->GetName()) {
            return false;
        }
    }

    return true;
}

bool DVNTPass::isNumberConstant(Value* value) const {
    return dynamic_cast<IntConstant*>(value) || dynamic_cast<FloatConstant*>(value);
}

bool DVNTPass::isReducible(Instruction* instruction) {
    // TODO: It's high time to use our RTTI...
    if (dynamic_cast<BinaryInstruction*>(instruction) || 
            dynamic_cast<CompareInstruction*>(instruction) ||
                dynamic_cast<MemberInstruction*>(instruction)) {
        return true;
    }
    return false;
}

DVNTPass::triplet DVNTPass::constructKey(Value* firstValue, Value* secondValue,
                        const std::string& opString) {
    return {firstValue, opString, secondValue};
}

std::string DVNTPass::getReducibleInstructionName(Instruction* instruction) {
    if (auto* binary = dynamic_cast<BinaryInstruction*>(instruction)) {
        return binary->GetOpTypeStr();
    }

    if (auto* compare = dynamic_cast<CompareInstruction*>(instruction)) {
        return compare->GetOpTypeStr();
    }

    if (auto* member = dynamic_cast<MemberInstruction*>(instruction)) {
        return "member";
    }

    ANCL_CRITICAL("GetInstructionName: Invalid reducible instruction");
    return "";
}

}  // namespace ir
