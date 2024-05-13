#include <Ancl/AnclIR/Instruction/PhiInstruction.hpp>

#include <Ancl/AnclIR/BasicBlock.hpp>

using namespace ir;

PhiInstruction::PhiInstruction(Type* type, const std::string& name, BasicBlock* basicBlock)
        : Instruction(type, basicBlock),
          m_Arguments(basicBlock->GetPredecessorsNumber()) {
    SetName(name);
    // TODO: Uses?
}
