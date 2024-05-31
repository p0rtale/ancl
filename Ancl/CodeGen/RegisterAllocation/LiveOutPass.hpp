#pragma once

#include <unordered_map>
#include <unordered_set>

#include <Ancl/CodeGen/MachineIR/MFunction.hpp>
#include <Ancl/CodeGen/MachineIR/MBasicBlock.hpp>


namespace gen {

/*
    This pass сalculates the LiveOUT sets for the SSA form.
    It can be used for global copy coalescing.
*/
class LiveOUTPass {
public:
    LiveOUTPass(MFunction& function);

    void Run();

    // TODO: const
    std::unordered_set<uint64_t> GetLiveOUT(MBasicBlock* basicBlock);

private:
    void initSets();

    void preorderPass(MBasicBlock* basicBlock);

private:
    MFunction& m_Function;

    std::unordered_set<MBasicBlock*> m_Visited;
    bool m_IsChanged = true;

    using TSet = std::unordered_set<uint64_t>;
    std::unordered_map<MBasicBlock*, TSet> m_BlocksUEVar;
    std::unordered_map<MBasicBlock*, TSet> m_BlocksVarKill;
    std::unordered_map<MBasicBlock*, TSet> m_BlocksLiveOUT;
};

}  // namespace gen
