#pragma once
#include "SkrBase/containers/array/array_def.hpp"

namespace skr::container
{
template <typename T, typename TS>
using RingBufferDataRef = ArrayDataRef<T, TS>;
}