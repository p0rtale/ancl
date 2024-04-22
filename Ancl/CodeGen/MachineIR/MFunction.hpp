#pragma once

#include <string>
#include <vector>

#include <Ancl/Base.hpp>

#include <Ancl/CodeGen/MachineIR/MBasicBlock.hpp>
#include <Ancl/CodeGen/MachineIR/LocalDataArea.hpp>
#include <Ancl/CodeGen/MachineIR/MType.hpp>


namespace gen {

class MFunction {
public:
    struct Parameter {
        uint VReg;
        MType Type;
        bool IsFloat;
        bool IsStructPtr;
    };

public:
    MFunction(const std::string& name): m_Name(name) {}

    std::string GetName() const {
        return m_Name;
    }

    std::vector<Parameter> GetParameters() const {
        return m_Parameters;
    }

    uint AddParameter(MType type, bool isFloat = false, bool isStructPtr = false) {
        uint paramVReg = NextVReg();
        m_Parameters.emplace_back(paramVReg, type, isFloat, isStructPtr);
        return paramVReg;
    }

    void AddBasicBlock(TScopePtr<MBasicBlock> MBB) {
        m_BasicBlocks.push_back(std::move(MBB));
    }

    MBasicBlock* GetBasicBlock(size_t idx) {
        return m_BasicBlocks[idx].get();
    }

    MBasicBlock* GetLastBasicBlock() {
        return m_BasicBlocks.back().get();
    }

    LocalDataArea& GetLocalDataArea() {
        return m_LocalDataArea;
    }

    bool HasSlot(uint vreg) const {
        return m_LocalDataArea.HasSlot(vreg);
    }

    void AddLocalData(uint vreg, uint size, uint align) {
        m_LocalDataArea.AddSlot(vreg, size, align);
    }

    uint NextVReg() {
        return m_NextVReg++;
    }

private:
    std::string m_Name;

    std::vector<Parameter> m_Parameters;
    LocalDataArea m_LocalDataArea;

    std::vector<TScopePtr<MBasicBlock>> m_BasicBlocks;

    uint m_NextVReg = 0;
};

}  // namespace gen
