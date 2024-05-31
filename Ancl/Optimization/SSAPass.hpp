#pragma once

#include <list>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Ancl/AnclIR/IR.hpp>
#include <Ancl/Graph/DominatorTree.hpp>


namespace ir {

class SSAPass {
public:
    SSAPass(Function* function);

    void Run();

private:
    void renameAllocas(BasicBlock* block);

    void insertPhiFunctions(AllocaInstruction* alloca);

    void addPhiFunction(AllocaInstruction* alloca, BasicBlock* block);

    void tryPromoteAlloca(AllocaInstruction* alloca, size_t& index);

    void removeAllocaFromList(size_t& index);

    void computeAllocasInfo();

    void findBadAllocas(BasicBlock* block);

    bool checkAllocaUser(Instruction* user, AllocaInstruction* alloca);

    bool isPromotable(AllocaInstruction* alloca);

private:
    Function* m_Function = nullptr;
    DominatorTree m_DomTree;

    std::unordered_map<AllocaInstruction*, bool> m_BadAllocas;

    std::vector<AllocaInstruction*> m_PromotableAllocaList;

    std::unordered_set<AllocaInstruction*> m_Globals;
    std::unordered_map<AllocaInstruction*, std::unordered_set<BasicBlock*>> m_PhiBasicBlocks;

    struct AllocaInfo {
        std::list<Instruction*>::iterator Iterator;

        std::vector<LoadInstruction*> Loads;
        std::vector<StoreInstruction*> Stores;

        std::unordered_set<BasicBlock*> DefBlocks;  // Store
        std::unordered_set<BasicBlock*> UseBlocks;  // Load

        bool IsOneBlock = true;
        BasicBlock* Block = nullptr;
    };
    std::unordered_map<AllocaInstruction*, AllocaInfo> m_PromotableAllocaInfo;

    std::unordered_map<PhiInstruction*, AllocaInstruction*> m_PhiAllocaMap;
    std::unordered_map<LoadInstruction*, Value*> m_LoadValueMap;
    std::unordered_map<AllocaInstruction*, std::stack<Value*>> m_AllocaValueStacks;
};

}  // namespace ir
