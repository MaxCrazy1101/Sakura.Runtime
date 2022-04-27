#pragma once
#include "ecs/dual.h"
#include "constants.hpp"

namespace dual
{
    class type_index_t
    {
        static_assert(sizeof(TIndex) * 8 == 32, "TIndex should be 32 bits");
        union
        {
            TIndex value;
            struct
            {
                TIndex id : 28;
                TIndex pin : 1;
                TIndex buffer : 1;
                TIndex managed : 1;
                TIndex tag : 1;
            };
        };

    public:
        constexpr TIndex index() const noexcept { return id; }
        constexpr bool is_pod() const noexcept { return value == id; }
        constexpr bool is_buffer() const noexcept { return buffer; }
        constexpr bool is_tag() const noexcept { return tag; }
        constexpr bool is_managed() const noexcept { return managed; }
        constexpr bool is_tracked() const noexcept { return pin; }

        constexpr type_index_t() noexcept : value(kInvalidTypeIndex) {}
        constexpr type_index_t(TIndex value) noexcept : value(value) { }
        constexpr type_index_t(TIndex a, bool pin, bool buffer, bool managed, bool tag) noexcept
            : id(a), pin(pin), buffer(buffer), managed(managed), tag(tag) { }

        constexpr operator TIndex() const { return value; }
    };
}