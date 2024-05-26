#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <Ancl/AnclIR/IRProgram.hpp>
#include <Ancl/DataLayout/Alignment.hpp>
#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>
#include <Ancl/CodeGen/MachineIR/MBasicBlock.hpp>


namespace gen {

class MIRGenerator {
public:
    MIRGenerator(ir::IRProgram& irProgram, target::TargetMachine* targetMachine)
        : m_IRProgram(irProgram), m_TargetMachine(targetMachine) {}

    void Generate(MIRProgram& machineProgram) {
        for (ir::Function* irFunction : m_IRProgram.GetFunctions()) {
            TScopePtr<MFunction> mirFunction = genMFromIRFunction(irFunction);
            if (mirFunction) {
                machineProgram.AddFunction(std::move(mirFunction));
            }
        }

        for (ir::GlobalVariable* globalVar : m_IRProgram.GetGlobalVars()) {
            gen::GlobalDataArea globalData = genGlobalDataArea(globalVar);
            if (globalVar->GetLinkage() == ir::GlobalValue::LinkageType::kStatic) {
                globalData.SetLocal();
            }
            if (globalVar->IsConst()) {
                globalData.SetConst();
            }
            machineProgram.AddGlobalDataArea(globalData);
        }
    }

private:
    void linkIRValueWithVReg(ir::Value* value, uint vreg) {
        // NB: Unnamed values must be assigned numbers in the AnclIR
        // TODO: Separate renumber pass?
        m_IRValueToVReg[value->GetName()] = vreg;
    }

    bool hasIRValueVReg(ir::Value* value) {
        return m_IRValueToVReg.contains(value->GetName());
    }

    uint getIRValueVReg(ir::Value* value) {
        return m_IRValueToVReg.at(value->GetName());
    }

private:
    MOperand genMVRegisterFromIRGlobalValueUse(ir::GlobalValue* irGlobalValue, MBasicBlock* mirBasicBlock) {
        MFunction* mirFunction = mirBasicBlock->GetFunction();

        MInstruction globalAddressInstr{MInstruction::OpType::kGlobalAddress};
        globalAddressInstr.AddVirtualRegister(mirFunction->NextVReg(),
                                              MType::CreatePointer(m_TargetMachine->GetPointerByteSize()));

        if (dynamic_cast<ir::Function*>(irGlobalValue)) {
            globalAddressInstr.AddFunction(irGlobalValue->GetName());
        } else {
            globalAddressInstr.AddGlobalSymbol(irGlobalValue->GetName());
        }

        mirBasicBlock->AddInstruction(globalAddressInstr);

        return *globalAddressInstr.GetDefinition();
    }

    MOperand genMVRegisterFromIRConstant(ir::Value* irConstant) {
        uint64_t constantSize = ir::Alignment::GetTypeSize(irConstant->GetType());

        if (auto* irIntConstant = dynamic_cast<ir::IntConstant*>(irConstant)) {
            IntValue intValue = irIntConstant->GetValue();
            if (intValue.IsSigned()) {
                return MOperand::CreateImmInteger(intValue.GetSignedValue(), constantSize);
            }
            return MOperand::CreateImmInteger(intValue.GetUnsignedValue(), constantSize);
        }

        if (auto* irFloatConstant = dynamic_cast<ir::FloatConstant*>(irConstant)) {
            FloatValue floatValue = irFloatConstant->GetValue();
            MOperand mirOperand = MOperand::CreateImmFloat(floatValue.GetValue(), constantSize);
            return mirOperand;      
        }

        assert(false);
    }

    MOperand genMVRegisterFromIRParameterUse(ir::Parameter* irParameter, MBasicBlock* mirBasicBlock) {
        // TODO: handle spilled parameters

        assert(hasIRValueVReg(irParameter));

        ir::Type* paramType = irParameter->GetType();
        MType mirType;
        if (dynamic_cast<ir::PointerType*>(paramType)) {
            mirType = MType::CreatePointer(m_TargetMachine->GetPointerByteSize());
        } else {
            bool isFloat = dynamic_cast<ir::FloatType*>(paramType);
            mirType = MType::CreateScalar(ir::Alignment::GetTypeSize(paramType), isFloat);
        }

        return MOperand::CreateRegister(getIRValueVReg(irParameter), mirType);
    }

    MOperand genMVRegisterFromIRInstr(ir::Instruction* irInstruction, MBasicBlock* mirBasicBlock) {
        MFunction* mirFunction = mirBasicBlock->GetFunction();
        uint vreg = 0;

        // TODO: spilled return values?

        if (!hasIRValueVReg(irInstruction)) {
            vreg = mirFunction->NextVReg();
            linkIRValueWithVReg(irInstruction, vreg);
        } else {
            vreg = getIRValueVReg(irInstruction);
            // TODO: Use separate indexing for stack slots?
        }

        ir::Type* instrType = irInstruction->GetType();
        MType mirType;
        if (dynamic_cast<ir::PointerType*>(instrType)) {
            mirType = MType::CreatePointer(m_TargetMachine->GetPointerByteSize());
        } else {
            bool isFloat = dynamic_cast<ir::FloatType*>(instrType);
            mirType = MType::CreateScalar(ir::Alignment::GetTypeSize(instrType), isFloat);
        }

        return MOperand::CreateRegister(vreg, mirType);
    }

    MOperand genMVRegisterFromIRAllocaUse(ir::AllocaInstruction* irAlloca, MBasicBlock* mirBasicBlock) {
        MFunction* mirFunction = mirBasicBlock->GetFunction();

        MInstruction stackAddress{MInstruction::OpType::kStackAddress};

        uint vreg = mirFunction->NextVReg();
        stackAddress.AddVirtualRegister(vreg, MType::CreatePointer(m_TargetMachine->GetPointerByteSize()));
        stackAddress.AddStackIndex(getIRValueVReg(irAlloca));
    
        mirBasicBlock->AddInstruction(stackAddress);

        return *stackAddress.GetDefinition();
    }

    MOperand genMVRegisterFromIRValue(ir::Value* irValue, MBasicBlock* mirBasicBlock) {
        if (auto* irConstant = dynamic_cast<ir::Constant*>(irValue)) {
            if (auto* irGlobalValue = dynamic_cast<ir::GlobalValue*>(irConstant)) {
                return genMVRegisterFromIRGlobalValueUse(irGlobalValue, mirBasicBlock);
            }
            return genMVRegisterFromIRConstant(irConstant);
        }

        if (auto* irParameter = dynamic_cast<ir::Parameter*>(irValue)) {
            return genMVRegisterFromIRParameterUse(irParameter, mirBasicBlock);
        }

        if (auto* irInstruction = dynamic_cast<ir::Instruction*>(irValue)) {
            if (auto* irAllocaInstr = dynamic_cast<ir::AllocaInstruction*>(irValue)) {
                return genMVRegisterFromIRAllocaUse(irAllocaInstr, mirBasicBlock);
            }
            return genMVRegisterFromIRInstr(irInstruction, mirBasicBlock);
        }

        assert(false);
    }

    void genMFromIRBinaryInstr(ir::BinaryInstruction* irInstr, MBasicBlock* basicBlock) {
        auto mirOpType = MInstruction::OpType::kNone;

        // TODO: use regular enums?
        switch (irInstr->GetOpType()) {
        case ir::BinaryInstruction::OpType::kMul:
            mirOpType = MInstruction::OpType::kMul;
            break;
        case ir::BinaryInstruction::OpType::kFMul:
            mirOpType = MInstruction::OpType::kFMul;
            break;
        case ir::BinaryInstruction::OpType::kSDiv:
            mirOpType = MInstruction::OpType::kSDiv;
            break;
        case ir::BinaryInstruction::OpType::kUDiv:
            mirOpType = MInstruction::OpType::kUDiv;
            break;
        case ir::BinaryInstruction::OpType::kFDiv:
            mirOpType = MInstruction::OpType::kFDiv;
            break;
        case ir::BinaryInstruction::OpType::kSRem:
            mirOpType = MInstruction::OpType::kSRem;
            break;
        case ir::BinaryInstruction::OpType::kURem:
            mirOpType = MInstruction::OpType::kURem;
            break;
        case ir::BinaryInstruction::OpType::kAdd:
            mirOpType = MInstruction::OpType::kAdd;
            break;
        case ir::BinaryInstruction::OpType::kFAdd:
            mirOpType = MInstruction::OpType::kFAdd;
            break;
        case ir::BinaryInstruction::OpType::kSub:
            mirOpType = MInstruction::OpType::kSub;
            break;
        case ir::BinaryInstruction::OpType::kFSub:
            mirOpType = MInstruction::OpType::kFSub;
            break;
        case ir::BinaryInstruction::OpType::kShiftL:
            mirOpType = MInstruction::OpType::kShiftL;
            break;
        case ir::BinaryInstruction::OpType::kLShiftR:
            mirOpType = MInstruction::OpType::kLShiftR;
            break;
        case ir::BinaryInstruction::OpType::kAShiftR:
            mirOpType = MInstruction::OpType::kAShiftR;
            break;
        case ir::BinaryInstruction::OpType::kAnd:
            mirOpType = MInstruction::OpType::kAnd;
            break;
        case ir::BinaryInstruction::OpType::kXor:
            mirOpType = MInstruction::OpType::kXor;
            break;
        case ir::BinaryInstruction::OpType::kOr:
            mirOpType = MInstruction::OpType::kOr;
            break;
        }

        MInstruction mirInstr{mirOpType};

        MOperand definition = genMVRegisterFromIRValue(irInstr, basicBlock);
        MOperand useFirst = genMVRegisterFromIRValue(irInstr->GetLeftOperand(), basicBlock);
        MOperand useSecond = genMVRegisterFromIRValue(irInstr->GetRightOperand(), basicBlock);

        mirInstr.AddOperand(definition);
        mirInstr.AddOperand(useFirst);
        mirInstr.AddOperand(useSecond);

        basicBlock->AddInstruction(mirInstr);
    }

    void genMFromIRCompareInstr(ir::CompareInstruction* cmpInstr, MBasicBlock* basicBlock) {
        auto mirOpType = MInstruction::OpType::kCmp;
        if (cmpInstr->IsUnsigned()) {
            mirOpType = MInstruction::OpType::kUCmp;
        } else if (cmpInstr->IsFloat()) {
            mirOpType = MInstruction::OpType::kFCmp;
        }

        auto compareKind = MInstruction::CompareKind::kEqual;
        if (cmpInstr->IsNEqual()) {
            compareKind = MInstruction::CompareKind::kNEqual;
        } else if (cmpInstr->IsLess()) {
            compareKind = MInstruction::CompareKind::kLess;
        } else if (cmpInstr->IsLessEq()) {
            compareKind = MInstruction::CompareKind::kLessEq;
        } else if (cmpInstr->IsGreater()) {
            compareKind = MInstruction::CompareKind::kGreater;
        } else if (cmpInstr->IsGreaterEq()) {
            compareKind = MInstruction::CompareKind::kGreaterEq;
        }

        MInstruction mirInstr{mirOpType, compareKind};

        MOperand definition = genMVRegisterFromIRValue(cmpInstr, basicBlock);
        MOperand useFirst = genMVRegisterFromIRValue(cmpInstr->GetLeftOperand(), basicBlock);
        MOperand useSecond = genMVRegisterFromIRValue(cmpInstr->GetRightOperand(), basicBlock);

        mirInstr.AddOperand(definition);
        mirInstr.AddOperand(useFirst);
        mirInstr.AddOperand(useSecond);

        basicBlock->AddInstruction(mirInstr);
    }

    void genMFromIRCastInstr(ir::CastInstruction* castInstr, MBasicBlock* basicBlock) {
        auto irOpType = castInstr->GetOpType();

        MOperand use = genMVRegisterFromIRValue(castInstr->GetFromOperand(), basicBlock);

        if (irOpType == ir::CastInstruction::OpType::kPtrToI || 
                irOpType == ir::CastInstruction::OpType::kIToPtr ||
                    irOpType == ir::CastInstruction::OpType::kBitcast) {
            // Skip
            // TODO: Generate MOV for Bitcast?
            linkIRValueWithVReg(castInstr, use.GetRegister());
            return;
        }

        auto mirOpType = MInstruction::OpType::kNone;

        switch (castInstr->GetOpType()) {
        case ir::CastInstruction::OpType::kITrunc:
            mirOpType = MInstruction::OpType::kITrunc;
            break;
        case ir::CastInstruction::OpType::kFTrunc:
            mirOpType = MInstruction::OpType::kFTrunc;
            break;
        case ir::CastInstruction::OpType::kZExt:
            mirOpType = MInstruction::OpType::kZExt;
            break;
        case ir::CastInstruction::OpType::kSExt:
            mirOpType = MInstruction::OpType::kSExt;
            break;
        case ir::CastInstruction::OpType::kFExt:
            mirOpType = MInstruction::OpType::kFExt;
            break;
        case ir::CastInstruction::OpType::kFToUI:
            mirOpType = MInstruction::OpType::kFToUI;
            break;
        case ir::CastInstruction::OpType::kFToSI:
            mirOpType = MInstruction::OpType::kFToSI;
            break;
        case ir::CastInstruction::OpType::kUIToF:
            mirOpType = MInstruction::OpType::kUIToF;
            break;
        case ir::CastInstruction::OpType::kSIToF:
            mirOpType = MInstruction::OpType::kSIToF;
            break;
        case ir::CastInstruction::OpType::kPtrToI:
        case ir::CastInstruction::OpType::kIToPtr:
            break;
        case ir::CastInstruction::OpType::kBitcast:
            mirOpType = MInstruction::OpType::kFMul;
            break;
        }

        MInstruction mirInstr{mirOpType};
        MOperand definition = genMVRegisterFromIRValue(castInstr, basicBlock);

        mirInstr.AddOperand(definition);
        mirInstr.AddOperand(use);

        basicBlock->AddInstruction(mirInstr);
    }

    void genMFromIRStoreInstr(ir::StoreInstruction* storeInstr, MBasicBlock* basicBlock) {
        MInstruction mirStore{MInstruction::OpType::kStore};

        ir::Value* addressValue = storeInstr->GetAddressOperand();
        MOperand addressOperand = genMVRegisterFromIRValue(addressValue, basicBlock);
        mirStore.AddOperand(addressOperand);

        ir::Value* value = storeInstr->GetValueOperand();
        MOperand valueOperand = genMVRegisterFromIRValue(value, basicBlock);
        mirStore.AddOperand(valueOperand);

        basicBlock->AddInstruction(mirStore);
    }

    void genMFromIRLoadInstr(ir::LoadInstruction* loadInstr, MBasicBlock* basicBlock) {
        MFunction* mirFunction = basicBlock->GetFunction();

        ir::LoadInstruction* toValue = loadInstr;
        ir::Value* fromValue = loadInstr->GetPtrOperand();

        ir::Type* toValueType = toValue->GetType();
        if (auto* structType = dynamic_cast<ir::StructType*>(toValueType)) {
            // TODO: load return struct from stack

            // uint currentOffset = 0;
            
            // auto fromOperand = genMVRegisterFromIRValue(fromValue, basicBlock);

            // auto vregList = std::vector<uint>{};
            // vregList.reserve(structType->GetElementsNumber());
            // for (auto* memberType : structType->GetElementTypes()) {
            //     size_t memberSize = ir::Alignment::GetTypeSize(memberType);

            //     auto mirLoad = MInstruction(MInstruction::OpType::kLoad, basicBlock);
            //     uint vreg = mirFunction->NextVReg();
            //     mirLoad.AddRegister(vreg, memberSize);

            //     // TODO: deal with bitcast
            //     mirLoad.AddStackIndex(fromOperand.GetRegister(), currentOffset);
            //     currentOffset += memberSize;

            //     vregList.push_back(vreg);
            // }

            // m_IRValueToVRegList[loadInstr->GetName()] = std::move(vregList);
            return;
        }

        MInstruction mirLoad{MInstruction::OpType::kLoad};

        MOperand fromOperand = genMVRegisterFromIRValue(fromValue, basicBlock);
        MOperand toOperand = genMVRegisterFromIRValue(toValue, basicBlock);
        
        mirLoad.AddOperand(toOperand);
        mirLoad.AddOperand(fromOperand);

        basicBlock->AddInstruction(mirLoad);
    }

    void genMFromIRMemberInstr(ir::MemberInstruction* memberInstr, MBasicBlock* basicBlock) {
        MFunction* mirFunction = basicBlock->GetFunction();

        ir::Value* ptrValue = memberInstr->GetPtrOperand();
        MOperand ptrOperand = genMVRegisterFromIRValue(ptrValue, basicBlock);

        ir::Value* indexValue = memberInstr->GetIndex();
        MOperand indexOperand = genMVRegisterFromIRValue(indexValue, basicBlock);
        
        bool isImmediate = false;
        uint immOffset = 0;
        uint indexImmValue = 0;
        if (indexOperand.IsImmediate()) {
            assert(indexOperand.IsImmInteger());
            indexImmValue = indexOperand.GetImmInteger();
            isImmediate = true;
        }

        auto* ptrType = dynamic_cast<ir::PointerType*>(ptrValue->GetType());
        assert(ptrType);

        ir::Type* ptrSubType = ptrType->GetSubType();
        ir::ArrayType* arrayTypeOpt = dynamic_cast<ir::ArrayType*>(ptrSubType);
        uint64_t typeSize = ir::Alignment::GetTypeSize(ptrSubType);
        if (!memberInstr->IsDeref() || arrayTypeOpt) {
            // offset = index * sizeof(ptr_subtype) | offset = index * sizeof(arr_subtype)
            if (arrayTypeOpt) {
                ir::Type* memberType = arrayTypeOpt->GetSubType();
                typeSize = ir::Alignment::GetTypeSize(memberType);
            }
            if (isImmediate) {
                immOffset = indexImmValue * typeSize;
            }
        } else {  // struct
            // offset = member_offset
            auto* structType = dynamic_cast<ir::StructType*>(ptrSubType);
            assert(structType);

            ir::Alignment::StructLayout layout = ir::Alignment::GetStructLayout(structType);
            immOffset = layout.Offsets[indexImmValue];
        }

        // BaseReg/BaseSymbol + ScaleImm * IndexReg + DispImm
        MOperand memberOperand = genMVRegisterFromIRValue(memberInstr, basicBlock);

        MInstruction memberAddress{MInstruction::OpType::kMemberAddress};
        memberAddress.AddOperand(memberOperand);
        memberAddress.AddOperand(ptrOperand);

        if (!isImmediate) {
            memberAddress.AddImmInteger(typeSize, m_TargetMachine->GetPointerByteSize());
            memberAddress.AddOperand(indexOperand);
            memberAddress.AddImmInteger(0);
        } else {
            memberAddress.AddImmInteger(0);
            memberAddress.AddVirtualRegister(0, MType::CreateScalar(ir::Alignment::GetTypeSize(ptrValue->GetType())));
            memberAddress.AddImmInteger(immOffset, m_TargetMachine->GetPointerByteSize());
        }

        linkIRValueWithVReg(memberInstr, memberOperand.GetRegister());
        basicBlock->AddInstruction(memberAddress);
    }

    void genMFromIRPhiInstr(ir::PhiInstruction* phiInstr, MBasicBlock* basicBlock) {
        MInstruction mirPhi{MInstruction::OpType::kPhi};

        MOperand definition = genMVRegisterFromIRValue(phiInstr, basicBlock);
        mirPhi.AddOperand(definition);

        // TODO: Remove copy paste (genMVRegisterFromIRValue)
        for (size_t i = 0; i < phiInstr->GetArgumentsNumber(); ++i) {
            ir::Value* argValue = phiInstr->GetIncomingValue(i);
            if (auto* irConstant = dynamic_cast<ir::Constant*>(argValue)) {
                if (auto* irGlobalValue = dynamic_cast<ir::GlobalValue*>(argValue)) {
                    if (dynamic_cast<ir::Function*>(irGlobalValue)) {
                        mirPhi.AddFunction(irGlobalValue->GetName());
                    } else {
                        mirPhi.AddGlobalSymbol(irGlobalValue->GetName());
                    }
                } else {
                    uint64_t constantSize = ir::Alignment::GetTypeSize(irConstant->GetType());
                    if (auto* irIntConstant = dynamic_cast<ir::IntConstant*>(irConstant)) {
                        IntValue intValue = irIntConstant->GetValue();
                        if (intValue.IsSigned()) {
                            mirPhi.AddImmInteger(intValue.GetSignedValue(), constantSize);
                        } else {
                            mirPhi.AddImmInteger(intValue.GetUnsignedValue(), constantSize);
                        }
                    } else if (auto* irFloatConstant = dynamic_cast<ir::FloatConstant*>(irConstant)) {
                        FloatValue floatValue = irFloatConstant->GetValue();
                        mirPhi.AddImmFloat(floatValue.GetValue(), constantSize);
                    }
                }
            } else if (auto* irParameter = dynamic_cast<ir::Parameter*>(argValue)) {
                assert(hasIRValueVReg(irParameter));

                ir::Type* paramType = irParameter->GetType();
                MType mirType;
                if (dynamic_cast<ir::PointerType*>(paramType)) {
                    mirType = MType::CreatePointer(m_TargetMachine->GetPointerByteSize());
                } else {
                    bool isFloat = dynamic_cast<ir::FloatType*>(paramType);
                    mirType = MType::CreateScalar(ir::Alignment::GetTypeSize(paramType));
                }

                mirPhi.AddVirtualRegister(getIRValueVReg(irParameter), mirType);
            } else if (auto* irInstruction = dynamic_cast<ir::Instruction*>(argValue)) {
                MFunction* mirFunction = basicBlock->GetFunction();
                uint vreg = 0;

                if (!hasIRValueVReg(irInstruction)) {
                    vreg = mirFunction->NextVReg();
                    linkIRValueWithVReg(irInstruction, vreg);
                } else {
                    vreg = getIRValueVReg(irInstruction);
                }

                ir::Type* instrType = irInstruction->GetType();
                MType mirType;
                if (dynamic_cast<ir::PointerType*>(instrType)) {
                    mirType = MType::CreatePointer(m_TargetMachine->GetPointerByteSize());
                } else {
                    bool isFloat = dynamic_cast<ir::FloatType*>(instrType);
                    mirType = MType::CreateScalar(ir::Alignment::GetTypeSize(instrType), isFloat);
                }

                mirPhi.AddVirtualRegister(vreg, mirType);
            } else {
                assert(false);
            }
        }

        basicBlock->AddInstruction(mirPhi);
    }

    void genMFromIRBranchInstr(ir::BranchInstruction* branchInstr, MBasicBlock* basicBlock) {
        if (branchInstr->IsUnconditional()) {  // Jump
            MInstruction mirJump{MInstruction::OpType::kJump};
            ir::BasicBlock* irBasicBlock = branchInstr->GetTrueBasicBlock();
            mirJump.AddBasicBlock(m_MBBMap[irBasicBlock->GetName()]);
            basicBlock->AddInstruction(mirJump);
            return;
        }

        MInstruction mirBranch{MInstruction::OpType::kBranch};

        ir::Value* condValue = branchInstr->GetCondition();
        MOperand condOperand = genMVRegisterFromIRValue(condValue, basicBlock);
        mirBranch.AddOperand(condOperand);

        ir::BasicBlock* irTrueBB = branchInstr->GetTrueBasicBlock();
        mirBranch.AddBasicBlock(m_MBBMap[irTrueBB->GetName()]);

        // TODO: Handle fall-through
        ir::BasicBlock* irFalseBB = branchInstr->GetFalseBasicBlock();
        mirBranch.AddBasicBlock(m_MBBMap[irFalseBB->GetName()]);

        basicBlock->AddInstruction(mirBranch);
    }

    void genMFromIRCallInstr(ir::CallInstruction* callInstr, MBasicBlock* basicBlock) {
        MFunction* function = basicBlock->GetFunction();
        function->SetCaller();

        target::TargetABI* targetABI = m_TargetMachine->GetABI();
        target::RegisterSet* targetRegSet = m_TargetMachine->GetRegisterSet();

        size_t intParamIndex = 0;
        size_t floatParamIndex = 0;
        for (ir::Value* argValue : callInstr->GetArguments()) {
            // TODO: Handle struct parameters

            ir::Type* argIRType = argValue->GetType();

            target::Register targetArgReg;
            bool isFloat = dynamic_cast<ir::FloatType*>(argIRType);
            if (isFloat) {
                targetArgReg = targetABI->GetFloatArgumentRegisters()[floatParamIndex++];
            } else {  // Pointer or Integer
                targetArgReg = targetABI->GetIntArgumentRegisters()[intParamIndex++];
                uint regSize = targetArgReg.GetBytes();
                target::Register subReg = targetArgReg;
                uint argSize = ir::Alignment::GetTypeSize(argIRType);
                if (argSize < regSize) {  // Use i32 register for i8 and i16 argument values
                    uint subRegNumber = targetArgReg.GetSubRegNumbers().at(0);
                    subReg = targetRegSet->GetRegister(subRegNumber);
                    regSize = subReg.GetBytes();
                }
                targetArgReg = subReg;
            }

            MOperand argOperand = genMVRegisterFromIRValue(argValue, basicBlock);

            MInstruction movInstr{isFloat ? MInstruction::OpType::kFMov : MInstruction::OpType::kMov};
            movInstr.AddPhysicalRegister(targetArgReg);
            movInstr.AddOperand(argOperand);
            basicBlock->AddInstruction(movInstr);
        }

        ir::Function* calleeValue = callInstr->GetCallee();

        MInstruction mirCall{MInstruction::OpType::kCall};
        mirCall.AddFunction(calleeValue->GetName());
        basicBlock->AddInstruction(mirCall);

        ir::Type* callResultIRType = callInstr->GetType();
        if (dynamic_cast<ir::VoidType*>(callResultIRType)) {
            return;
        }

        uint callResultSize = ir::Alignment::GetTypeSize(callResultIRType);
        bool isFloat = dynamic_cast<ir::FloatType*>(callResultIRType);

        target::Register targetCallResultReg;
        if (isFloat) {
            targetCallResultReg = targetABI->GetFloatReturnRegisters()[0];
        } else {  // Pointer or Integer
            targetCallResultReg = targetABI->GetIntReturnRegisters()[0];
            uint regSize = targetCallResultReg.GetBytes();
            target::Register subReg = targetCallResultReg;
            while (callResultSize < regSize) {
                uint subRegNumber = targetCallResultReg.GetSubRegNumbers().at(0);
                subReg = targetRegSet->GetRegister(subRegNumber);
                regSize = subReg.GetBytes();
            }

            assert(callResultSize == regSize);
            targetCallResultReg = subReg;
        }

        MOperand callResultOperand = genMVRegisterFromIRValue(callInstr, basicBlock);

        auto movType = MInstruction::OpType::kMov;
        if (targetCallResultReg.IsFloat()) {
            movType = MInstruction::OpType::kFMov;
        }

        MInstruction movInstr{isFloat ? MInstruction::OpType::kFMov : MInstruction::OpType::kMov};
        movInstr.AddOperand(callResultOperand);
        movInstr.AddPhysicalRegister(targetCallResultReg);

        basicBlock->AddInstruction(movInstr);
    }

    void genMFromIRReturnInstr(ir::ReturnInstruction* returnInstr, MBasicBlock* basicBlock) {
        MInstruction mirReturn{MInstruction::OpType::kRet};
        if (!returnInstr->HasReturnValue()) {
            basicBlock->AddInstruction(mirReturn);
            return;
        }

        ir::Value* returnValue = returnInstr->GetReturnValue();
        ir::Type* returnIRType = returnValue->GetType();

        target::TargetABI* targetABI = m_TargetMachine->GetABI();
        target::RegisterSet* targetRegSet = m_TargetMachine->GetRegisterSet();

        target::Register targetReturnReg;
        bool isFloat = dynamic_cast<ir::FloatType*>(returnIRType);
        if (isFloat) {
            targetReturnReg = targetABI->GetFloatReturnRegisters()[0];
        } else {  // Pointer or Integer
            targetReturnReg = targetABI->GetIntReturnRegisters()[0];
            uint regSize = targetReturnReg.GetBytes();
            target::Register subReg = targetReturnReg;
            uint returnSize = ir::Alignment::GetTypeSize(returnIRType);
            if (returnSize < regSize) {  // Use i32 register for i8 and i16 return values
                uint subRegNumber = targetReturnReg.GetSubRegNumbers().at(0);
                subReg = targetRegSet->GetRegister(subRegNumber);
                regSize = subReg.GetBytes();
            }
            targetReturnReg = subReg;
        }

        MOperand returnOperand = genMVRegisterFromIRValue(returnValue, basicBlock);

        MInstruction movInstr{isFloat ? MInstruction::OpType::kFMov : MInstruction::OpType::kMov};
        movInstr.AddPhysicalRegister(targetReturnReg);
        movInstr.AddOperand(returnOperand);
        basicBlock->AddInstruction(movInstr);

        basicBlock->AddInstruction(mirReturn);
    }

    void genMFromIRInstruction(ir::Instruction* instruction, MBasicBlock* basicBlock) {
        // TODO: memcpy

        if (auto* allocaInstr = dynamic_cast<ir::AllocaInstruction*>(instruction)) {
            genMFromIRAllocaInstr(allocaInstr, basicBlock);
        } else if (auto* binaryInstr = dynamic_cast<ir::BinaryInstruction*>(instruction)) {
            genMFromIRBinaryInstr(binaryInstr, basicBlock);
        } else if (auto* compareInstr = dynamic_cast<ir::CompareInstruction*>(instruction)) {
            genMFromIRCompareInstr(compareInstr, basicBlock);
        } else if (auto* castInstr = dynamic_cast<ir::CastInstruction*>(instruction)) {
            genMFromIRCastInstr(castInstr, basicBlock);
        } else if (auto* storeInstr = dynamic_cast<ir::StoreInstruction*>(instruction)) {
            genMFromIRStoreInstr(storeInstr, basicBlock);
        }else if (auto* loadInstr = dynamic_cast<ir::LoadInstruction*>(instruction)) {
            genMFromIRLoadInstr(loadInstr, basicBlock);
        } else if (auto* memberInstr = dynamic_cast<ir::MemberInstruction*>(instruction)) {
            genMFromIRMemberInstr(memberInstr, basicBlock);
        } else if (auto* phiInstr = dynamic_cast<ir::PhiInstruction*>(instruction)) {
            genMFromIRPhiInstr(phiInstr, basicBlock);
        } else if (auto* branchInstr = dynamic_cast<ir::BranchInstruction*>(instruction)) {
            genMFromIRBranchInstr(branchInstr, basicBlock);
        } else if (auto* switchInstr = dynamic_cast<ir::SwitchInstruction*>(instruction)) {
            // genMFromIRSwitchInstr(switchInstr, basicBlock);
        } else if (auto* callInstr = dynamic_cast<ir::CallInstruction*>(instruction)) {
            genMFromIRCallInstr(callInstr, basicBlock);
        } else if (auto* returnInstr = dynamic_cast<ir::ReturnInstruction*>(instruction)) {
            genMFromIRReturnInstr(returnInstr, basicBlock);
        } else {
            assert(false);
        }
    }

    GlobalDataArea genGlobalDataArea(ir::GlobalVariable* globalVar) {
        auto* globalVarPtrType = dynamic_cast<ir::PointerType*>(globalVar->GetType());
        assert(globalVarPtrType);

        ir::Type* globalVarType = globalVarPtrType->GetSubType();
        if (auto* irStructType = dynamic_cast<ir::StructType*>(globalVarType)) {
            return genStructDataArea(globalVar, irStructType);
        }
        if (auto* irArrayType = dynamic_cast<ir::ArrayType*>(globalVarType)) {
            return genArrayDataArea(globalVar, irArrayType);
        }
        return genScalarDataArea(globalVar);
    }

    GlobalDataArea genStructDataArea(ir::GlobalVariable* globalVar, ir::StructType* irStructType) {
        GlobalDataArea globalDataArea{globalVar->GetName()};

        ir::Alignment::StructLayout structLayout = ir::Alignment::GetStructLayout(irStructType);
        uint64_t structSize = structLayout.Size;

        assert(globalVar->IsInitList() && "GlobalVariable init must be a list");
        std::vector<ir::Constant*> initList = globalVar->GetInitList();
        if (initList.empty()) {
            globalDataArea.AddIntegerSlot(structSize, 0);
            return globalDataArea;
        }

        std::vector<ir::Type*> memberTypes = irStructType->GetElementTypes();
        std::vector<uint64_t> memberOffsets = structLayout.Offsets;
        uint64_t currentOffset = 0;
        for (size_t i = 0; i < memberTypes.size(); ++i) {
            assert(memberOffsets[i] > currentOffset);

            uint64_t padding = memberOffsets[i] - currentOffset;
            if (padding > 0) {
                globalDataArea.AddIntegerSlot(padding, 0);
            }

            uint64_t memberSize = ir::Alignment::GetTypeSize(memberTypes[i]);
            addGlobalSlot(globalDataArea, initList[i], memberSize); 

            currentOffset = memberOffsets[i] + memberSize;
        }

        uint64_t lastPadding = structSize - currentOffset;
        if (lastPadding > 0) {
            globalDataArea.AddIntegerSlot(lastPadding, 0);
        } 

        return globalDataArea;
    }

    GlobalDataArea genArrayDataArea(ir::GlobalVariable* globalVar, ir::ArrayType* irArrayType) {
        GlobalDataArea globalDataArea{globalVar->GetName()};

        if (globalVar->IsInitString()) {
            globalDataArea.AddStringSlot(globalVar->GetInitString());
            return globalDataArea;
        }

        uint64_t size = ir::Alignment::GetTypeSize(irArrayType);

        assert(globalVar->IsInitList() && "GlobalVariable init must be a list");
        std::vector<ir::Constant*> initList = globalVar->GetInitList();
        if (initList.empty()) {
            globalDataArea.AddIntegerSlot(size, 0);
            return globalDataArea;
        }

        ir::Type* memberType = irArrayType->GetSubType();
        uint64_t memberSize = ir::Alignment::GetTypeSize(memberType);
        for (ir::Constant* init : initList) {
            addGlobalSlot(globalDataArea, init, memberSize);
        }
        return globalDataArea;
    }

    void addGlobalSlot(GlobalDataArea& globalDataArea, ir::Constant* init, uint64_t size) {
        if (auto* intInit = dynamic_cast<ir::IntConstant*>(init)) {
            IntValue intValue = intInit->GetValue();
            globalDataArea.AddIntegerSlot(size, intValue.GetUnsignedValue());
            return;
        }

        if (auto* floatInit = dynamic_cast<ir::FloatConstant*>(init)) {
            FloatValue floatValue = floatInit->GetValue();
            double value = floatValue.GetValue();
            if (floatValue.IsDoublePrecision()) {
                globalDataArea.AddDoubleSlot(value);
            } else {
                globalDataArea.AddFloatSlot(static_cast<float>(value));
            }
        }
    }

    GlobalDataArea genScalarDataArea(ir::GlobalVariable* globalVar) {
        GlobalDataArea globalDataArea{globalVar->GetName()};
        uint64_t scalarSize = ir::Alignment::GetTypeSize(globalVar->GetType());

        if (globalVar->IsInitVariable()) {
            ir::GlobalVariable* initVariable = globalVar->GetInitVariable();
            globalDataArea.AddLabelSlot(scalarSize, initVariable->GetName());
        }

        if (!globalVar->HasInit()) {
            globalDataArea.AddIntegerSlot(scalarSize, 0);
            return globalDataArea;
        }

        addGlobalSlot(globalDataArea, globalVar->GetInit(), scalarSize);
        return globalDataArea;
    }

    TScopePtr<MFunction> genMFromIRFunction(ir::Function* irFunction) {
        if (irFunction->IsDeclaration()) {
            // TODO: Handle PLT calls
            return nullptr;
        }

        auto mirFunctionScope = CreateScope<MFunction>(irFunction->GetName());
        MFunction* mirFunction = mirFunctionScope.get();


        for (ir::BasicBlock* basicBlock : irFunction->GetBasicBlocks()) {
            const std::string& name = basicBlock->GetName();
            auto MBB = CreateScope<MBasicBlock>(name, mirFunction);
            mirFunction->AddBasicBlock(std::move(MBB));
            m_MBBMap[name] = mirFunction->GetLastBasicBlock();
        }

        updateMIRFunctionParameters(irFunction, mirFunction);

        for (ir::BasicBlock* basicBlock : irFunction->GetBasicBlocks()) {
            MBasicBlock* mirBasicBlock = m_MBBMap[basicBlock->GetName()];

            for (ir::BasicBlock* predecessor : basicBlock->GetPredecessors()) {
                mirBasicBlock->AddPredecessor(m_MBBMap[predecessor->GetName()]);
            }
            for (ir::BasicBlock* successor : basicBlock->GetSuccessors()) {
                mirBasicBlock->AddSuccessor(m_MBBMap[successor->GetName()]);
            }

            for (ir::Instruction* instruction : basicBlock->GetInstructions()) {
                genMFromIRInstruction(instruction, mirBasicBlock);
            }
        }

        return mirFunctionScope;
    }

    void genMFromIRAllocaInstr(ir::AllocaInstruction* allocaInstr, MBasicBlock* basicBlock) {
        auto* allocaPtrType = dynamic_cast<ir::PointerType*>(allocaInstr->GetType());
        assert(allocaPtrType);

        ir::Type* elementType = allocaPtrType->GetSubType();

        MFunction* mirFunction = basicBlock->GetFunction();
        uint vreg = mirFunction->NextVReg();
        linkIRValueWithVReg(allocaInstr, vreg);

        mirFunction->AddLocalData(vreg, ir::Alignment::GetTypeSize(elementType),
                                        ir::Alignment::GetTypeAlignment(elementType));
    }

    void updateMIRFunctionParameters(ir::Function* irFunction, MFunction* mirFunction) {
        size_t intParamIndex = 0;
        size_t floatParamIndex = 0;
        for (ir::Parameter* irParam : irFunction->GetParameters()) {
            ir::Type* paramIRType = irParam->GetType();

            // TODO: add Target Machine to Alignment
            uint64_t paramSize = ir::Alignment::GetTypeSize(paramIRType);
            bool isFloat = dynamic_cast<ir::FloatType*>(paramIRType);

            // if (dynamic_cast<ir::PointerType*>(paramIRType)) {
            //     vreg = mirFunction->AddParameter(MType::CreatePointer(pointerSize), false, isImplicit);
            // } else {
            //     vreg = mirFunction->AddParameter(MType::CreateScalar(paramSize), isFloat, false);
            // }
            // linkIRValueWithVReg(irParam, vreg);

            // if (dynamic_cast<ir::StructType*>(paramIRType)) {
            //     auto targetABI = m_TargetMachine->GetABI();
            //     uint maxStructParamSize = targetABI->GetMaxStructParamSize();
            //     uint structParamNum = maxStructParamSize / pointerSize;

            //     auto paramVRegs = std::vector<uint>{};
            //     paramVRegs.reserve(structParamNum);
            //     for (uint i = 0; i < structParamNum; ++i) {
            //         uint vreg = mirFunction->AddParameter(MType::CreateScalar(paramSize));
            //         paramVRegs.push_back(vreg);
            //     }
            //     linkIRParamWithVRegs(irParam, paramVRegs);
            //     continue;
            // }

            target::TargetABI* targetABI = m_TargetMachine->GetABI();
            target::RegisterSet* targetRegSet = m_TargetMachine->GetRegisterSet();
            target::Register targetParamReg;
            if (isFloat) {
                targetParamReg = targetABI->GetFloatArgumentRegisters()[floatParamIndex++];
            } else {  // Pointer or Integer
                targetParamReg = targetABI->GetIntArgumentRegisters()[intParamIndex++];
                uint regSize = targetParamReg.GetBytes();
                target::Register subReg = targetParamReg;
                while (paramSize < regSize) {
                    uint subRegNumber = targetParamReg.GetSubRegNumbers().at(0);
                    subReg = targetRegSet->GetRegister(subRegNumber);
                    regSize = subReg.GetBytes();
                }

                assert(paramSize == regSize);
                targetParamReg = subReg;
            }

            MBasicBlock* firstBasicBlock = mirFunction->GetFirstBasicBlock();
            MInstruction movInstr{isFloat ? MInstruction::OpType::kFMov : MInstruction::OpType::kMov};

            MType mirType;
            if (dynamic_cast<ir::PointerType*>(paramIRType)) {
                mirType = MType::CreatePointer(m_TargetMachine->GetPointerByteSize());
            } else {
                mirType = MType::CreateScalar(paramSize, isFloat);
            }

            uint vreg = mirFunction->NextVReg();
            linkIRValueWithVReg(irParam, vreg);
            movInstr.AddVirtualRegister(vreg, mirType);
            movInstr.AddPhysicalRegister(targetParamReg);
            firstBasicBlock->AddInstructionToBegin(movInstr);

            // if (dynamic_cast<ir::PointerType*>(paramIRType)) {
                // uint vreg = mirFunction->AddParameter(MType::CreatePointer(pointerSize), false, isImplicit);
                // linkIRValueWithVReg(irParam, vreg);
            // } else if (paramSize <= pointerSize) {
                // uint vreg = mirFunction->AddParameter(MType::CreateScalar(paramSize), isFloat, isImplicit);
                // linkIRValueWithVReg(irParam, vreg);
            // } else {  // divide into multiple registers
                // uint regNum = paramSize / pointerSize;
                // auto paramVRegs = std::vector<uint>{};
                // paramVRegs.reserve(regNum);
                // for (uint i = 0; i < regNum; ++i) {
                //     uint vreg = mirFunction->AddParameter(
                //                     MType::CreateScalar(pointerSize), isFloat, isImplicit);
                //     paramVRegs.push_back(vreg);
                // }
                // linkIRParamWithVRegs(irParam, paramVRegs);
            // }
        }
    }

    // void linkIRParamWithVRegs(ir::Parameter* param, const std::vector<uint>& vregs) {
    //     m_IRParamToVRegs[param->GetName()] = vregs;
    // }

private:
    ir::IRProgram& m_IRProgram;
    target::TargetMachine* m_TargetMachine = nullptr;

    std::unordered_map<std::string, uint> m_IRValueToVReg;
    std::unordered_map<std::string, std::vector<uint>> m_IRValueToVRegList;

    std::unordered_map<std::string, MBasicBlock*> m_MBBMap;
};

}  // namespace gen
