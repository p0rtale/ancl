#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

#include <Ancl/Grammar/AST/Declaration/Declaration.hpp>
#include <Ancl/Grammar/AST/Declaration/LabelDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/TagDeclaration.hpp>


namespace ancl {

using namespace ast;

class Scope {
public:
    using Symbol = std::string;
    using TSymbols = std::vector<std::pair<Symbol, Declaration*>>;

    enum class NamespaceType {
        Label = 1,
        Tag,
        Ident,
    };

public:
    Scope() = default; 
    Scope(std::string name): m_Name(std::move(name)) {}

    std::string GetName() const {
        return m_Name;
    }

    Scope* GetParentScope() const {
        return m_ParentScope;
    }

    void AddChild(Scope* child) {
        m_ChildrenScopes.push_back(child);
        child->m_ParentScope = this;
    }

    std::vector<Scope*> GetChildrenScopes() const {
        return m_ChildrenScopes;
    }

    bool IsGlobalScope() const {
        return !m_ParentScope;
    }

    void AddSymbol(NamespaceType type, const Symbol& symbol, Declaration* decl) {
        m_OrderedSymbols.push_back({symbol, decl});
        switch (type) {
        case NamespaceType::Label:
            m_LabelNamespace[symbol] = static_cast<LabelDeclaration*>(decl);
            break;
        case NamespaceType::Tag:
            m_TagNamespace[symbol] = static_cast<TagDeclaration*>(decl);
            break;
        default:
            m_IdentNamespace[symbol] = decl;
        }
    }

    Declaration* GetSymbol(NamespaceType type, const Symbol& symbol) {
        switch (type) {
        case NamespaceType::Label:
            return m_LabelNamespace.at(symbol);
        case NamespaceType::Tag:
            return m_TagNamespace.at(symbol);
        default:
            return m_IdentNamespace.at(symbol);
        }
    }

    const TSymbols& GetSymbols() const {
        return m_OrderedSymbols;
    }

    bool HaveSymbol(NamespaceType type, const Symbol& symbol) const {
        switch (type) {
        case NamespaceType::Label:
            return m_LabelNamespace.contains(symbol);
        case NamespaceType::Tag:
            return m_TagNamespace.contains(symbol);
        default:
            return m_IdentNamespace.contains(symbol);
        }
    }

    std::optional<Declaration*> FindSymbol(NamespaceType type, const Symbol& symbol) {
        auto scope = findScopeBySymbol(type, symbol);
        if (scope->HaveSymbol(type, symbol)) {
            return scope->GetSymbol(type, symbol);
        }
        return std::nullopt;
    }

private:
    Scope* findScopeBySymbol(NamespaceType type, const Symbol& symbol) {
        Scope* scope = this;
        while (!scope->IsGlobalScope() && !scope->HaveSymbol(type, symbol)) {
            auto parent = scope->GetParentScope();
            if (parent) {
                scope = parent;
            } else {
                // TODO: handle error             
            }
        }
        return scope;
    }

    // INamespace& dispatch(Declaration* decl) {
    //     if (std::dynamic_cast<LabelDeclaration*>(decl)) {
    //         return m_LabelNamespace;
    //     }
    //     if (std::dynamic_cast<TagDeclaration*>(decl)) {
    //         return m_TagNamespace;
    //     }
    //     return m_IdentNamespace;
    // }

private:
    std::string m_Name;

    Scope* m_ParentScope;
    std::vector<Scope*> m_ChildrenScopes;

    TSymbols m_OrderedSymbols;

    std::unordered_map<Symbol, LabelDeclaration*> m_LabelNamespace;
    std::unordered_map<Symbol, TagDeclaration*> m_TagNamespace;
    std::unordered_map<Symbol, Declaration*> m_IdentNamespace;
};

}  // namespace ancl
