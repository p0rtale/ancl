#include <Ancl/SymbolTable/Scope.hpp>

#include <cassert>


namespace ast {

Scope::Scope(const std::string& name): m_Name(name) {}

std::string Scope::GetName() const {
    return m_Name;
}

Scope* Scope::GetParentScope() const {
    return m_ParentScope;
}

void Scope::AddChild(Scope* child) {
    m_ChildrenScopes.push_back(child);
    child->m_ParentScope = this;
}

std::vector<Scope*> Scope::GetChildrenScopes() const {
    return m_ChildrenScopes;
}

bool Scope::IsGlobalScope() const {
    return !m_ParentScope;
}

void Scope::AddSymbol(NamespaceType type, const Symbol& symbol, Declaration* decl) {
    m_OrderedSymbols.push_back({symbol, decl});
    UpdateSymbol(type, symbol, decl);
}

void Scope::UpdateSymbol(NamespaceType type, const Symbol& symbol, Declaration* decl) {
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

Declaration* Scope::GetSymbol(NamespaceType type, const Symbol& symbol) {
    switch (type) {
        case NamespaceType::Label:
            return m_LabelNamespace.at(symbol);
        case NamespaceType::Tag:
            return m_TagNamespace.at(symbol);
        default:
            return m_IdentNamespace.at(symbol);
    }
}

const Scope::TSymbols& Scope::GetSymbols() const {
    return m_OrderedSymbols;
}

bool Scope::HasSymbol(NamespaceType type, const Symbol& symbol) const {
    switch (type) {
        case NamespaceType::Label:
            return m_LabelNamespace.contains(symbol);
        case NamespaceType::Tag:
            return m_TagNamespace.contains(symbol);
        default:
            return m_IdentNamespace.contains(symbol);
    }
}

std::optional<Declaration*> Scope::FindSymbol(NamespaceType type, const Symbol& symbol) {
    Scope* scope = findScopeBySymbol(type, symbol);
    if (scope->HasSymbol(type, symbol)) {
        return scope->GetSymbol(type, symbol);
    }
    return std::nullopt;
}

Scope* Scope::findScopeBySymbol(NamespaceType type, const Symbol& symbol) {
    Scope* scope = this;
    while (!scope->IsGlobalScope() && !scope->HasSymbol(type, symbol)) {
        Scope* parent = scope->GetParentScope();
        assert(parent);
        scope = parent;
    }
    return scope;
}

// INamespace& Scope::dispatch(Declaration* decl) {
//     if (std::dynamic_cast<LabelDeclaration*>(decl)) {
//         return m_LabelNamespace;
//     }
//     if (std::dynamic_cast<TagDeclaration*>(decl)) {
//         return m_TagNamespace;
//     }
//     return m_IdentNamespace;
// }

}  // namespace ast
