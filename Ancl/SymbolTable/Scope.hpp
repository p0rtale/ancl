#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <Ancl/Grammar/AST/Declaration/Declaration.hpp>
#include <Ancl/Grammar/AST/Declaration/LabelDeclaration.hpp>
#include <Ancl/Grammar/AST/Declaration/TagDeclaration.hpp>


namespace ast {

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
    Scope(const std::string& name);

    std::string GetName() const;

    Scope* GetParentScope() const;

    void AddChild(Scope* child);

    std::vector<Scope*> GetChildrenScopes() const;

    bool IsGlobalScope() const;

    void AddSymbol(NamespaceType type, const Symbol& symbol, Declaration* decl);

    void UpdateSymbol(NamespaceType type, const Symbol& symbol, Declaration* decl);

    Declaration* GetSymbol(NamespaceType type, const Symbol& symbol);

    const TSymbols& GetSymbols() const;

    bool HasSymbol(NamespaceType type, const Symbol& symbol) const;

    std::optional<Declaration*> FindSymbol(NamespaceType type, const Symbol& symbol);

private:
    Scope* findScopeBySymbol(NamespaceType type, const Symbol& symbol);

private:
    std::string m_Name;

    Scope* m_ParentScope = nullptr;
    std::vector<Scope*> m_ChildrenScopes;

    TSymbols m_OrderedSymbols;

    std::unordered_map<Symbol, LabelDeclaration*> m_LabelNamespace;
    std::unordered_map<Symbol, TagDeclaration*> m_TagNamespace;
    std::unordered_map<Symbol, Declaration*> m_IdentNamespace;
};

}  // namespace ast
