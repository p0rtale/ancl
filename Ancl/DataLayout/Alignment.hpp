#pragma once

#include <cstdint>
#include <vector>

#include <Ancl/AnclIR/IR.hpp>


namespace ir {

class Alignment {
public:
    struct StructLayout {
        uint64_t Size;
        uint64_t Alignment;
        std::vector<uint64_t> Offsets;
    };

public:
    static uint64_t Align(uint64_t minSize, uint64_t alignment);

    static StructLayout GetStructLayout(StructType* structType);

    // TODO: move to Target Machine
    static constexpr uint64_t GetPointerTypeSize() {
        return 8;
    }

    static uint64_t GetTypeSize(Type* type);

    static uint64_t GetTypeBitSize(Type* type);

    static uint64_t GetTypeAlignment(Type* type);
};

}  // namespace ir
