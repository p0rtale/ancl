#pragma once

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

    MInstruction(OpType opType);

    MInstruction(OpType opType, CompareKind compareKind);

    OpType GetOpType() const;
    std::string GetOpTypeString() const;

    bool IsCall() const;
    bool IsReturn() const;
    bool IsJump() const;
    bool IsBranch() const;
    bool IsTerminator() const;
    bool IsStore() const;
    bool IsLoad() const;
    bool IsStackAddress() const;
    bool IsGlobalAddress() const;
    bool IsMemberAddress() const;
    bool IsCmp() const;
    bool IsMov() const;

    // TODO: Const
    bool IsRegMov();

    bool IsSubregToReg() const;
    bool IsRegToSubreg() const;
    bool IsPush() const;
    bool IsPop() const;
    bool IsPhi() const;

    CompareKind GetCompareKind() const;

    bool HasBasicBlock() const;

    void SetBasicBlock(MBasicBlock* MBB);

    MBasicBlock* GetBasicBlock() const;

    void AddOperand(MOperand operand);
    void AddOperandToBegin(MOperand operand);

    void RemoveFirstOperand();

    void AddImplicitRegDefinition(target::Register reg);
    void AddImplicitRegUse(target::Register reg);

    void AddPhysicalRegister(target::Register reg);
    void AddVirtualRegister(uint64_t regNumber, MType type);
    void AddImmInteger(int64_t value, uint64_t bytes = 8);
    void AddImmFloat(double value, uint64_t bytes = 8);
    void AddGlobalSymbol(const std::string& symbol);
    void AddFunction(const std::string& symbol);
    void AddStackIndex(uint64_t slot, int64_t offset = 0);
    void AddMemory(uint64_t vreg, uint64_t bytes = 8);
    void AddBasicBlock(MBasicBlock* basicBlock);

    bool HasOperands() const;

    size_t GetOperandsNumber() const;

    size_t GetUsesNumber() const;


    using TOperandIt = std::list<MOperand>::iterator;

    TOperandIt GetOperand(size_t index);

    // ADD defReg, useReg -> ADD mem, useReg
    // IDIV
    void Undefine();

    bool IsDefinition() const;

    TOperandIt GetDefinition();

    TOperandIt GetUse(size_t index);

    TOperandIt GetOpBegin();
    TOperandIt GetOpEnd();

    std::list<MOperand>& GetOperands();

    std::list<target::Register> GetImplicitRegDefinitions();
    std::list<target::Register> GetImplicitRegUses();

    void SetTargetInstructionCode(uint64_t code);
    bool HasTargetInstruction() const;
    uint64_t GetTargetInstructionCode() const;

    void SetInstructionClass(unsigned int instrClass);
    unsigned int GetInstructionClass() const;

private:
    OpType m_OpType = OpType::kNone;
    CompareKind m_CompareKind = CompareKind::kNone;

    // TODO: Use something with random-access and handle iterator/pointer invalidation
    std::list<MOperand> m_Operands;

    std::list<target::Register> m_ImplicitRegDefinitions;
    std::list<target::Register> m_ImplicitRegUses;

    uint64_t m_TargetInstructionCode = 0;
    unsigned int m_InstructionClass = 0;

    MBasicBlock* m_BasicBlock = nullptr;

    bool m_IsDefinition = true;
};

}  // namespace gen
