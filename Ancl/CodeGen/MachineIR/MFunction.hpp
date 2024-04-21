#pragma once

#include <string>
#include <vector>

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

    MFunction(const std::string& name, const std::vector<MBasicBlock>& basicBlocks)
        : m_Name(name), m_BasicBlocks(basicBlocks) {}

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

    void AddBasicBlock(MBasicBlock MBB) {
        m_BasicBlocks.push_back(MBB);
    }

    std::vector<MBasicBlock>& GetBasicBlocks() {
        return m_BasicBlocks;
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

    std::vector<MBasicBlock> m_BasicBlocks;

    uint m_NextVReg = 0;
};

}  // namespace gen
