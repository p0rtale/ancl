#pragma once

#include <Ancl/AnclIR/IR.hpp>


namespace ir {

class Alignment {
public:
    struct StructLayout {
        size_t Size;
        size_t Alignment;
        std::vector<size_t> Offsets;
    };

public:
    static size_t Align(size_t minSize, size_t alignment);

    static StructLayout GetStructLayout(StructType* structType);

    // TODO: move to Target Machine
    static constexpr size_t GetPointerTypeSize() {
        return 8;
    }

    static size_t GetTypeSize(Type* type);

    static size_t GetTypeAlignment(Type* type);
};

}  // namespace ir
