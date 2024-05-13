#pragma once

#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>
#include <Ancl/CodeGen/Selection/SelectionGraph.hpp>

namespace gen {

class InstructionSelector {
public:
    InstructionSelector(MIRProgram* mirProgram,
                        target::TargetMachine* targetMachine)
        : m_MIRProgram(mirProgram), m_TargetMachine(targetMachine) {}

    void Select() {
        for (const auto& function : m_MIRProgram->GetFunctions()) {
            SelectionGraph graph;
            graph.Build(function.get());

            for (SelectionTree& tree : graph.GetTrees()) {
                m_TargetMachine->Select(tree);
            }

            // TODO: Generate target instructions
        }
    }

private:
    MIRProgram* m_MIRProgram;
    target::TargetMachine* m_TargetMachine;
};

}  // namespace gen

