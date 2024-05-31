#include <Ancl/CodeGen/MachineIR/MInstruction.hpp>

#include <cassert>


namespace gen {

MInstruction::MInstruction(OpType opType)
    : m_OpType(opType) {}

MInstruction::MInstruction(OpType opType, CompareKind compareKind)
    : m_OpType(opType), m_CompareKind(compareKind) {}

MInstruction::OpType MInstruction::GetOpType() const {
    return m_OpType;
}

std::string MInstruction::GetOpTypeString() const {
    if (m_OpType == OpType::kCmp || m_OpType == OpType::kUCmp || m_OpType == OpType::kFCmp) {
        std::string name = "CMP";
        if (m_OpType == OpType::kUCmp) {
            name = "UCMP";
        } else if (m_OpType == OpType::kFCmp) {
            name = "FCMP";
        }

        switch (m_CompareKind){
            case CompareKind::kEqual:
                return name + "EQ";
            case CompareKind::kNEqual:
                return name + "NE";
            case CompareKind::kGreater:
                return name + "GT";
            case CompareKind::kLess:
                return name + "LT";
            case CompareKind::kGreaterEq:
                return name + "GE";
            case CompareKind::kLessEq:
                return name + "LE";
            default:
                return name;
        }
    }

    switch (m_OpType) {
        case OpType::kNone:
            return "";
        case OpType::kMul:
            return "MUL";
        case OpType::kFMul:
            return "FMUL";
        case OpType::kSDiv:
            return "SDIV";
        case OpType::kUDiv:
            return "UDIV";
        case OpType::kFDiv:
            return "FDIV";
        case OpType::kSRem:
            return "SREM";
        case OpType::kURem:
            return "UREM";
        case OpType::kAdd:
            return "ADD";
        case OpType::kFAdd:
            return "FADD";
        case OpType::kSub:
            return "SUB";
        case OpType::kFSub:
            return "FSUB";
        case OpType::kShiftL:
            return "SHIFTL";
        case OpType::kLShiftR:
            return "LSHIFTR";
        case OpType::kAShiftR:
            return "ASHIFTR";
        case OpType::kAnd:
            return "AND";
        case OpType::kXor:
            return "XOR";
        case OpType::kOr:
            return "OR";
        case OpType::kITrunc:
            return "ITRUNC";
        case OpType::kFTrunc:
            return "FTRUNC";
        case OpType::kZExt:
            return "ZEXT";
        case OpType::kSExt:
            return "SEXT";
        case OpType::kFExt:
            return "FEXT";
        case OpType::kFToUI:
            return "FTOUI";
        case OpType::kFToSI:
            return "FTOSI";
        case OpType::kUIToF:
            return "UITOF";
        case OpType::kSIToF:
            return "SITOF";
        case OpType::kCall:
            return "CALL";
        case OpType::kJump:
            return "JUMP";
        case OpType::kBranch:
            return "BRANCH";
        case OpType::kRet:
            return "RET";
        case OpType::kMov:
            return "MOV";
        case OpType::kFMov:
            return "FMOV";
        case OpType::kLoad:
            return "LOAD";
        case OpType::kStore:
            return "STORE";
        case OpType::kStackAddress:
            return "STACKADDR";
        case OpType::kGlobalAddress:
            return "GLOBALADDR";
        case OpType::kMemberAddress:
            return "MEMBERADDR";
        case OpType::kPush:
            return "PUSH";
        case OpType::kPop:
            return "POP";
        case OpType::kPhi:
            return "PHI";
        case OpType::kSubregToReg:
            return "SUBREG_TO_REG";
        case OpType::kRegToSubreg:
            return "REG_TO_SUBREG";
        default:
            return "";
    }
}

bool MInstruction::IsCall() const {
    return m_OpType == OpType::kCall;
}

bool MInstruction::IsReturn() const {
    return m_OpType == OpType::kRet;
}

bool MInstruction::IsJump() const {
    return m_OpType == OpType::kJump;
}

bool MInstruction::IsBranch() const {
    return m_OpType == OpType::kBranch;
}

bool MInstruction::IsTerminator() const {
    return IsJump() || IsBranch() || IsReturn();
}

bool MInstruction::IsStore() const {
    return m_OpType == OpType::kStore;
}

bool MInstruction::IsLoad() const {
    return m_OpType == OpType::kLoad;
}

bool MInstruction::IsStackAddress() const {
    return m_OpType == OpType::kStackAddress;
}

bool MInstruction::IsGlobalAddress() const {
    return m_OpType == OpType::kGlobalAddress;
}

bool MInstruction::IsMemberAddress() const {
    return m_OpType == OpType::kMemberAddress;
}

bool MInstruction::IsCmp() const {
    return m_OpType == OpType::kCmp;
}

bool MInstruction::IsMov() const {
    return m_OpType == OpType::kMov || m_OpType == OpType::kFMov;
}

bool MInstruction::IsRegMov() {
    if (!IsMov()) {
        return false;
    }

    TOperandIt toOperand = GetDefinition();
    TOperandIt fromOperand = GetUse(0);

    return toOperand->IsRegister() && fromOperand->IsRegister();
}

bool MInstruction::IsSubregToReg() const {
    return m_OpType == OpType::kSubregToReg;
}

bool MInstruction::IsRegToSubreg() const {
    return m_OpType == OpType::kRegToSubreg;
}

bool MInstruction::IsPush() const {
    return m_OpType == OpType::kPush;
}

bool MInstruction::IsPop() const {
    return m_OpType == OpType::kPop;
}

bool MInstruction::IsPhi() const {
    return m_OpType == OpType::kPhi;
}

MInstruction::CompareKind MInstruction::GetCompareKind() const {
    return m_CompareKind;
}

bool MInstruction::HasBasicBlock() const {
    return m_BasicBlock;
}

void MInstruction::SetBasicBlock(MBasicBlock* MBB) {
    m_BasicBlock = MBB;
}

MBasicBlock* MInstruction::GetBasicBlock() const {
    return m_BasicBlock;
}

void MInstruction::AddOperand(MOperand operand) {
    m_Operands.push_back(operand);
}

void MInstruction::AddOperandToBegin(MOperand operand) {
    m_Operands.push_front(operand);
}

void MInstruction::RemoveFirstOperand() {
    m_Operands.pop_front();
}

void MInstruction::AddImplicitRegDefinition(target::Register reg) {
    m_ImplicitRegDefinitions.push_back(reg);
}

void MInstruction::AddImplicitRegUse(target::Register reg) {
    m_ImplicitRegUses.push_back(reg);
}

void MInstruction::AddPhysicalRegister(target::Register reg) {
    MType type;
    if (reg.IsFloat()) {
        type = MType(MType::Kind::kFloat, reg.GetBytes());
    } else {
        type = MType(MType::Kind::kInteger, reg.GetBytes());
    }
    AddOperand(MOperand::CreateRegister(reg.GetNumber(), type, /*isVirtual=*/false));
}

void MInstruction::AddVirtualRegister(uint64_t regNumber, MType type) {
    AddOperand(MOperand::CreateRegister(regNumber, type, /*isVirtual=*/true));
}

void MInstruction::AddImmInteger(int64_t value, uint64_t bytes) {
    AddOperand(MOperand::CreateImmInteger(value, bytes));
}  

void MInstruction::AddImmFloat(double value, uint64_t bytes) {
    AddOperand(MOperand::CreateImmFloat(value, bytes));
}  

void MInstruction::AddGlobalSymbol(const std::string& symbol) {
    AddOperand(MOperand::CreateGlobalSymbol(symbol));
}

void MInstruction::AddFunction(const std::string& symbol) {
    AddOperand(MOperand::CreateFunction(symbol));
}

void MInstruction::AddStackIndex(uint64_t slot, int64_t offset) {
    AddOperand(MOperand::CreateStackIndex(slot));
}

void MInstruction::AddMemory(uint64_t vreg, uint64_t bytes) {
    AddOperand(MOperand::CreateMemory(vreg, bytes));
}

void MInstruction::AddBasicBlock(MBasicBlock* basicBlock) {
    AddOperand(MOperand::CreateBasicBlock(basicBlock));
}

bool MInstruction::HasOperands() const {
    return !m_Operands.empty();
}

size_t MInstruction::GetOperandsNumber() const {
    return m_Operands.size();
}

size_t MInstruction::GetUsesNumber() const {
    size_t opNumber = GetOperandsNumber();
    if (!IsDefinition()) {
        return opNumber;
    }
    return opNumber - 1;
}

MInstruction::TOperandIt MInstruction::GetOperand(size_t index) {
    return std::next(m_Operands.begin(), index);
}

void MInstruction::Undefine() {
    m_IsDefinition = false;
}

bool MInstruction::IsDefinition() const {
    return !IsReturn() && !IsJump() && !IsBranch() && !IsStore() &&
            !IsPush() && !IsPop() && m_IsDefinition; 
}

MInstruction::TOperandIt MInstruction::GetDefinition() {
    assert(IsDefinition());
    return GetOperand(0);
}

MInstruction::TOperandIt MInstruction::GetUse(size_t index) {
    if (IsDefinition()) {
        ++index;
    }
    return GetOperand(index);
}

MInstruction::TOperandIt MInstruction::GetOpBegin() {
    return m_Operands.begin();
}

MInstruction::TOperandIt MInstruction::GetOpEnd() {
    return m_Operands.end();
}

std::list<MOperand>& MInstruction::GetOperands() {
    return m_Operands;
}

std::list<target::Register> MInstruction::GetImplicitRegDefinitions() {
    return m_ImplicitRegDefinitions;
}

std::list<target::Register> MInstruction::GetImplicitRegUses() {
    return m_ImplicitRegUses;
}

void MInstruction::SetTargetInstructionCode(uint64_t code) {
    m_TargetInstructionCode = code;
}

bool MInstruction::HasTargetInstruction() const {
    return m_TargetInstructionCode;
}

uint64_t MInstruction::GetTargetInstructionCode() const {
    return m_TargetInstructionCode;
}

void MInstruction::SetInstructionClass(unsigned int instrClass) {
    m_InstructionClass = instrClass;
}

unsigned int MInstruction::GetInstructionClass() const {
    return m_InstructionClass;
}

}  // namespace gen
