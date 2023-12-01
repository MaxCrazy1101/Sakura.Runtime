#pragma once
#include "SkrRT/containers_new/function_ref.hpp" // IWYU pragma: keep

namespace skr::resource
{
    /**
    template<>
    struct ResourceTrait<T>
    {
        static void EnumerateDependecies(const T& resource, skr::FunctionRef<void(const Resource&)> callback);
    }
    */
    template<class T>
    struct ResourceTrait;
}