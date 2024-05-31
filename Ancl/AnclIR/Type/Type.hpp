#pragma once


namespace ir {

class IRProgram;

class Type {
public:
    Type(IRProgram& program);

    virtual ~Type() = default;

    IRProgram& GetProgram() const;

private:
    IRProgram& m_Program;
};

}  // namespace ir
