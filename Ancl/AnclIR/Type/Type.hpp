#pragma once

namespace ir {

class IRProgram;

class Type {
public:
    Type(IRProgram& program): m_Program(program) {}

    virtual ~Type() = default;

    IRProgram& GetProgram() const {
        return m_Program;
    }

private:
    IRProgram& m_Program;
};

}  // namespace ir
