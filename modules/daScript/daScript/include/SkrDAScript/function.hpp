#pragma once
#include "SkrDAScript/env.hpp"

namespace skr {
namespace das {

struct ContextImpl;
struct SKR_DASCRIPT_API FunctionId
{
    friend struct skr::das::Context;
    friend struct skr::das::ContextImpl;
    ~FunctionId() SKR_NOEXCEPT;
    operator bool() const { return ptr; }
protected:
    FunctionId(void* ptr) SKR_NOEXCEPT;
    void* ptr = nullptr;
};

} // namespace das
} // namespace skr