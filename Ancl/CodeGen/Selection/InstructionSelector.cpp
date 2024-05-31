#include <Ancl/CodeGen/Selection/InstructionSelector.hpp>

#include <Ancl/CodeGen/Selection/VirtualRegClassSelector.hpp>


namespace gen {

InstructionSelector::InstructionSelector(MIRProgram& mirProgram,
                    target::TargetMachine* targetMachine)
    : m_MIRProgram(mirProgram), m_TargetMachine(targetMachine) {}

void InstructionSelector::Select() {
    for (const auto& function : m_MIRProgram.GetFunctions()) {
        SelectionGraph graph;
        graph.Build(function.get());

        for (SelectionTree& tree : graph.GetTrees()) {
            m_TargetMachine->Select(tree);
        }

        for (auto& basicBlock : function->GetBasicBlocks()) {
            basicBlock->ClearInstructions();
        }

        for (SelectionTree& tree : graph.GetTrees()) {
            std::vector<MInstruction> instructions = tree.GenerateTargetInstructions();
            for (MInstruction& instr : instructions) {
                MBasicBlock* basicBlock = instr.GetBasicBlock();
                basicBlock->AddInstruction(instr);
            }
        }
    }
}

MInstruction InstructionSelector::SelectInstruction(MInstruction& instruction,
                                                    target::TargetMachine* targetMachine) {
    for (MOperand& operand : instruction.GetOperands()) {
        VirtualRegClassSelector::SelectOperandClass(operand, targetMachine);
    }

    SelectionTree tree{instruction};
    targetMachine->Select(tree);

    MInstruction targetInstr = tree.GenerateTargetInstructions().at(0);

    return targetInstr;
}

}  // namespace gen
