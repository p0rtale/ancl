#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <Ancl/AnclIR/IRProgram.hpp>
#include <Ancl/CodeGen/MachineIR/MIRProgram.hpp>
#include <Ancl/CodeGen/Target/Base/Machine.hpp>
#include <Ancl/CodeGen/MachineIR/MBasicBlock.hpp>


namespace gen {

class MIRGenerator {
public:
    MIRGenerator(ir::IRProgram& irProgram, target::TargetMachine* targetMachine)
        : m_IRProgram(irProgram), m_TargetMachine(targetMachine) {}

    void Generate(MIRProgram& machineProgram) {
        for (auto* irFunction : m_IRProgram.GetFunctions()) {
            auto mirFunction = genMFromIRFunction(irFunction);
            if (mirFunction) {
                machineProgram.AddFunction(std::move(mirFunction));
            }
        }

        for (auto* globalVar : m_IRProgram.GetGlobalVars()) {
            auto mirGlobalDataArea = genGlobalDataArea(globalVar);
            machineProgram.AddGlobalDataArea(std::move(mirGlobalDataArea));
        }
    }

private:
    void linkIRValueWithVReg(ir::Value* value, uint vreg) {
        // NB: unnamed values must be assigned numbers in the AnclIR
        m_IRValueToVReg[value->GetName()] = vreg;
    }

    bool hasIRValueVReg(ir::Value* value) {
        return m_IRValueToVReg.contains(value->GetName());
    }

    uint getIRValueVReg(ir::Value* value) {
        return m_IRValueToVReg.at(value->GetName());
    }

    void addDefinition(MInstruction instr, MOperand operand, MBasicBlock* basicBlock) {

    }

private:
    MOperand genMVRegisterFromIRGlobalValueUse(ir::GlobalValue* irGlobalValue, MBasicBlock* mirBasicBlock) {
        auto* mirFunction = mirBasicBlock->GetFunction();

        auto globalAddressInstr = MInstruction(MInstruction::OpType::kGlobalAddress, mirBasicBlock);

        uint vreg = mirFunction->NextVReg();
        auto ptrReg = MOperand::CreateRegister(vreg);
        ptrReg.SetType(MType::CreatePointer(m_TargetMachine->GetPointerByteSize()));

        globalAddressInstr.AddOperand(ptrReg);

        if (dynamic_cast<ir::Function*>(irGlobalValue)) {
            globalAddressInstr.AddFunction(irGlobalValue->GetName());
        } else {
            globalAddressInstr.AddGlobalSymbol(irGlobalValue->GetName());
        }

        // m_VRegToInstrDef[vreg] = mirBasicBlock->AddInstruction(globalAddressInstr);
        mirBasicBlock->AddInstruction(globalAddressInstr);

        return *globalAddressInstr.GetDefinition();
    }

    MOperand genMVRegisterFromIRConstant(ir::Value* irConstant) {
        uint constantSize = ir::Alignment::GetTypeSize(irConstant->GetType());

        if (auto* irIntConstant = dynamic_cast<ir::IntConstant*>(irConstant)) {
            auto intValue = irIntConstant->GetValue();
            auto mirOperand = MOperand::CreateImmInteger(intValue.GetValue(), constantSize);
            mirOperand.SetType(MType::CreateScalar(constantSize));
            return mirOperand;
        }

        if (auto* irFloatConstant = dynamic_cast<ir::FloatConstant*>(irConstant)) {
            auto floatValue = irFloatConstant->GetValue();
            auto mirOperand = MOperand::CreateImmFloat(floatValue.GetValue(), constantSize);
            mirOperand.SetType(MType::CreateScalar(constantSize));
            return mirOperand;      
        }

        assert(false);
    }

    MOperand genMVRegisterFromIRParameterUse(ir::Parameter* irParameter, MBasicBlock* mirBasicBlock) {
        auto mirFunction = mirBasicBlock->GetFunction();

        // TODO: handle spilled parameters

        auto mirOperand = MOperand::CreateParameter(getIRValueVReg(irParameter));

        auto paramType = irParameter->GetType();
        if (dynamic_cast<ir::PointerType*>(paramType)) {
            mirOperand.SetType(MType::CreatePointer(m_TargetMachine->GetPointerByteSize()));
        } else {
            mirOperand.SetType(MType::CreateScalar(ir::Alignment::GetTypeSize(paramType)));
        }

        return mirOperand;
    }

    MOperand genMVRegisterFromIRInstr(ir::Instruction* irInstruction, MBasicBlock* mirBasicBlock) {
        auto mirFunction = mirBasicBlock->GetFunction();
        uint vreg = 0;

        // TODO: spilled return values?

        if (!hasIRValueVReg(irInstruction)) {
            vreg = mirFunction->NextVReg();
            linkIRValueWithVReg(irInstruction, vreg);
        } else {
            vreg = getIRValueVReg(irInstruction);
            // TODO: stack slot?
        }

        auto mirOperand = MOperand::CreateRegister(vreg);

        auto instrType = irInstruction->GetType();
        if (dynamic_cast<ir::PointerType*>(instrType)) {
            mirOperand.SetType(MType::CreatePointer(m_TargetMachine->GetPointerByteSize()));
        } else {
            bool isFloat = dynamic_cast<ir::FloatType*>(instrType);
            mirOperand.SetType(MType::CreateScalar(ir::Alignment::GetTypeSize(instrType), isFloat));
        }

        return mirOperand;
    }

    MOperand genMVRegisterFromIRAllocaUse(ir::AllocaInstruction* irAlloca, MBasicBlock* mirBasicBlock) {
        auto* mirFunction = mirBasicBlock->GetFunction();

        auto stackAddress = MInstruction(MInstruction::OpType::kStackAddress, mirBasicBlock);

        uint vreg = mirFunction->NextVReg();
        auto ptrReg = MOperand::CreateRegister(vreg);
        ptrReg.SetType(MType::CreatePointer(m_TargetMachine->GetPointerByteSize()));

        stackAddress.AddOperand(ptrReg);
        stackAddress.AddStackIndex(getIRValueVReg(irAlloca));

        // m_VRegToInstrDef[vreg] = mirBasicBlock->AddInstruction(stackAddress);
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
        case ir::BinaryInstruction::OpType::kFRem:
            mirOpType = MInstruction::OpType::kFRem;
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

        auto mirInstr = MInstruction(mirOpType, basicBlock);

        auto definition = genMVRegisterFromIRValue(irInstr, basicBlock);
        auto useFirst = genMVRegisterFromIRValue(irInstr->GetLeftOperand(), basicBlock);
        auto useSecond = genMVRegisterFromIRValue(irInstr->GetRightOperand(), basicBlock);

        mirInstr.AddOperand(definition);
        mirInstr.AddOperand(useFirst);
        mirInstr.AddOperand(useSecond);

        // m_VRegToInstrDef[definition.GetVRegister()] = basicBlock->AddInstruction(mirInstr);
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

        auto mirInstr = MInstruction(mirOpType, compareKind, basicBlock);

        auto definition = genMVRegisterFromIRValue(cmpInstr, basicBlock);
        auto useFirst = genMVRegisterFromIRValue(cmpInstr->GetLeftOperand(), basicBlock);
        auto useSecond = genMVRegisterFromIRValue(cmpInstr->GetRightOperand(), basicBlock);

        mirInstr.AddOperand(definition);
        mirInstr.AddOperand(useFirst);
        mirInstr.AddOperand(useSecond);

        // m_VRegToInstrDef[definition.GetVRegister()] = basicBlock->AddInstruction(mirInstr);
        basicBlock->AddInstruction(mirInstr);
    }

    void genMFromIRStoreInstr(ir::StoreInstruction* storeInstr, MBasicBlock* basicBlock) {
        auto mirStore = MInstruction(MInstruction::OpType::kStore, basicBlock);

        auto* toValue = storeInstr->GetToOperand();
        auto toOperand = genMVRegisterFromIRValue(toValue, basicBlock);
        mirStore.AddMemory(toOperand.GetRegister());

        auto* fromValue = storeInstr->GetFromOperand();
        auto fromOperand = genMVRegisterFromIRValue(fromValue, basicBlock);
        mirStore.AddOperand(fromOperand);

        basicBlock->AddInstruction(mirStore);
    }

    void genMFromIRLoadInstr(ir::LoadInstruction* loadInstr, MBasicBlock* basicBlock) {
        auto mirFunction = basicBlock->GetFunction();

        auto* toValue = loadInstr;
        auto* fromValue = loadInstr->GetPtrOperand();

        auto toValueType = toValue->GetType();
        if (auto structType = dynamic_cast<ir::StructType*>(toValueType)) {
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

        auto mirLoad = MInstruction(MInstruction::OpType::kLoad, basicBlock);

        auto toOperand = genMVRegisterFromIRValue(toValue, basicBlock);
        mirLoad.AddOperand(toOperand);

        auto fromOperand = genMVRegisterFromIRValue(fromValue, basicBlock);
        mirLoad.AddMemory(fromOperand.GetRegister());

        // m_VRegToInstrDef[toOperand.GetVRegister()] = basicBlock->AddInstruction(mirLoad);
        basicBlock->AddInstruction(mirLoad);
    }

    void genMFromIRMemberInstr(ir::MemberInstruction* memberInstr, MBasicBlock* basicBlock) {
        auto mirFunction = basicBlock->GetFunction();

        auto mirMember = MInstruction(MInstruction::OpType::kLoad, basicBlock);

        auto* ptrValue = memberInstr->GetPtrOperand();
        auto ptrOperand = genMVRegisterFromIRValue(ptrValue, basicBlock);

        auto* indexValue = memberInstr->GetIndex();
        auto indexOperand = genMVRegisterFromIRValue(indexValue, basicBlock);
        
        uint vregOffset = 0;

        bool isImmediate = false;
        int64_t immOffset = 0;
        int64_t indexImmValue = 0;
        if (indexOperand.IsImmediate()) {
            assert(indexOperand.IsImmInteger());
            indexImmValue = indexOperand.GetImmInteger();
            isImmediate = true;
        }

        auto ptrType = dynamic_cast<ir::PointerType*>(ptrValue->GetType());
        assert(ptrType);

        auto ptrSubType = ptrType->GetSubType();
        auto arrayTypeOpt = dynamic_cast<ir::ArrayType*>(ptrSubType);
        if (!memberInstr->IsDeref() || arrayTypeOpt) {
            // offset = index * sizeof(ptr_subtype) | offset = index * sizeof(arr_subtype)
            size_t typeSize = ir::Alignment::GetTypeSize(ptrSubType);
            if (arrayTypeOpt) {
                auto memberType = arrayTypeOpt->GetSubType();
                typeSize = ir::Alignment::GetTypeSize(memberType);
            }
            if (isImmediate) {
                immOffset = indexImmValue * typeSize;
            } else {
                auto mirMul = MInstruction(MInstruction::OpType::kMul, basicBlock);
                vregOffset = mirFunction->NextVReg();

                auto ptrReg = MOperand::CreateRegister(vregOffset);
                ptrReg.SetType(MType::CreatePointer(m_TargetMachine->GetPointerByteSize()));
                mirMul.AddOperand(ptrReg);

                mirMul.AddOperand(indexOperand);
                mirMul.AddImmInteger(typeSize, m_TargetMachine->GetPointerByteSize());

                basicBlock->AddInstruction(mirMul);
            }
        } else {  // struct
            // offset = member_offset
            auto structType = dynamic_cast<ir::StructType*>(ptrSubType);
            assert(structType);

            auto layout = ir::Alignment::GetStructLayout(structType);
            immOffset = layout.Offsets[indexImmValue];
        }

        auto mirAdd = MInstruction(MInstruction::OpType::kAdd, basicBlock);
        mirAdd.AddOperand(ptrOperand);
        mirAdd.AddOperand(genMVRegisterFromIRValue(ptrValue, basicBlock));
        if (isImmediate) {
            mirAdd.AddImmInteger(immOffset, m_TargetMachine->GetPointerByteSize());
        } else {
            mirAdd.AddRegister(vregOffset, m_TargetMachine->GetPointerByteSize());
        }

        linkIRValueWithVReg(memberInstr, ptrOperand.GetRegister());

        // TODO: ...
        basicBlock->AddInstruction(mirMember);
    }

    void genMFromIRBranchInstr(ir::BranchInstruction* branchInstr, MBasicBlock* basicBlock) {
        if (branchInstr->IsUnconditional()) {  // Jump
            auto mirJump = MInstruction(MInstruction::OpType::kJump, basicBlock);
            auto irBasicBlock = branchInstr->GetTrueBasicBlock();
            mirJump.AddBasicBlock(m_MBBMap[irBasicBlock->GetName()]);
            basicBlock->AddInstruction(mirJump);
            return;
        }

        auto mirBranch = MInstruction(MInstruction::OpType::kBranch, basicBlock);

        auto condValue = branchInstr->GetCondition();
        auto condOperand = genMVRegisterFromIRValue(condValue, basicBlock);
        mirBranch.AddOperand(condOperand);

        auto irTrueBB = branchInstr->GetTrueBasicBlock();
        mirBranch.AddBasicBlock(m_MBBMap[irTrueBB->GetName()]);

        auto irFalseBB = branchInstr->GetFalseBasicBlock();
        if (irFalseBB) {
            mirBranch.AddBasicBlock(m_MBBMap[irFalseBB->GetName()]);
        }

        basicBlock->AddInstruction(mirBranch);
    }

    void genMFromIRCallInstr(ir::CallInstruction* callInstr, MBasicBlock* basicBlock) {
        auto mirCall = MInstruction(MInstruction::OpType::kCall, basicBlock);
        
        auto calleeValue = callInstr->GetCallee();

        for (auto* argValue : callInstr->GetArguments()) {
            // TODO: handle struct param
            // TODO: ...
        }

        // TODO: save return physical registers?

        basicBlock->AddInstruction(mirCall);
    }

    void genMFromIRReturnInstr(ir::ReturnInstruction* returnInstr, MBasicBlock* basicBlock) {
        auto mirReturn = MInstruction(MInstruction::OpType::kRet, basicBlock);
        if (!returnInstr->HasReturnValue()) {
            basicBlock->AddInstruction(mirReturn);
            return;
        }

        auto returnValue = returnInstr->GetReturnValue();
        auto returnOperand = genMVRegisterFromIRValue(returnValue, basicBlock);

        // TODO: move to physical registers

        basicBlock->AddInstruction(mirReturn);
    }

    void genMFromIRInstruction(ir::Instruction* instruction, MBasicBlock* basicBlock) {
        // TODO: memcpy

        // NB: allocas are processed in handleAllocaInstruction

        if (auto binaryInstr = dynamic_cast<ir::BinaryInstruction*>(instruction)) {
            genMFromIRBinaryInstr(binaryInstr, basicBlock);
        } else if (auto compareInstr = dynamic_cast<ir::CompareInstruction*>(instruction)) {
            genMFromIRCompareInstr(compareInstr, basicBlock);
        } else if (auto castInstr = dynamic_cast<ir::CastInstruction*>(instruction)) {
            // genMFromIRCastInstr(castInstr, basicBlock);
        } else if (auto storeInstr = dynamic_cast<ir::StoreInstruction*>(instruction)) {
            genMFromIRStoreInstr(storeInstr, basicBlock);
        }else if (auto loadInstr = dynamic_cast<ir::LoadInstruction*>(instruction)) {
            genMFromIRLoadInstr(loadInstr, basicBlock);
        } else if (auto memberInstr = dynamic_cast<ir::MemberInstruction*>(instruction)) {
            genMFromIRMemberInstr(memberInstr, basicBlock);
        } else if (auto branchInstr = dynamic_cast<ir::BranchInstruction*>(instruction)) {
            genMFromIRBranchInstr(branchInstr, basicBlock);
        } else if (auto switchInstr = dynamic_cast<ir::SwitchInstruction*>(instruction)) {
            // genMFromIRSwitchInstr(switchInstr, basicBlock);
        } else if (auto callInstr = dynamic_cast<ir::CallInstruction*>(instruction)) {
            genMFromIRCallInstr(callInstr, basicBlock);
        } else if (auto returnInstr = dynamic_cast<ir::ReturnInstruction*>(instruction)) {
            genMFromIRReturnInstr(returnInstr, basicBlock);
        } else {
            assert(false);
        }
    }

    GlobalDataArea genGlobalDataArea(ir::GlobalVariable* globalVar) {
        auto globalVarPtrType = dynamic_cast<ir::PointerType*>(globalVar->GetType());
        assert(globalVarPtrType);

        auto globalVarType = globalVarPtrType->GetSubType();

        if (auto irStructType = dynamic_cast<ir::StructType*>(globalVarType)) {
            return genStructDataArea(globalVar, irStructType);
        }
        if (auto irArrayType = dynamic_cast<ir::ArrayType*>(globalVarType)) {
            return genArrayDataArea(globalVar, irArrayType);
        }

        return genScalarDataArea(globalVar);
    }

    GlobalDataArea genStructDataArea(ir::GlobalVariable* globalVar, ir::StructType* irStructType) {
        auto globalDataArea = GlobalDataArea{globalVar->GetName()};

        assert(!globalVar->GetInit());  // GlobalVar init must be a list

        auto structLayout = ir::Alignment::GetStructLayout(irStructType);
        uint structSize = structLayout.Size;

        auto initList = globalVar->GetInitList();
        if (initList.empty()) {
            globalDataArea.AddSlot(structSize, 0);
            return globalDataArea;
        }

        auto memberTypes = irStructType->GetElementTypes();
        auto memberOffsets = structLayout.Offsets;
        uint currentOffset = 0;
        for (size_t i = 0; i < memberTypes.size(); ++i) {
            uint padding = memberOffsets[i] - currentOffset;
            if (padding > 0) {
                globalDataArea.AddSlot(padding, 0);
            }

            uint memberSize = ir::Alignment::GetTypeSize(memberTypes[i]);
            addGlobalSlot(globalDataArea, initList[i], memberSize); 

            currentOffset = memberOffsets[i] + memberSize;
        }

        uint lastPadding = structSize - currentOffset;
        if (lastPadding > 0) {
            globalDataArea.AddSlot(lastPadding, 0);
        } 

        return globalDataArea;
    }

    GlobalDataArea genArrayDataArea(ir::GlobalVariable* globalVar, ir::ArrayType* irArrayType) {
        auto globalDataArea = GlobalDataArea{globalVar->GetName()};

        if (globalVar->IsStringInit()) {
            auto initString = globalVar->GetInitString();
            globalDataArea.AddSlot(initString);
            return globalDataArea;
        }

        assert(!globalVar->GetInit());  // GlobalVar init must be a list

        uint size = ir::Alignment::GetTypeSize(irArrayType);

        auto initList = globalVar->GetInitList();
        if (initList.empty()) {
            globalDataArea.AddSlot(size, 0);
            return globalDataArea;
        }

        auto* memberType = irArrayType->GetSubType();
        uint memberSize = ir::Alignment::GetTypeSize(memberType);
        for (auto* init : initList) {
            addGlobalSlot(globalDataArea, init, memberSize);
        }
        return globalDataArea;
    }

    void addGlobalSlot(GlobalDataArea& globalDataArea, ir::Constant* init, uint size) {
        if (auto* intInit = dynamic_cast<ir::IntConstant*>(init)) {
            auto intValue = intInit->GetValue();
            globalDataArea.AddSlot(size, intValue.GetValue());
            return;
        }

        if (auto* floatInit = dynamic_cast<ir::FloatConstant*>(init)) {
            auto floatValue = floatInit->GetValue();
            auto value = floatValue.GetValue();
            if (floatValue.IsDoublePrecision()) {
                globalDataArea.AddSlot(value);
            } else {
                globalDataArea.AddSlot(static_cast<float>(value));
            }
        }
    }

    GlobalDataArea genScalarDataArea(ir::GlobalVariable* globalVar) {
        auto globalDataArea = GlobalDataArea{globalVar->GetName()};
        uint scalarSize = ir::Alignment::GetTypeSize(globalVar->GetType());

        if (!globalVar->HasInit()) {
            globalDataArea.AddSlot(scalarSize, 0);
            return globalDataArea;
        }

        addGlobalSlot(globalDataArea, globalVar->GetInit(), scalarSize);
        return globalDataArea;
    }

    TScopePtr<MFunction> genMFromIRFunction(ir::Function* irFunction) {
        if (irFunction->IsDeclaration()) {
            return nullptr;
        }

        auto mirFunctionScope = CreateScope<MFunction>(irFunction->GetName());
        auto mirFunction = mirFunctionScope.get();

        updateMIRFunctionParameters(irFunction, mirFunction);

        for (auto* basicBlock : irFunction->GetBasicBlocks()) {
            auto name = basicBlock->GetName();
            auto MBB = CreateScope<MBasicBlock>(name, mirFunction);
            mirFunction->AddBasicBlock(std::move(MBB));
            m_MBBMap[name] = mirFunction->GetLastBasicBlock();
        }

        size_t basicBlockIdx = 0;
        for (auto* basicBlock : irFunction->GetBasicBlocks()) {
            for (auto* instruction : basicBlock->GetInstructions()) {
                if (auto allocaInstr = dynamic_cast<ir::AllocaInstruction*>(instruction)) {
                    handleAllocaInstruction(allocaInstr, mirFunction);
                    continue;
                }

                genMFromIRInstruction(instruction, mirFunction->GetBasicBlock(basicBlockIdx));

                // TODO: delete instructions after the terminator?
            }
            ++basicBlockIdx;
        }

        return mirFunctionScope;
    }

    void handleAllocaInstruction(ir::AllocaInstruction* allocaInstr, MFunction* mirFunction) {
        auto allocaPtrType = dynamic_cast<ir::PointerType*>(allocaInstr->GetType());
        assert(allocaPtrType);

        auto elementType = allocaPtrType->GetSubType();

        uint vreg = mirFunction->NextVReg();
        linkIRValueWithVReg(allocaInstr, vreg);
        
        mirFunction->AddLocalData(vreg, ir::Alignment::GetTypeSize(elementType),
                                        ir::Alignment::GetTypeAlignment(elementType));
    }

    void updateMIRFunctionParameters(ir::Function* irFunction, MFunction* mirFunction) {
        // TODO:
        // NB: Structure parameters of size <= pointer size * 2
        //       are already divided into registers in AnclIR.
        //     Other structure parameters are passed as a struct pointer.

        for (auto* irParam : irFunction->GetParameters()) {
            auto paramIRType = irParam->GetType();
            // TODO: add Target Machine to Alignment
            uint paramSize = ir::Alignment::GetTypeSize(paramIRType);
            uint pointerSize = m_TargetMachine->GetPointerByteSize();
    
            bool isImplicit = irParam->IsImplicit();
            bool isFloat = dynamic_cast<ir::FloatType*>(paramIRType);

            uint vreg = 0;
            if (dynamic_cast<ir::PointerType*>(paramIRType)) {
                vreg = mirFunction->AddParameter(MType::CreatePointer(pointerSize), false, isImplicit);
            } else {
                vreg = mirFunction->AddParameter(MType::CreateScalar(paramSize), isFloat, false);
            }
            linkIRValueWithVReg(irParam, vreg);

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

            // if (dynamic_cast<ir::PointerType*>(paramIRType)) {
            //     uint vreg = mirFunction->AddParameter(MType::CreatePointer(pointerSize), false, isImplicit);
            //     linkIRValueWithVReg(irParam, vreg);
            // } else if (paramSize <= pointerSize) {
            //     uint vreg = mirFunction->AddParameter(MType::CreateScalar(paramSize), isFloat, isImplicit);
            //     linkIRValueWithVReg(irParam, vreg);
            // } else {  // divide into multiple registers
            //     uint regNum = paramSize / pointerSize;
            //     auto paramVRegs = std::vector<uint>{};
            //     paramVRegs.reserve(regNum);
            //     for (uint i = 0; i < regNum; ++i) {
            //         uint vreg = mirFunction->AddParameter(
            //                         MType::CreateScalar(pointerSize), isFloat, isImplicit);
            //         paramVRegs.push_back(vreg);
            //     }
            //     linkIRParamWithVRegs(irParam, paramVRegs);
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

    // std::unordered_map<uint, MBasicBlock::TInstructionIt> m_VRegToInstrDef;
};

}  // namespace gen
