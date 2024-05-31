#include <Ancl/AnclIR/Constant/GlobalValue.hpp>


namespace ir {

GlobalValue::GlobalValue(Type* type, LinkageType linkage)
    : Constant(type), m_Linkage(linkage) {}

GlobalValue::LinkageType GlobalValue::GetLinkage() const {
    return m_Linkage;
}

}  // namespace ir
