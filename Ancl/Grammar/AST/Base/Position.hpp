#pragma once

#include <string>


class Position {
public:
    Position() = default;

    Position(std::string filename, size_t line = 0, size_t column = 0)
        : m_FileName(std::move(filename)), m_Line(line), m_Column(column) {}

    std::string GetFileName() const {
        return m_FileName;
    }

    size_t GetLine() const {
        return m_Line;
    }

    size_t GetColumn() const {
        return m_Column;
    }

private:
    std::string m_FileName;
    size_t m_Line = 0;
    size_t m_Column = 0;
};
