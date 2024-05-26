#pragma once

#include <string>
#include <vector>
#include <fstream>

#include <format>

#include <Ancl/SymbolTable/SymbolTable.hpp>


namespace ast {

class SymbolTreeDotConverter {
public:
    SymbolTreeDotConverter(const SymbolTable& tree, const std::string& filename)
        : m_OutputStream(filename), m_Tree(tree) {}

    void Convert() {
        printWithSpaces(std::format("digraph \"{}\" {{\n", m_GraphName));

        m_CurrentNodeIdent = "ScopeTable";
        spaceLevelUp();
        printWithSpaces(std::format("{} [\n", m_CurrentNodeIdent));

        spaceLevelUp();
        printSpaces();
        m_OutputStream << "style=filled,\n";
        printSpaces();
        m_OutputStream << "fillcolor=\"#181818\",\n";
        printSpaces();
        m_OutputStream << "label=\"ScopeTree\",\n";
        printSpaces();
        m_OutputStream << "fontsize=\"15pt\",\n";
        printSpaces();
        m_OutputStream << "fontcolor=\"#22bd74\",\n";
        spaceLevelDown();

        printWithSpaces("];\n");

        convertScope(m_Tree.GetGlobalScope());

        spaceLevelDown();

        printWithSpaces("}\n");      
    }

private:
    void spaceLevelUp() {
        ++m_SpaceLevel;
    }

    void spaceLevelDown() {
        if (m_SpaceLevel > 0) { 
            --m_SpaceLevel;
        }
    }

    void printWithSpaces(const std::string& string) {
        printSpaces();
        m_OutputStream << string;
    }

    void printSpaces() {
        m_OutputStream << std::string(m_SpaceLevel, '\t');
    }
    
    void printNodeAttributes(const std::string& label, const Scope::TSymbols& symbols) {
        printSpaces();
        m_OutputStream << "shape=record,\n";
        printSpaces();
        m_OutputStream << "style=filled,\n";
        printSpaces();
        m_OutputStream << "fillcolor=\"#181818\",\n";
        printSpaces();
        m_OutputStream << std::format("label=\"{{{}|", label);
        for (auto [symbol, decl] : symbols) {
            if (dynamic_cast<EnumConstDeclaration*>(decl)) {
                m_OutputStream << "EnumConst ";
            } else if (dynamic_cast<EnumDeclaration*>(decl)) {
                m_OutputStream << "EnumDecl ";
            } else if (dynamic_cast<FieldDeclaration*>(decl)) {
                m_OutputStream << "FieldDecl ";
            } else if (dynamic_cast<FunctionDeclaration*>(decl)) {
                m_OutputStream << "FuncDecl ";
            } else if (dynamic_cast<LabelDeclaration*>(decl)) {
                m_OutputStream << "LabelDecl ";
            } else if (dynamic_cast<ParameterDeclaration*>(decl)) {
                m_OutputStream << "ParamDecl ";
            } else if (dynamic_cast<RecordDeclaration*>(decl)) {
                m_OutputStream << "RecordDecl ";
            } else if (dynamic_cast<TypedefDeclaration*>(decl)) {
                m_OutputStream << "TypedefDecl ";
            } else if (dynamic_cast<VariableDeclaration*>(decl)) {
                m_OutputStream << "VarDecl ";
            }

            m_OutputStream << std::format("\\\"{}\\\" ", symbol);
            m_OutputStream << "\\l";

            // if (type->IsFunctionType()) {
            //     m_OutputStream << "Function ";
            // } else if (type->IsPointerType()) {
            //     m_OutputStream << "Pointer ";
            // } else if (type->IsArrayType()) {
            //     m_OutputStream << "Array ";
            // } else if (!type->IsBasicType()) {
            //     using clipl::type::ComplexType;
            //     auto complexType = std::dynamic_pointer_cast<ComplexType>(type);
            //     auto internalType = complexType->GetInternalType();
            //     switch (internalType) {
            //     case ComplexType::InternalType::kStruct:
            //         m_OutputStream << "Tag Struct ";
            //         break;
            //     case ComplexType::InternalType::kUnion:
            //         m_OutputStream << "Tag Union ";
            //         break;
            //     case ComplexType::InternalType::kEnum:
            //         m_OutputStream << "Tag Enum ";
            //         break;
            //     default:
            //         break;
            //     }
            // } else {
            //     using clipl::type::BasicType;
            //     auto basicType = std::dynamic_pointer_cast<type::BasicType>(type);
            //     auto internalType = basicType->GetInternalType();
            //     switch (internalType) {
            //     case BasicType::InternalType::kVoid:
            //         m_OutputStream << "Void ";
            //         break;
            //     case BasicType::InternalType::kInteger:
            //         m_OutputStream << "Integer ";
            //         break;
            //     case BasicType::InternalType::kRealNumber:
            //         m_OutputStream << "RealNumber ";
            //         break;
            //     default:
            //         break;
            //     }
            // }
            // m_OutputStream << std::format("\\\"{}\\\" ", symbol.GetName());
            // if (symbol.IsTypeSymbol()) {
            //     m_OutputStream << "[typename]\\l";
            //     m_TypeSymbols.push_back({symbol, type});
            // } else {
            //     m_OutputStream << "\\l";
            // }
        }
        m_OutputStream << "}\",\n";
        printSpaces();
        m_OutputStream << "fontsize=\"15pt\",\n";
        printSpaces();
        m_OutputStream << "fontcolor=\"#22bd74\",\n";
    }

    void printNode(const std::string& ident, const Scope::TSymbols& symbols) {
        m_CurrentNodeIdent = getId();

        printWithSpaces(std::format("{} [\n", m_CurrentNodeIdent));

        spaceLevelUp();
        printNodeAttributes(ident, symbols);
        spaceLevelDown();

        printWithSpaces("];\n");

        printWithSpaces(std::format("{} -> {};\n", m_PreviousNodeIdent, m_CurrentNodeIdent));
    }

    void convertScope(Scope* scope) {
        m_PreviousNodeIdent = m_CurrentNodeIdent;
        std::string prevNodeIdent = m_PreviousNodeIdent;
        
        std::string scopeName = scope->GetName();
        printNode(scopeName, scope->GetSymbols());

        convertScopes(scope->GetChildrenScopes());   

        m_CurrentNodeIdent = prevNodeIdent;
    }

    void convertScopes(const std::vector<Scope*>& scopes) {
        m_PreviousNodeIdent = m_CurrentNodeIdent;
        std::string prevNodeIdent = m_PreviousNodeIdent;
        for (auto scope : scopes) {
            convertScope(scope);
            m_PreviousNodeIdent = prevNodeIdent;
        }
        m_CurrentNodeIdent = prevNodeIdent;
    }

    std::string getId() {
        return std::to_string(m_IdCounter++);
    }

private:
    std::ofstream m_OutputStream;
    size_t m_SpaceLevel = 0;

    const std::string m_GraphName = "symbol_table";

    std::string m_CurrentNodeIdent;
    std::string m_PreviousNodeIdent;

    size_t m_IdCounter = 0;

    const SymbolTable& m_Tree;
};

}  // namespace ast
