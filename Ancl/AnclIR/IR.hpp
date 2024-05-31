#pragma once

#include <Ancl/AnclIR/BasicBlock.hpp>
#include <Ancl/AnclIR/Parameter.hpp>


/*
=====================================================================
                              Constant
=====================================================================
*/

#include <Ancl/AnclIR/Constant/Constant.hpp>
#include <Ancl/AnclIR/Constant/FloatConstant.hpp>
#include <Ancl/AnclIR/Constant/Function.hpp>
#include <Ancl/AnclIR/Constant/GlobalValue.hpp>
#include <Ancl/AnclIR/Constant/GlobalVariable.hpp>
#include <Ancl/AnclIR/Constant/IntConstant.hpp>


/*
=====================================================================
                             Instruction
=====================================================================
*/

#include <Ancl/AnclIR/Instruction/AllocaInstruction.hpp>
#include <Ancl/AnclIR/Instruction/BinaryInstruction.hpp>
#include <Ancl/AnclIR/Instruction/BranchInstruction.hpp>
#include <Ancl/AnclIR/Instruction/CallInstruction.hpp>
#include <Ancl/AnclIR/Instruction/CastInstruction.hpp>
#include <Ancl/AnclIR/Instruction/CompareInstruction.hpp>
#include <Ancl/AnclIR/Instruction/Instruction.hpp>
#include <Ancl/AnclIR/Instruction/LoadInstruction.hpp>
#include <Ancl/AnclIR/Instruction/MemberInstruction.hpp>
#include <Ancl/AnclIR/Instruction/MemoryCopyInstruction.hpp>
#include <Ancl/AnclIR/Instruction/MemorySetInstruction.hpp>
#include <Ancl/AnclIR/Instruction/ReturnInstruction.hpp>
#include <Ancl/AnclIR/Instruction/StoreInstruction.hpp>
#include <Ancl/AnclIR/Instruction/SwitchInstruction.hpp>
#include <Ancl/AnclIR/Instruction/TerminatorInstruction.hpp>


/*
=====================================================================
                             Type
=====================================================================
*/

#include <Ancl/AnclIR/Type/ArrayType.hpp>
#include <Ancl/AnclIR/Type/FloatType.hpp>
#include <Ancl/AnclIR/Type/FunctionType.hpp>
#include <Ancl/AnclIR/Type/IntType.hpp>
#include <Ancl/AnclIR/Type/LabelType.hpp>
#include <Ancl/AnclIR/Type/PointerType.hpp>
#include <Ancl/AnclIR/Type/StructType.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>
#include <Ancl/AnclIR/Type/VoidType.hpp>
