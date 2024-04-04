#pragma once

#include <Ancl/AnclIR/Constant.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>

namespace ir {

class GlobalValue: public Constant {
public:
    enum class LinkageType {
        kStatic, kExtern,
    };

public:
    GlobalValue(Type* type, LinkageType linkage)
        : Constant(type), m_Linkage(linkage) {}

    LinkageType GetLinkage() const {
        return m_Linkage;
    }

private:
    LinkageType m_Linkage = LinkageType::kExtern;
};

}  // namespace ir
