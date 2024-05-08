#pragma once

#include <unordered_map>

#include <Ancl/CodeGen/Target/Base/Instruction.hpp>


namespace gen::target {

class InstructionSet {
public:
    InstructionSet() = default;
    virtual ~InstructionSet() = default;

    virtual TargetInstruction GetInstruction(TInstrCode code) const;
};

}  // namespace target
