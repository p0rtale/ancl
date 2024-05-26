#pragma once

#include <string>
#include <fstream>
#include <format>

#include <Ancl/AnclIR/IRProgram.hpp>
#include <Ancl/AnclIR/IR.hpp>


namespace ir {

class IREmitter {
public:
    IREmitter(const std::string& filename): m_OutputStream(filename) {}

    void Emit(const IRProgram& program) {
        for (const Function* function : program.GetFunctions()) {
            std::string signature = getFunctionSignatureString(function);
            if (!function->IsDeclaration()) {
                m_OutputStream << signature << " {\n";

                std::vector<BasicBlock*> basicBlocks = function->GetBasicBlocks();
                for (size_t i = 0; i < basicBlocks.size(); ++i) {
                    emitBasicBlock(basicBlocks[i]);

                    if (i + 1 < basicBlocks.size()) {
                        m_OutputStream << "\n";
                    }
                }

                m_OutputStream << "}\n\n";
            } else {
                m_OutputStream << signature << "\n\n";
            }
        }
    }

private:
    std::string getValueString(const Value* value) {
        if (const auto* intConstant = dynamic_cast<const IntConstant*>(value)) {
            IntValue intValue = intConstant->GetValue();
            if (intValue.IsSigned()) {
                return std::to_string(intValue.GetSignedValue());
            }
            return std::to_string(intValue.GetUnsignedValue());
        }

        if (const auto* floatConstant = dynamic_cast<const FloatConstant*>(value)) {
            FloatValue floatValue = floatConstant->GetValue();
            return std::to_string(floatValue.GetValue());
        }

        if (const auto* globalValue = dynamic_cast<const GlobalValue*>(value)) {
            return "@" + globalValue->GetName();
        }

        if (const auto* instrValue = dynamic_cast<const Instruction*>(value)) {
            return "%" + instrValue->GetName();
        }

        return value->GetName();
    }

    void emitInstruction(const Instruction* instruction) {
        // TODO: Simplify with operand interface

        if (const auto* alloca = dynamic_cast<const AllocaInstruction*>(instruction)) {
            m_OutputStream << std::format("alloca {} '{}'", getValueString(alloca),
                                          getTypeString(alloca->GetType()));
        } else if (const auto* binary = dynamic_cast<const BinaryInstruction*>(instruction)) {
            std::string instrName = binary->GetOpTypeStr();

            std::string resultName = getValueString(binary);
            std::string typeString = getTypeString(binary->GetType());

            Value* leftValue = binary->GetLeftOperand();
            std::string leftName = getValueString(leftValue);

            Value* rightValue = binary->GetRightOperand();
            std::string rightName = getValueString(rightValue);

            m_OutputStream << std::format("{} {}, {}, {} '{}'",
                                          instrName, resultName, leftName, rightName, typeString);
        } else if (const auto* branch = dynamic_cast<const BranchInstruction*>(instruction)) {
            BasicBlock* trueBasicBlock = branch->GetTrueBasicBlock();
            std::string trueName = getValueString(trueBasicBlock);
            if (branch->IsUnconditional()) {
                m_OutputStream << std::format("jump {}", trueName);
            } else {
                Value* condition = branch->GetCondition();
                std::string condName = getValueString(condition);
                std::string typeString = getTypeString(condition->GetType());

                BasicBlock* falseBasicBlock = branch->GetFalseBasicBlock();
                std::string falseName = getValueString(falseBasicBlock);

                m_OutputStream << std::format("branch {} '{}': {}, {}",
                                              condName, typeString, trueName, falseName);
            }
        } else if (const auto* call = dynamic_cast<const CallInstruction*>(instruction)) {
            std::string callName = getValueString(call);

            Function* callee = call->GetCallee();
            std::string calleeName = getValueString(callee);

            std::string retTypeString = getTypeString(call->GetType());

            // TODO: String stream
            std::string argsString;
            for (ir::Value* argument : call->GetArguments()) {
                std::string argName = getValueString(argument);
                std::string argTypeString = getTypeString(argument->GetType());
                argsString += std::format("{} '{}', ", argName, argTypeString);
            }
            if (call->HasArguments()) {
                argsString.pop_back();
                argsString.pop_back();
            }

            m_OutputStream << std::format("call {} '{}' {}({})",
                                          callName, retTypeString, calleeName, argsString);
        } else if (const auto* compare = dynamic_cast<const CompareInstruction*>(instruction)) {
            std::string instrName = compare->GetOpTypeStr();

            std::string resultName = getValueString(compare);
            std::string cmpTypeString = getTypeString(compare->GetType());

            Value* leftValue = compare->GetLeftOperand();
            std::string leftName = getValueString(leftValue);

            Value* rightValue = compare->GetRightOperand();
            std::string rightName = getValueString(rightValue);

            std::string operandTypeString = getTypeString(leftValue->GetType());

            m_OutputStream << std::format("{} {} '{}', {}, {} '{}'",
                                          instrName, resultName, cmpTypeString,
                                          leftName, rightName, operandTypeString);
        } else if (const auto* cast = dynamic_cast<const CastInstruction*>(instruction)) {
            std::string instrName = cast->GetOpTypeStr();

            std::string resultName = getValueString(cast);
            std::string typeString = getTypeString(cast->GetType());

            Value* fromValue = cast->GetFromOperand();
            std::string fromName = getValueString(fromValue);
            std::string fromTypeString = getTypeString(fromValue->GetType());

            m_OutputStream << std::format("{} {} '{}', {} '{}'",
                                          instrName, resultName, typeString, fromName, fromTypeString);
        } else if (const auto* load = dynamic_cast<const LoadInstruction*>(instruction)) {
            std::string loadName = getValueString(load);
            std::string loadTypeString = getTypeString(load->GetType());

            Value* ptrOperand = load->GetPtrOperand();
            std::string operandName = getValueString(ptrOperand);
            std::string operandTypeString = getTypeString(ptrOperand->GetType());

            m_OutputStream << std::format("load {} '{}', {} '{}'", loadName, loadTypeString,
                                                                   operandName, operandTypeString);
        } else if (const auto* store = dynamic_cast<const StoreInstruction*>(instruction)) {
            Value* valueOperand = store->GetValueOperand();
            std::string valueName = getValueString(valueOperand);
            std::string valueTypeString = getTypeString(valueOperand->GetType());

            Value* addressOperand = store->GetAddressOperand();
            std::string addressName = getValueString(addressOperand);
            std::string addressTypeString = getTypeString(addressOperand->GetType());

            m_OutputStream << std::format("store {} '{}', {} '{}'", valueName, valueTypeString,
                                                                    addressName, addressTypeString);
        } else if (const auto* phi = dynamic_cast<const PhiInstruction*>(instruction)) {
            // name = "phi";
        } else if (const auto* ret = dynamic_cast<const ReturnInstruction*>(instruction)) {
            Type* returnType = ret->GetType();
            std::string returnTypeString = getTypeString(returnType);

            if (ret->HasReturnValue()) {
                Value* returnValue = ret->GetReturnValue();
                std::string returnName = getValueString(returnValue);
                m_OutputStream << std::format("return {} '{}'", returnName, returnTypeString);
            } else {
                m_OutputStream << std::format("return {}", returnTypeString);
            }
        } else if (const auto* member = dynamic_cast<const MemberInstruction*>(instruction)) {
            std::string memberName = getValueString(member);
            std::string memberTypeString = getTypeString(instruction->GetType());

            Value* ptrOperand = member->GetPtrOperand();
            std::string ptrName = getValueString(ptrOperand);
            std::string ptrTypeString = getTypeString(ptrOperand->GetType());

            Value* indexOperand = member->GetIndex();
            std::string indexName = getValueString(indexOperand);
            std::string indexTypeString = getTypeString(indexOperand->GetType());

            m_OutputStream << std::format("member {} '{}', {} '{}', {} '{}'",
                                          memberName, memberTypeString,
                                          ptrName, ptrTypeString,
                                          indexName, indexTypeString);
        }
    }

    void emitBasicBlock(const BasicBlock* basicBlock) {
        m_OutputStream << basicBlock->GetName() << ":\n";
        for (Instruction* instruction : basicBlock->GetInstructions()) {
            m_OutputStream << "\t";
            emitInstruction(instruction);
            m_OutputStream << "\n";
        }
    }

    std::string getFunctionSignatureString(const Function* function) {
        std::string functionName = function->GetName();

        auto* functionType = static_cast<FunctionType*>(function->GetType());
        std::string returnTypeStr = getTypeString(functionType->GetReturnType());

        std::vector<Parameter*> params = function->GetParameters();
        bool isDeclaration = function->IsDeclaration();

        std::string result = std::format("func {}(", functionName);
        std::vector<Type*> paramTypes = functionType->GetParamTypes();
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            if (!isDeclaration) {
                result += params[i]->GetName() + " ";
            }
            result += getTypeString(paramTypes[i]) + ", ";
        }
        if (!paramTypes.empty()) {
            result.pop_back();
            result.pop_back();
        }
        if (functionType->IsVariadic()) {
            result += ", ...";
        }

        result += ") " + returnTypeStr;

        return result;
    }

    std::string getTypeString(const Type* type) {
        if (const auto* voidType = dynamic_cast<const VoidType*>(type)) {
            return "void";
        }

        if (const auto* labelType = dynamic_cast<const LabelType*>(type)) {
            return "label";
        }

        if (const auto* intType = dynamic_cast<const IntType*>(type)) {
            return "i" + std::to_string(intType->GetBytesNumber() * 8);
        }

        if (const auto* floatType = dynamic_cast<const FloatType*>(type)) {
            FloatType::Kind kind = floatType->GetKind();
            if (kind == FloatType::Kind::kFloat) {
                return "fp";
            }
            if (kind == FloatType::Kind::kDouble) {
                return "dp";
            }  
            if (kind == FloatType::Kind::kLongDouble) {
                return "ld";
            }
        }

        if (const auto* ptrType = dynamic_cast<const PointerType*>(type)) {
            return getTypeString(ptrType->GetSubType()) + "*";
        }

        if (const auto* arrayType = dynamic_cast<const ArrayType*>(type)) {
            return getTypeString(arrayType->GetSubType()) + "[" + std::to_string(arrayType->GetSize()) + "]";
        }

        if (const auto* functionType = dynamic_cast<const FunctionType*>(type)) {
            std::string retTypeString = getTypeString(functionType->GetReturnType());

            std::string paramsTypeString;
            for (ir::Type* paramType : functionType->GetParamTypes()) {
                paramsTypeString += getTypeString(paramType) + ", ";
            }
            if (functionType->HasParameters()) {
                paramsTypeString.pop_back();
                paramsTypeString.pop_back();
            }
            if (functionType->IsVariadic()) {
                paramsTypeString += ", ...";
            }

            return std::format("{} ({})", retTypeString, paramsTypeString);
        }  

        if (const auto* structType = dynamic_cast<const StructType*>(type)) {
            std::string structName = structType->GetName();

            std::string result = structName + "{";
            for (const Type* type : structType->GetElementTypes()) {
                result += getTypeString(type) + ", ";
            }
            result.pop_back();
            result.pop_back();
            result += "}";

            return result;
        }

        return "";
    }

private:
    std::ofstream m_OutputStream;
};

}  // namespace ir
