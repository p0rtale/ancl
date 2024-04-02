#pragma once

#include <format>

#include <Ancl/Grammar/AST/Base/Position.hpp>


class Location {
public:
    Location() = default;

    Location(Position begin, Position end)
        : m_Begin(std::move(begin)), m_End(std::move(end)) {}

    Position GetBegin() const {
        return m_Begin;
    }

    Position GetEnd() const {
        return m_End;
    }

    std::string ToStr() const {
        std::string filename = m_Begin.GetFileName();
        return std::format("{}:{}.{}-{}.{}",
                           std::move(filename),
                           m_Begin.GetLine(), m_Begin.GetColumn(),
                           m_End.GetLine(), m_End.GetColumn());
    }

private:
    Position m_Begin;
    Position m_End;
};
