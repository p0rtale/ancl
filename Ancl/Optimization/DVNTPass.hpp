#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <Ancl/AnclIR/Constexpr.hpp>
#include <Ancl/AnclIR/IR.hpp>
#include <Ancl/Graph/DominatorTree.hpp>


namespace ir {

/*
    Dominator-Based Value Numbering Technique
*/
class DVNTPass {
public:
    DVNTPass(Function* function);

    void Run();

private:
    using triplet = std::tuple<Value*, std::string, Value*>;

    void runPreorderDVNT(BasicBlock* basicBlock);

    bool handleReducible(Instruction* instruction);

    void allocateScope();
    void freeScope();

    void addDefinition(const triplet& key, Instruction* instruction);

    std::optional<Value*> getDefinition(const triplet& key) ;

    bool isMeaninglessConstPhi(PhiInstruction* phiInstr, const IntValue& intValue);
    bool isMeaninglessPhi(PhiInstruction* phiInstr);

    bool isNumberConstant(Value* value) const;

    bool isReducible(Instruction* instruction);

    triplet constructKey(Value* firstValue, Value* secondValue,
                         const std::string& opString);

    std::string getReducibleInstructionName(Instruction* instruction);

private:
    Function* m_Function = nullptr;
    DominatorTree m_DomTree;

    Constexpr m_Constexpr;

    std::vector<std::unordered_map<triplet, Value*>> m_ExprDefinitions;

    std::unordered_map<std::string, Value*> m_AssignedDefinitions;
};

}  // namespace ir
