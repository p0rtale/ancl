#pragma once

#include <Ancl/SymbolTable/Scope.hpp>
#include <Ancl/Tracker.hpp>


class SymbolTable {
public:
    SymbolTable(): m_GlobalScope(m_Tracker.Allocate<Scope>("global")) {}

    Scope* GetGlobalScope() const {
        return m_GlobalScope;
    }

    Scope* CreateScope(std::string name = "", Scope* parent = nullptr) {
        auto scope = m_Tracker.Allocate<Scope>(std::move(name));
        if (!parent) {
            parent = m_GlobalScope;
        }
        parent->AddChild(scope);
        return scope;
    }

private:
    Tracker<Scope> m_Tracker;

    Scope* m_GlobalScope;
};
