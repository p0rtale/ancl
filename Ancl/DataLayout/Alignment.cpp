#include <vector>

#include <Ancl/DataLayout/Alignment.hpp>
#include <Ancl/Logger/Logger.hpp>


namespace ir {

uint64_t Alignment::Align(uint64_t minSize, uint64_t alignment) {
    if (minSize % alignment == 0) {
        return minSize;
    }
    return (minSize + alignment - 1) / alignment * alignment;
}

Alignment::StructLayout Alignment::GetStructLayout(StructType* structType) {
    StructLayout layout{
        .Size = 0,
        .Alignment = 8,
    };

    for (ir::Type* elemType : structType->GetElementTypes()) {
        uint64_t elemSize = GetTypeSize(elemType);
        uint64_t elemAlignment = GetTypeAlignment(elemType);

        layout.Size = Align(layout.Size, elemAlignment);
        layout.Offsets.push_back(layout.Size);
        layout.Size += elemSize;
        layout.Alignment = std::max(layout.Alignment, elemAlignment);
    }

    // if (layout.Size % layout.Alignment != 0) {
    //     layout.Size = Align(layout.Size, layout.Alignment);
    // }

    return layout;
}

uint64_t GetStructTypeSize(StructType* structType) {
    Alignment::StructLayout layout = Alignment::GetStructLayout(structType);
    return layout.Size;
}

uint64_t GetFloatTypeSize(FloatType* floatType) {
    switch (floatType->GetKind()) {
    case FloatType::Kind::kFloat:
        return 4;
    case FloatType::Kind::kDouble:
        return 8;
    case FloatType::Kind::kLongDouble:
        return 16;
    }

    ANCL_WARN("Invalid float IR type");
    return 0;
}

uint64_t Alignment::GetTypeSize(Type* type) {
    if (auto* labelType = dynamic_cast<LabelType*>(type)) {
        return 8;
    }
    if (auto* intType = dynamic_cast<IntType*>(type)) {
        return intType->GetBytesNumber();
    }
    if (auto* floatType = dynamic_cast<FloatType*>(type)) {
        return GetFloatTypeSize(floatType);
    }
    if (auto* pointerType = dynamic_cast<PointerType*>(type)) {
        return 8;
    }
    if (auto* arrayType = dynamic_cast<ArrayType*>(type)) {
        return arrayType->GetSize() * GetTypeSize(arrayType->GetSubType());
    }
    if (auto* structType = dynamic_cast<StructType*>(type)) {
        return GetStructTypeSize(structType);
    }

    // TODO: Remove the crutch
    if (auto* functionType = dynamic_cast<FunctionType*>(type)) {
        return 1;
    }

    ANCL_WARN("Invalid IR type");
    return 0;
}

uint64_t Alignment::GetTypeBitSize(Type* type) {
    return GetTypeSize(type) * 8;
}

uint64_t GetStructTypeAlignment(StructType* structType) {
    auto layout = Alignment::GetStructLayout(structType);
    return layout.Alignment;
}

uint64_t GetFloatTypeAlignment(FloatType* floatType) {
    switch (floatType->GetKind()) {
    case FloatType::Kind::kFloat:
        return 4;
    case FloatType::Kind::kDouble:
        return 8;
    case FloatType::Kind::kLongDouble:
        return 16;
    }

    ANCL_WARN("Invalid float IR type");
    return 0;
}

uint64_t Alignment::GetTypeAlignment(Type* type) {
    if (auto* labelType = dynamic_cast<LabelType*>(type)) {
        return 8;
    }
    if (auto* intType = dynamic_cast<IntType*>(type)) {
        return intType->GetBytesNumber();
    }
    if (auto* floatType = dynamic_cast<FloatType*>(type)) {
        return GetFloatTypeAlignment(floatType);
    }
    if (auto* pointerType = dynamic_cast<PointerType*>(type)) {
        return 8;
    }

    /*
        AMD64 ABI:

        "An array uses the same alignment as its elements, except that a local or global
        array variable of length at least 16 bytes or a C99 variable-length array variable
        always has alignment of at least 16 bytes."
    */
    if (auto* arrayType = dynamic_cast<ArrayType*>(type)) {
        if (GetTypeSize(arrayType) >= 16) {
            return 16;
        }
        return GetTypeAlignment(arrayType->GetSubType());
    }

    if (auto* structType = dynamic_cast<StructType*>(type)) {
        return GetStructTypeAlignment(structType);
    }

    ANCL_WARN("Invalid IR type");
    return 0;
}

}  // namespace ir
