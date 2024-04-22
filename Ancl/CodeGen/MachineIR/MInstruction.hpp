#pragma once

#include <cassert>
#include <vector>

#include <Ancl/CodeGen/MachineIR/MOperand.hpp>
#include <Ancl/CodeGen/MachineIR/MBasicBlock.hpp>


namespace gen {

class MBasicBlock;

class MInstruction {
public:
    enum class OpType {
        kNone = 0,
        kMul, kFMul,
        kSDiv, kUDiv, kFDiv,
        kSRem, kURem, kFRem,
        kAdd, kFAdd,
        kSub, kFSub,
        kShiftL, kLShiftR, kAShiftR,
        kAnd, kXor, kOr,

        kCmp, kUCmp, kFCmp,

        kITrunc, kFTrunc,
        kZExt, kSExt, kFExt,
        kFToUI, kFToSI,
        kUIToF, kSIToF,
        kPtrToI, kIToPtr,

        kCall, kJump, kBranch, kRet,

        kImmLoad, kMov, kFMov,

        kLoad, kStore,
        kStackAddress, kGlobalAddress,

        kSextLoad, kZextLoad,
    };

    enum class CompareKind {
        kNone = 0,
        kEqual, kNEqual,
        kGreater, kLess,
        kGreaterEq, kLessEq,
    };

public:
    MInstruction(OpType opType, MBasicBlock* MBB)
        : m_OpType(opType), m_BasicBlock(MBB) {}

    MInstruction(OpType opType, CompareKind compareKind, MBasicBlock* MBB)
        : m_OpType(opType), m_CompareKind(compareKind), m_BasicBlock(MBB) {}

    OpType GetOpType() const {
        return m_OpType;
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

    bool IsStore() const {
        return m_OpType == OpType::kStore;
    }

    bool IsLoad() const {
        return m_OpType == OpType::kLoad || m_OpType == OpType::kSextLoad ||
               m_OpType == OpType::kZextLoad;
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

    void AddRegister(uint regNumber, uint bytes = 8, bool isScalar = true, bool isVirtual = true) {
        AddOperand(MOperand::CreateRegister(regNumber, bytes, isScalar, isVirtual));
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
        AddOperand(MOperand::CreateStackIndex(slot, offset));
    }

    void AddMemory(uint vreg, uint bytes = 8, int64_t offset = 0) {
        AddOperand(MOperand::CreateMemory(vreg, bytes, offset));
    }

    void AddBasicBlock(MBasicBlock* basicBlock) {
        AddOperand(MOperand::CreateBasicBlock(basicBlock));
    }

    size_t GetOperandsNumber() const {
        return m_Operands.size();
    }

    MOperand& GetOperand(size_t index) {
        // TODO: fix reference invalidation
        return m_Operands.at(index);
    }

    bool IsDefinition() const {
        return !IsReturn() && !IsJump() && !IsBranch() && !IsStore(); 
    }

    MOperand& GetDefinition() {
        assert(IsDefinition());
        return GetOperand(0);
    }

    MOperand& GetUse(size_t index) {
        if (IsDefinition()) {
            ++index;
        }
        return GetOperand(index);
    } 

    std::vector<MOperand>& GetOperands() {
        return m_Operands;
    }

private:
    OpType m_OpType = OpType::kNone;
    CompareKind m_CompareKind = CompareKind::kNone;

    std::vector<MOperand> m_Operands;

    MBasicBlock* m_BasicBlock = nullptr;
};

}  // namespace gen
