#pragma once

#include <cassert>
#include <list>

#include <Ancl/CodeGen/MachineIR/MOperand.hpp>
#include <Ancl/CodeGen/Target/Base/Register.hpp>


namespace gen {

class MBasicBlock;

class MInstruction {
public:
    enum class OpType {
        kNone = 0,
        kMul, kFMul,
        kSDiv, kUDiv, kFDiv,
        kSRem, kURem,
        kAdd, kFAdd,
        kSub, kFSub,
        kShiftL, kLShiftR, kAShiftR,
        kAnd, kXor, kOr,

        kCmp, kUCmp, kFCmp,

        kITrunc, kFTrunc,
        kZExt, kSExt, kFExt,
        kFToUI, kFToSI,
        kUIToF, kSIToF,

        kCall, kJump, kBranch, kRet,

        kMov, kFMov,

        kLoad, kStore,
        kStackAddress, kGlobalAddress,
        kMemberAddress,

        kPush, kPop,

        kPhi,
        kSubregToReg,
        kRegToSubreg,
    };

    enum class CompareKind {
        kNone = 0,
        kEqual, kNEqual,
        kGreater, kLess,
        kGreaterEq, kLessEq,
    };

public:
    MInstruction() = default;

    MInstruction(OpType opType): m_OpType(opType) {}

    MInstruction(OpType opType, CompareKind compareKind)
        : m_OpType(opType), m_CompareKind(compareKind) {}

    OpType GetOpType() const {
        return m_OpType;
    }

    std::string GetOpTypeString() const {
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

    bool IsCall() const {
        return m_OpType == OpType::kCall;
    }

    bool IsReturn() const {
        return m_OpType == OpType::kRet;
    }

    bool IsJump() const {
        return m_OpType == OpType::kJump;
    }

    bool IsBranch() const {
        return m_OpType == OpType::kBranch;
    }

    bool IsTerminator() const {
        return IsJump() || IsBranch() || IsReturn();
    }

    bool IsStore() const {
        return m_OpType == OpType::kStore;
    }

    bool IsLoad() const {
        return m_OpType == OpType::kLoad;
    }

    bool IsStackAddress() const {
        return m_OpType == OpType::kStackAddress;
    }

    bool IsGlobalAddress() const {
        return m_OpType == OpType::kGlobalAddress;
    }

    bool IsMemberAddress() const {
        return m_OpType == OpType::kMemberAddress;
    }

    bool IsCmp() const {
        return m_OpType == OpType::kCmp;
    }

    bool IsMov() const {
        return m_OpType == OpType::kMov;
    }

    // TODO: Const
    bool IsRegMov() {
        if (!IsMov()) {
            return false;
        }

        TOperandIt toOperand = GetDefinition();
        TOperandIt fromOperand = GetUse(0);

        return toOperand->IsRegister() && fromOperand->IsRegister();
    }

    bool IsSubregToReg() const {
        return m_OpType == OpType::kSubregToReg;
    }

    bool IsRegToSubreg() const {
        return m_OpType == OpType::kRegToSubreg;
    }

    bool IsPush() const {
        return m_OpType == OpType::kPush;
    }

    bool IsPop() const {
        return m_OpType == OpType::kPop;
    }

    bool IsPhi() const {
        return m_OpType == OpType::kPhi;
    }

    CompareKind GetCompareKind() const {
        return m_CompareKind;
    }

    bool HasBasicBlock() const {
        return m_BasicBlock;
    }

    void SetBasicBlock(MBasicBlock* MBB) {
        m_BasicBlock = MBB;
    }

    MBasicBlock* GetBasicBlock() const {
        return m_BasicBlock;
    }

    void AddOperand(MOperand operand) {
        m_Operands.push_back(operand);
    }

    void AddOperandToBegin(MOperand operand) {
        m_Operands.push_front(operand);
    }

    void RemoveFirstOperand() {
        m_Operands.pop_front();
    }

    void AddImplicitRegDefinition(target::Register reg) {
        m_ImplicitRegDefinitions.push_back(reg);
    }

    void AddImplicitRegUse(target::Register reg) {
        m_ImplicitRegUses.push_back(reg);
    }

    void AddPhysicalRegister(target::Register reg) {
        MType type;
        if (reg.IsFloat()) {
            type = MType(MType::Kind::kFloat, reg.GetBytes());
        } else {
            type = MType(MType::Kind::kInteger, reg.GetBytes());
        }
        AddOperand(MOperand::CreateRegister(reg.GetNumber(), type, /*isVirtual=*/false));
    }

    void AddVirtualRegister(uint regNumber, MType type) {
        AddOperand(MOperand::CreateRegister(regNumber, type, /*isVirtual=*/true));
    }

    void AddImmInteger(int64_t value, uint bytes = 8) {
        AddOperand(MOperand::CreateImmInteger(value, bytes));
    }  

    void AddImmFloat(double value, uint bytes = 8) {
        AddOperand(MOperand::CreateImmFloat(value, bytes));
    }  

    void AddGlobalSymbol(const std::string& symbol) {
        AddOperand(MOperand::CreateGlobalSymbol(symbol));
    }

    void AddFunction(const std::string& symbol) {
        AddOperand(MOperand::CreateFunction(symbol));
    }

    void AddStackIndex(uint slot, int64_t offset = 0) {
        AddOperand(MOperand::CreateStackIndex(slot));
    }

    void AddMemory(uint vreg, uint bytes = 8) {
        AddOperand(MOperand::CreateMemory(vreg, bytes));
    }

    void AddBasicBlock(MBasicBlock* basicBlock) {
        AddOperand(MOperand::CreateBasicBlock(basicBlock));
    }

    bool HasOperands() const {
        return !m_Operands.empty();
    }

    size_t GetOperandsNumber() const {
        return m_Operands.size();
    }

    size_t GetUsesNumber() const {
        size_t opNumber = GetOperandsNumber();
        if (!IsDefinition()) {
            return opNumber;
        }
        return opNumber - 1;
    }

    using TOperandIt = std::list<MOperand>::iterator;

    TOperandIt GetOperand(size_t index) {
        return std::next(m_Operands.begin(), index);
    }

    // ADD defReg, useReg -> ADD mem, useReg
    // IDIV
    void Undefine() {
        m_IsDefinition = false;
    }

    bool IsDefinition() const {
        return !IsReturn() && !IsJump() && !IsBranch() && !IsStore() &&
               !IsPush() && !IsPop() && m_IsDefinition; 
    }

    TOperandIt GetDefinition() {
        assert(IsDefinition());
        return GetOperand(0);
    }

    TOperandIt GetUse(size_t index) {
        if (IsDefinition()) {
            ++index;
        }
        return GetOperand(index);
    }

    TOperandIt GetOpBegin() {
        return m_Operands.begin();
    }

    TOperandIt GetOpEnd() {
        return m_Operands.end();
    }

    std::list<MOperand>& GetOperands() {
        return m_Operands;
    }

    std::list<target::Register> GetImplicitRegDefinitions() {
        return m_ImplicitRegDefinitions;
    }

    std::list<target::Register> GetImplicitRegUses() {
        return m_ImplicitRegUses;
    }

    void SetTargetInstructionCode(uint code) {
        m_TargetInstructionCode = code;
    }

    bool HasTargetInstruction() const {
        return m_TargetInstructionCode;
    }

    uint GetTargetInstructionCode() const {
        return m_TargetInstructionCode;
    }

    void SetInstructionClass(uint instrClass) {
        m_InstructionClass = instrClass;
    }

    uint GetInstructionClass() const {
        return m_InstructionClass;
    }

private:
    OpType m_OpType = OpType::kNone;
    CompareKind m_CompareKind = CompareKind::kNone;

    // TODO: Use something with random-access and handle iterator/pointer invalidation
    std::list<MOperand> m_Operands;

    std::list<target::Register> m_ImplicitRegDefinitions;
    std::list<target::Register> m_ImplicitRegUses;

    uint m_TargetInstructionCode = 0;
    uint m_InstructionClass = 0;

    MBasicBlock* m_BasicBlock = nullptr;

    bool m_IsDefinition = true;
};

}  // namespace gen
