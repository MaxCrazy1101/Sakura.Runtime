#pragma once
#include "platform/configure.h"
#include <type_traits>

struct skr_json_writer_t;
typedef char skr_json_writer_char_t;
typedef size_t skr_json_writer_size_t;

#if defined(__cplusplus)

namespace skr
{
namespace json
{
template <class T>
using TParamType = std::conditional_t<std::is_fundamental_v<T> || std::is_enum_v<T>, T, const T&>;

template <class T, class = void>
struct WriteHelper;
} // namespace json
} // namespace skr

#endif // defined(__cplusplus)