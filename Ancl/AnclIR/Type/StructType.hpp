#pragma once

#include <vector>
#include <string>

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class StructType: public Type {
public:
    StructType(const std::vector<Type*>& elementTypes)
        : m_ElementTypes(elementTypes) {}

    void SetName(const std::string& name) {
        m_Name = name;
    }

    std::string GetName() const {
        return m_Name;
    }

    std::vector<Type*> GetElementTypes() const {
        return m_ElementTypes;
    }

    size_t GetElementsNumber() const {
        return m_ElementTypes.size();
    }

private:
    std::string m_Name;
    std::vector<Type*> m_ElementTypes;
};

}  // namespace ir
