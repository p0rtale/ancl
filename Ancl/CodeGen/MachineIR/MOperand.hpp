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

        kRegister,

        kGlobalSymbol,
        kMBasicBlock,
        kFunction,

        kStackIndex,
        kMemory,
    };

public:
    MOperand(Kind kind): m_Kind(kind) {}

    static MOperand CreateImmInteger(int64_t value, uint64_t bytes = 8) {
        auto operand = MOperand{Kind::kImmInteger};
        operand.SetImmInteger(value);
        operand.SetType(MType::CreateScalar(bytes, /*isFloat=*/false));
        return operand;
    }

    static MOperand CreateImmFloat(double value, uint64_t bytes = 8) {
        auto operand = MOperand{Kind::kImmFloat};
        operand.SetImmFloat(value);
        operand.SetType(MType::CreateScalar(bytes, /*isFloat=*/true));
        return operand;
    }

    static MOperand CreateRegister(uint64_t vreg, MType type, bool isVirtual = true) {
        auto operand = MOperand{Kind::kRegister};
        operand.SetRegister(vreg);
        operand.SetVirtual(isVirtual);
        operand.SetType(type);
        return operand;
    }

    static MOperand CreateGlobalSymbol(const std::string& symbol) {
        auto operand = MOperand{Kind::kGlobalSymbol};
        operand.SetGlobalSymbol(symbol);
        return operand;
    }

    static MOperand CreateBasicBlock(MBasicBlock* MBB) {
        auto operand = MOperand{Kind::kMBasicBlock};
        operand.SetBasicBlock(MBB);
        return operand;
    }

    static MOperand CreateFunction(const std::string& symbol) {
        auto operand = MOperand{Kind::kFunction};
        operand.SetGlobalSymbol(symbol);
        return operand;
    }

    static MOperand CreateStackIndex(uint64_t index) {
        auto operand = MOperand{Kind::kStackIndex};
        operand.SetIndex(index);
        return operand;
    }

    static MOperand CreateMemory(uint64_t vreg, uint64_t bytes = 8) {
        auto operand = MOperand{Kind::kMemory};
        operand.SetRegister(vreg);
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
        return IsRegister() && !m_IsVirtual;
    }

    bool IsVRegister() const {
        return IsRegister() && m_IsVirtual;
    }

    bool IsValidVRegister() const {
        return IsRegister() && m_IsVirtual && GetRegister();
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

    void SetRegister(uint64_t vreg) {
        m_Data.Register = vreg;
    }

    bool IsInvalidRegister() const {
        return !m_Data.Register;
    }

    uint64_t GetRegister() const {
        return m_Data.Register;
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

    void SetBasicBlock(MBasicBlock* MBB) {
        m_Data.MBB = MBB;
    }

    bool HasBasicBlock() {
        return m_Data.MBB;
    }

    MBasicBlock* GetBasicBlock() const {
        return m_Data.MBB;
    }

    void SetIndex(uint64_t index) {
        m_Index = index;
    }

    uint64_t GetIndex() const {
        return m_Index;
    }

    void SetVirtual(bool isVirtual) {
        m_IsVirtual = isVirtual;
    }
    
    bool IsVirtual() const {
        return m_IsVirtual;
    }

    void SetRegisterClass(unsigned int regClass) {
        m_RegisterClass = regClass;
    }

    unsigned int GetRegisterClass() const {
        return m_RegisterClass;
    }

private:
    Kind m_Kind = Kind::kNone;

    MType m_Type;

    union DataUnion {
        int64_t ImmInt;
        double ImmFloat;  // TODO: FloatValue?
        uint64_t Register;
        MBasicBlock* MBB;
    } m_Data;

    bool m_IsVirtual = false;
    unsigned int m_RegisterClass = 0;

    std::string m_GlobalSymbol;  // TODO: add to union
    uint64_t m_Index = 0;
};

}  // namespace gen
