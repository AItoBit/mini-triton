#pragma once

#include <string>
#include <memory>

namespace minitriton {

// -----------------------------------------------------------------------
// Type System
// -----------------------------------------------------------------------

enum class TypeKind {
    I32,
    F32,
    Pointer,
    Tensor,
    Void,
    Bool,
    Unknown
};

class Type {
public:
    TypeKind kind;
    std::shared_ptr<Type> elementType;  // For Pointer and Tensor
    int size;                            // For Tensor (static size), -1 = unknown

    Type() : kind(TypeKind::Unknown), size(-1) {}
    explicit Type(TypeKind k) : kind(k), size(-1) {}

    static std::shared_ptr<Type> makeI32()  { return std::make_shared<Type>(TypeKind::I32); }
    static std::shared_ptr<Type> makeF32()  { return std::make_shared<Type>(TypeKind::F32); }
    static std::shared_ptr<Type> makeBool() { return std::make_shared<Type>(TypeKind::Bool); }
    static std::shared_ptr<Type> makeVoid() { return std::make_shared<Type>(TypeKind::Void); }

    static std::shared_ptr<Type> makePointer(std::shared_ptr<Type> elem) {
        auto t = std::make_shared<Type>(TypeKind::Pointer);
        t->elementType = std::move(elem);
        return t;
    }

    static std::shared_ptr<Type> makeTensor(std::shared_ptr<Type> elem, int sz = -1) {
        auto t = std::make_shared<Type>(TypeKind::Tensor);
        t->elementType = std::move(elem);
        t->size = sz;
        return t;
    }

    bool isNumeric() const { return kind == TypeKind::I32 || kind == TypeKind::F32; }
    bool isPointer() const { return kind == TypeKind::Pointer; }
    bool isTensor()  const { return kind == TypeKind::Tensor; }

    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const { return !(*this == other); }

    std::string toString() const;
};

} // namespace minitriton
