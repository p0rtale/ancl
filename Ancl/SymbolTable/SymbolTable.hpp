#pragma once

#include <unordered_map>
#include <fstream>


#include <Ancl/SymbolTable/Scope.hpp>


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

public:
    class Tracker {
    public:
        Tracker() = default;

        ~Tracker() {
            DeallocateAll();
        }

        template<typename T, typename... Args>
        T* Allocate(Args&&... args) {
            static_assert(std::is_base_of_v<Scope, T>, "Argument must be a Scope type");
            T* result = new T(args...);
            m_Allocated.push_back(result);
            return result;
        }

        void DeallocateAll() {
            for (Scope* entry : m_Allocated) {
                delete entry;
            }
            m_Allocated.clear();
        }

    private:
        std::vector<Scope*> m_Allocated;
    };

private:
    Tracker m_Tracker;

    Scope* m_GlobalScope;
};
