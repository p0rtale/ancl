#include <vector>

#include <Ancl/AnclIR/DataLayout/Alignment.hpp>


namespace ir {

size_t Alignment::Align(size_t minSize, size_t alignment) {
    if (minSize % alignment == 0) {
        return minSize;
    }
    return (minSize + alignment - 1) / alignment * alignment;
}

Alignment::StructLayout Alignment::GetStructLayout(StructType* structType) {
    auto layout = StructLayout{
        .Size = 0,
        .Alignment = 8,
    };

    for (auto elemType : structType->GetElementTypes()) {
        size_t elemSize = GetTypeSize(elemType);
        size_t elemAlignment = GetTypeAlignment(elemType);

        layout.Size = Align(layout.Size, elemAlignment);
        layout.Offsets.push_back(layout.Size);
        layout.Size += elemSize;
        layout.Alignment = std::max(layout.Alignment, elemAlignment);
    }

    if (layout.Size % layout.Alignment != 0) {
        layout.Size = Align(layout.Size, layout.Alignment);
    }

    return layout;
}

size_t GetTypeSize(StructType* structType) {
    auto layout = Alignment::GetStructLayout(structType);
    return layout.Size;
}

size_t GetTypeSize(FloatType* floatType) {
    switch (floatType->GetKind()) {
    case FloatType::Kind::kFloat:
        return 4;
    case FloatType::Kind::kDouble:
        return 8;
    case FloatType::Kind::kLongDouble:
        return 16;
    }

    // TODO: handle error
    return 0;
}

size_t Alignment::GetTypeSize(Type* type) {
    if (auto labelType = dynamic_cast<LabelType*>(type)) {
        return 8;
    }
    if (auto intType = dynamic_cast<IntType*>(type)) {
        return intType->GetBytesNumber();
    }
    if (auto floatType = dynamic_cast<FloatType*>(type)) {
        return GetTypeSize(floatType);
    }
    if (auto pointerType = dynamic_cast<PointerType*>(type)) {
        return 8;
    }
    if (auto arrayType = dynamic_cast<ArrayType*>(type)) {
        return arrayType->GetSize() * GetTypeSize(arrayType->GetSubType());
    }
    if (auto structType = dynamic_cast<StructType*>(type)) {
        return GetTypeSize(structType);
    }

    // TODO: handle error
    return 0;
}

size_t GetTypeAlignment(StructType* structType) {
    auto layout = Alignment::GetStructLayout(structType);
    return layout.Alignment;
}

size_t GetTypeAlignment(FloatType* floatType) {
    switch (floatType->GetKind()) {
    case FloatType::Kind::kFloat:
        return 4;
    case FloatType::Kind::kDouble:
        return 8;
    case FloatType::Kind::kLongDouble:
        return 16;
    }

    // TODO: handle error
    return 0;
}

size_t Alignment::GetTypeAlignment(Type* type) {
    if (auto labelType = dynamic_cast<LabelType*>(type)) {
        return 8;
    }
    if (auto intType = dynamic_cast<IntType*>(type)) {
        return intType->GetBytesNumber();
    }
    if (auto floatType = dynamic_cast<FloatType*>(type)) {
        return GetTypeAlignment(floatType);
    }
    if (auto pointerType = dynamic_cast<PointerType*>(type)) {
        return 8;
    }
    if (auto arrayType = dynamic_cast<ArrayType*>(type)) {
        return GetTypeAlignment(arrayType->GetSubType());
    }
    if (auto structType = dynamic_cast<StructType*>(type)) {
        return GetTypeAlignment(structType);
    }

    // TODO: handle error
    return 0;
}

}  // namespace ir
