#pragma once
#include "SkrRT/config.h"
#include <SkrRT/containers_new/string.hpp>

namespace skr::type
{
    template<class T>
    struct EnumToStringTrait;

    template<class T>
    skr::string_view enum_to_string(const T& value)
    {
        return EnumToStringTrait<T>::ToString(value);
    }

    template<class T>
    bool enum_from_string(skr::string_view str, T& value)
    {
        return EnumToStringTrait<T>::FromString(str, value);
    }
}