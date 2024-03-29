#pragma once

#include <vector>

#include <Ancl/Grammar/AST/Base/ASTNode.hpp>
#include <Ancl/Grammar/AST/Declaration/TranslationUnit.hpp>

#include <Ancl/Grammar/AST/Tracker.hpp>


using namespace ast;

class Program {
public:
    Program() = default;
    ~Program() = default;

    void SetTranslationUnit(TranslationUnit* translationUnit) {
        m_TranslationUnit = translationUnit;
    }

    TranslationUnit* GetTranslationUnit() const {
        return m_TranslationUnit;
    }

    template<typename T, typename... Args>
    T* CreateAstNode(Args&&... args) {
        return m_AstTracker.Allocate<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T* CreateType(Args&&... args) {
        return m_TypeTracker.Allocate<T>(std::forward<Args>(args)...);
    }

private:
    Tracker<ASTNode> m_AstTracker;
    Tracker<TypeNode> m_TypeTracker;

    TranslationUnit* m_TranslationUnit;
};
