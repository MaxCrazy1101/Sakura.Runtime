#pragma once
#include "SkrContainers/span.hpp"
#include "SkrRT/rttr/type_desc.hpp"
#include "SkrGuid/guid.hpp"

namespace skr::rttr
{
struct Type;
struct TypeLoader;
struct GenericTypeLoader;

// type register (loader)
SKR_RUNTIME_API void register_type_loader(const GUID& guid, TypeLoader* loader);
SKR_RUNTIME_API void unregister_type_loader(const GUID& guid);

// generic type loader
SKR_RUNTIME_API void register_generic_type_loader(const GUID& generic_guid, GenericTypeLoader* loader);
SKR_RUNTIME_API void unregister_generic_type_loader(const GUID& generic_guid);

// get type (after register)
SKR_RUNTIME_API Type* get_type_from_guid(const GUID& guid);
SKR_RUNTIME_API Type* get_type_from_type_desc(span<TypeDesc> type_desc);

} // namespace skr::rttr