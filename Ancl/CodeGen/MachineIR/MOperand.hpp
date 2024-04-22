#pragma once

#include <string>

#include <Ancl/CodeGen/MachineIR/MType.hpp>


namespace gen {

class MBasicBlock;

class MOperand {
public:
    enum class Kind {
        kNone,

        kImmInteger, kImmFloat,

        kRegister, kParameter,

        kGlobalSymbol,
        kMBasicBlock,
        kFunction,

        kStackIndex,
        kMemory,
    };

public:
    MOperand(Kind kind): m_Kind(kind) {}

    static MOperand CreateImmInteger(int64_t value, uint bytes = 8) {
        auto operand = MOperand{Kind::kImmInteger};
        operand.SetImmInteger(value);
        operand.SetType(MType::CreateScalar(bytes));
        return operand;
    }

    static MOperand CreateImmFloat(double value, uint bytes = 8) {
        auto operand = MOperand{Kind::kImmFloat};
        operand.SetImmFloat(value);
        operand.SetType(MType::CreateScalar(bytes));
        return operand;
    }

    static MOperand CreateRegister(uint regNumber, uint bytes = 8, bool isScalar = true,
                                   bool isVirtual = true) {
        auto operand = MOperand{Kind::kRegister};
        operand.SetVRegister(regNumber);
        operand.SetVirtual(isVirtual);

        if (isScalar) {
            operand.SetType(MType::CreateScalar(bytes));
        } else {
            operand.SetType(MType::CreatePointer(bytes));
        }
        
        return operand;
    }

    static MOperand CreateParameter(uint vreg) {
        auto operand = MOperand{Kind::kParameter};
        operand.SetVRegister(vreg);
        return operand;
    }

    static MOperand CreateGlobalSymbol(const std::string& symbol) {
        auto operand = MOperand{Kind::kGlobalSymbol};
        operand.SetGlobalSymbol(symbol);
        return operand;
    }

    static MOperand CreateBasicBlock(MBasicBlock* MBB) {
        auto operand = MOperand{Kind::kMBasicBlock};
        operand.SetMBasicBlock(MBB);
        return operand;
    }

    static MOperand CreateFunction(const std::string& symbol) {
        auto operand = MOperand{Kind::kFunction};
        operand.SetGlobalSymbol(symbol);
        return operand;
    }

    static MOperand CreateStackIndex(uint index, int64_t offset = 0) {
        auto operand = MOperand{Kind::kStackIndex};
        operand.SetIndex(index);
        operand.SetOffset(offset);
        return operand;
    }

    static MOperand CreateMemory(uint vreg, uint bytes = 8, int64_t offset = 0) {
        auto operand = MOperand{Kind::kMemory};
        operand.SetVRegister(vreg);
        operand.SetOffset(offset);
        operand.SetType(MType::CreatePointer(bytes));
        return operand;
    }

    Kind GetKind() const {
        return m_Kind;
    }

    bool IsImmediate() const {
        return m_Kind == Kind::kImmInteger || m_Kind == Kind::kImmFloat;
    }

    bool IsImmInteger() const {
        return m_Kind == Kind::kImmInteger;
    }

    bool IsImmFloat() const {
        return m_Kind == Kind::kImmFloat;
    }

    bool IsRegister() const {
        return m_Kind == Kind::kRegister;
    }

    bool IsPRegister() const {
        return m_Kind == Kind::kRegister && !m_IsVirtual;
    }

    bool IsVRegister() const {
        return m_Kind == Kind::kRegister && m_IsVirtual;
    }

    bool IsGlobalSymbol() const {
        return m_Kind == Kind::kGlobalSymbol;
    }

    bool IsMBasicBlock() const {
        return m_Kind == Kind::kMBasicBlock;
    }

    bool IsFunction() const {
        return m_Kind == Kind::kFunction;
    }

    bool IsStackIndex() const {
        return m_Kind == Kind::kStackIndex;
    }

    bool IsMemory() const {
        return m_Kind == Kind::kMemory;
    }

    void SetType(MType type) {
        m_Type = type;
    }

    MType GetType() const {
        return m_Type;
    }

    MType& GetTypeRef() {
        return m_Type;
    }

    void SetImmInteger(int64_t value) {
        m_Data.ImmInt = value;
    }

    int64_t GetImmInteger() const {
        return m_Data.ImmInt;
    }

    void SetImmFloat(double value) {
        m_Data.ImmFloat = value;
    }

    double GetImmFloat() const {
        return m_Data.ImmFloat;
    }

    void SetVRegister(uint vreg) {
        m_Data.VRegister = vreg;
    }

    uint GetVRegister() const {
        return m_Data.VRegister;
    }

    void SetGlobalSymbol(const std::string& symbol) {
        m_GlobalSymbol = symbol;
    }

    std::string GetGlobalSymbol() const {
        return m_GlobalSymbol;
    }

    std::string GetFunctionSymbol() const {
        return GetGlobalSymbol();
    }

    void SetMBasicBlock(MBasicBlock* MBB) {
        m_Data.MBB = MBB;
    }

    MBasicBlock* GetBasicBlock() const {
        return m_Data.MBB;
    }

    void SetOffset(int64_t offset) {
        m_Offset = offset;
    }

    int64_t GetOffset() {
        return m_Offset;
    }

    void SetIndex(int index) {
        m_Index = index;
    }

    int GetIndex() const {
        return m_Index;
    }

    void SetVirtual(bool isVirtual) {
        m_IsVirtual = isVirtual;
    }
    
    bool IsVirtual() const {
        return m_IsVirtual;
    }

private:
    Kind m_Kind = Kind::kNone;

    MType m_Type;

    union DataUnion {
        int64_t ImmInt;
        double ImmFloat;  // TODO: FloatValue?
        uint VRegister;  // TODO: Register class
        MBasicBlock* MBB;
    } m_Data;

    bool m_IsVirtual = false;
    std::string m_GlobalSymbol;  // TODO: add to union
    int64_t m_Offset = 0;  // TODO: add to union
    int m_Index = 0;
};

}  // namespace gen
