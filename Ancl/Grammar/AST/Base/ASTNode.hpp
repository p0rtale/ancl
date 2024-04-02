#pragma once

#include <Ancl/Visitor/AstVisitor.hpp>
#include <Ancl/Grammar/AST/Base/Location.hpp>

namespace ast {

class ASTNode {
public:
    virtual ~ASTNode() = default;

    virtual void Accept(AstVisitor& visitor) = 0;

    void SetLocation(Location location) {
        m_Location = std::move(location);
    }

    Location GetLocation() const {
        return m_Location;
    }

private:
    Location m_Location;
};

}  // namespace ast
