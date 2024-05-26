#pragma once

#include <cassert>

#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <stack>

#include <Ancl/AnclIR/Constant/Function.hpp>
#include <Ancl/AnclIR/BasicBlock.hpp>

#include <Ancl/AnclIR/Instruction/AllocaInstruction.hpp>
#include <Ancl/AnclIR/Instruction/LoadInstruction.hpp>
#include <Ancl/AnclIR/Instruction/StoreInstruction.hpp>
#include <Ancl/AnclIR/Instruction/PhiInstruction.hpp>

#include <Ancl/Graph/DominatorTree.hpp>

#include <Ancl/Logger/Logger.hpp>


namespace ir {

class SSAPass {
public:
    SSAPass(Function* function)
        : m_Function(function), m_DomTree(function->GetEntryBlock()) {}

    void Run() {
        for (BasicBlock* block : m_Function->GetBasicBlocks()) {
            findBadAllocas(block);
        }

        computeAllocasInfo();

        for (size_t i = 0; i < m_PromotableAllocaList.size(); ++i) {
            tryPromoteAlloca(m_PromotableAllocaList[i], i);
        }

        for (AllocaInstruction* alloca : m_Globals) {
            insertPhiFunctions(alloca);
        }

        renameAllocas(m_Function->GetEntryBlock());
    }

private:
    void renameAllocas(BasicBlock* block) {
        std::unordered_map<AllocaInstruction*, int> stacksGrowth;

        for (PhiInstruction* phi : block->GetPhiFunctions()) {
            auto* alloca = m_PhiAllocaMap[phi];
            m_AllocaValueStacks[alloca].push(phi);
            ++stacksGrowth[alloca];
        }

        auto& instructions = block->GetInstructionsRef();
        for (auto it = instructions.begin(); it != instructions.end();) {
            auto instr = *it;

            bool isPromotableInstr = false;
            if (auto* store = dynamic_cast<StoreInstruction*>(instr)) {
                auto* toOperand = store->GetAddressOperand();
                if (auto* alloca = dynamic_cast<AllocaInstruction*>(toOperand)) {
                    if (isPromotable(alloca)) {
                        // TODO: Check LOAD
                        m_AllocaValueStacks[alloca].push(store->GetValueOperand());
                        ++stacksGrowth[alloca];
                        isPromotableInstr = true;
                    }
                }
            } else if (auto* load = dynamic_cast<LoadInstruction*>(instr)) {
                auto* ptrOperand = load->GetPtrOperand();
                if (auto* alloca = dynamic_cast<AllocaInstruction*>(ptrOperand)) {
                    if (isPromotable(alloca)) {
                        if (m_AllocaValueStacks[alloca].empty()) {
                            // TODO: Set some empty Value
                            ANCL_CRITICAL("Undefined behavior: Alloca without Store was promoted");
                        }
                        m_LoadValueMap[load] = m_AllocaValueStacks[alloca].top();
                        isPromotableInstr = true;
                    }
                }
            } else if (auto* alloca = dynamic_cast<AllocaInstruction*>(instr)) {
                if (isPromotable(alloca)) {
                    isPromotableInstr = true;
                }
            }

            std::vector<Value*> operands = instr->GetOperands();
            for (size_t i = 0; i < operands.size(); ++i) {
                if (auto* load = dynamic_cast<LoadInstruction*>(operands[i])) {
                    if (m_LoadValueMap.contains(load)) {
                        instr->SetOperand(m_LoadValueMap[load], i);
                    }
                }
            }

            if (isPromotableInstr) {
                it = instructions.erase(it);
            } else {
                ++it;
            }
        }

        for (BasicBlock* next : block->GetSuccessors()) {
            auto preds = next->GetPredecessors();
            size_t predIndex = 0;
            for (; predIndex < preds.size(); ++predIndex) {
                if (preds[predIndex] == block) {
                    break;
                }
            }

            auto phis = next->GetPhiFunctions();
            for (PhiInstruction* phi : phis) {
                AllocaInstruction* alloca = m_PhiAllocaMap[phi];
                Value* allocaValue = m_AllocaValueStacks[alloca].top();
                phi->SetIncomingValue(predIndex, allocaValue);
            }
        }

        for (BasicBlock* next : m_DomTree.GetChildren(block)) {
            renameAllocas(next);
        }

        for (auto entry : stacksGrowth) {
            AllocaInstruction* alloca = entry.first;
            int growth = entry.second;
            while (growth > 0) {
                m_AllocaValueStacks[alloca].pop();
                --growth;
            }
        }
    }

    void insertPhiFunctions(AllocaInstruction* alloca) {
        auto allocaInfo = m_PromotableAllocaInfo[alloca];
        auto workSet = allocaInfo.DefBlocks;

        std::vector<BasicBlock*> workList;
        workList.insert(workList.end(), workSet.begin(), workSet.end());

        while (!workList.empty()) {
            BasicBlock* block = std::move(workList.back());
            workList.pop_back();
            workSet.erase(block);

            for (BasicBlock* frontier : m_DomTree.GetDominanceFrontier(block)) {
                if (m_PhiBasicBlocks[alloca].contains(frontier)) {
                    continue;
                }

                addPhiFunction(alloca, frontier);

                if (!workSet.contains(frontier)) {
                    workList.push_back(frontier);
                    workSet.insert(frontier);
                }
            }
        }
    }

    void addPhiFunction(AllocaInstruction* alloca, BasicBlock* block) {
        m_PhiBasicBlocks[alloca].insert(block);

        auto allocaPtrType = dynamic_cast<PointerType*>(alloca->GetType());
        assert(allocaPtrType);

        auto& program = m_Function->GetProgram();
        auto* phiInstr = program.CreateValue<PhiInstruction>(allocaPtrType->GetSubType(), "phi", block);
        block->AddPhiFunction(phiInstr);

        auto preds = block->GetPredecessors();
        for (size_t i = 0; i < preds.size(); ++i) {
            phiInstr->SetIncomingBlock(i, preds[i]);
        }

        m_PhiAllocaMap[phiInstr] = alloca;
    }

    void tryPromoteAlloca(AllocaInstruction* alloca, size_t& index) {
        auto allocaInfo = m_PromotableAllocaInfo[alloca];

        if (allocaInfo.Loads.empty() && allocaInfo.Stores.empty()) {
            m_PromotableAllocaInfo.erase(alloca);
            removeAllocaFromList(index);
            return;
        }

        if (allocaInfo.DefBlocks.size() == 1) {
            // TODO: handle single store
        }
    }

    void removeAllocaFromList(size_t& index) {
        m_PromotableAllocaList[index] = m_PromotableAllocaList.back();
        m_PromotableAllocaList.pop_back();
        --index;
    }

    void computeAllocasInfo() {
        for (BasicBlock* block : m_Function->GetBasicBlocks()) {
            std::unordered_set<AllocaInstruction*> storedAllocas;

            auto instructions = block->GetInstructions();
            for (auto it = instructions.begin(); it != instructions.end(); ++it) {
                Instruction* instruction = *it;
                if (auto* alloca = dynamic_cast<AllocaInstruction*>(instruction)) {
                    if (!m_BadAllocas[alloca]) {
                        m_PromotableAllocaList.push_back(alloca);
                        m_PromotableAllocaInfo[alloca].Iterator = it;
                    }
                } else if (auto* load = dynamic_cast<LoadInstruction*>(instruction)) {
                    if (auto* alloca = dynamic_cast<AllocaInstruction*>(load->GetPtrOperand())) {
                        if (!m_BadAllocas[alloca]) {
                            m_PromotableAllocaInfo[alloca].Loads.push_back(load);
                            m_PromotableAllocaInfo[alloca].UseBlocks.insert(block);

                            if (!storedAllocas.contains(alloca)) {
                                m_Globals.insert(alloca);  // Alloca with upward-exposed use
                            }
                        }
                    }
                } else if (auto* store = dynamic_cast<StoreInstruction*>(instruction)) {
                    if (auto* alloca = dynamic_cast<AllocaInstruction*>(store->GetAddressOperand())) {
                        if (!m_BadAllocas[alloca]) {
                            m_PromotableAllocaInfo[alloca].Stores.push_back(store);
                            m_PromotableAllocaInfo[alloca].DefBlocks.insert(block);

                            storedAllocas.insert(alloca);
                        }
                    }
                }
            }
        }
    }

    void findBadAllocas(BasicBlock* block) {
        for (Instruction* instruction : block->GetInstructions()) {
            for (ir::Value* operand : instruction->GetOperands()) {
                if (auto* alloca = dynamic_cast<AllocaInstruction*>(operand)) {
                    if (!checkAllocaUser(instruction, alloca)) {
                        m_BadAllocas[alloca] = true;
                    }
                }
            }
        }
    }

    bool checkAllocaUser(Instruction* user, AllocaInstruction* alloca) {
        if (auto* load = dynamic_cast<LoadInstruction*>(user)) {
            if (load->IsVolatile()) {
                return false;
            }
        } else if (auto* store = dynamic_cast<StoreInstruction*>(user)) {
            if (store->GetValueOperand() == alloca) {
                return false;
            }
            if (store->IsVolatile()) {
                return false;
            }
        } else {
            return false;
        }

        return true;
    }

    bool isPromotable(AllocaInstruction* alloca) {
        return m_PromotableAllocaInfo.contains(alloca);
    }

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
