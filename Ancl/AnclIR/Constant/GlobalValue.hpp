#pragma once

#include <Ancl/AnclIR/Constant/Constant.hpp>
#include <Ancl/AnclIR/Type/Type.hpp>


namespace ir {

class GlobalValue: public Constant {
public:
    enum class LinkageType {
        kStatic, kExtern,
    };

public:
    GlobalValue(Type* type, LinkageType linkage);

    LinkageType GetLinkage() const;

private:
    LinkageType m_Linkage = LinkageType::kExtern;
};

}  // namespace ir
