#pragma once

#include <vector>
#include <string>

#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class StructType: public Type {
public:
    StructType(IRProgram& program, const std::vector<Type*>& elementTypes);

    static StructType* Create(const std::vector<Type*>& elementTypes);

    void SetName(const std::string& name);

    std::string GetName() const;

    void SetElementTypes(const std::vector<Type*>& elementTypes) {
        m_ElementTypes = elementTypes;
    }

    Type* GetElementType(size_t index) const;

    std::vector<Type*> GetElementTypes() const;

    size_t GetElementsNumber() const;

private:
    std::string m_Name;
    std::vector<Type*> m_ElementTypes;
};

}  // namespace ir
