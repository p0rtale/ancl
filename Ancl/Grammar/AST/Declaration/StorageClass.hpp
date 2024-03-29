#pragma once

namespace ast {

enum class StorageClass {
    kNone = 0,
    kExtern,
    kStatic,
    kAuto,
    kRegister,
};

}  // namespace ast
