#pragma once
#include "json/reader_fwd.h"

#if defined(__cplusplus)
    #include <EASTL/vector.h>
    #include "simdjson.h"
    #include "containers/hashmap.hpp"
    #include "containers/variant.hpp"
    #include "containers/string.hpp"
    #include "type/type_id.hpp"
    #include "platform/guid.hpp"
    #include "type/type_helper.hpp"

// forward declaration for resources
struct skr_resource_handle_t;
namespace skr::resource
{
template <class T>
struct TResourceHandle;
}
// end forward declaration for resources
// utils for codegen
namespace skr
{
namespace json
{
struct error_code_info {
    error_code code;
    const char* message; // do not use a fancy std::string where a simple C string will do (no alloc, no destructor)
};
RUNTIME_API const char* error_message(error_code err) noexcept;
RUNTIME_API void set_error_message(error_code err) noexcept;

template <class T>
error_code Read(simdjson::ondemand::value&& json, T& value);
template <>
struct RUNTIME_API ReadHelper<bool> {
    static error_code Read(simdjson::ondemand::value&& json, bool& value);
};

template <>
struct RUNTIME_API ReadHelper<uint32_t> {
    static error_code Read(simdjson::ondemand::value&& json, uint32_t& value);
};

template <>
struct RUNTIME_API ReadHelper<uint64_t> {
    static error_code Read(simdjson::ondemand::value&& json, uint64_t& value);
};

template <>
struct RUNTIME_API ReadHelper<int32_t> {
    static error_code Read(simdjson::ondemand::value&& json, int32_t& value);
};

template <>
struct RUNTIME_API ReadHelper<int64_t> {
    static error_code Read(simdjson::ondemand::value&& json, int64_t& value);
};

template <>
struct RUNTIME_API ReadHelper<float> {
    static error_code Read(simdjson::ondemand::value&& json, float& value);
};

template <>
struct RUNTIME_API ReadHelper<double> {
    static error_code Read(simdjson::ondemand::value&& json, double& value);
};

template <>
struct RUNTIME_API ReadHelper<skr::string> {
    static error_code Read(simdjson::ondemand::value&& json, skr::string& value);
};

template <>
struct RUNTIME_API ReadHelper<skr_guid_t> {
    static error_code Read(simdjson::ondemand::value&& json, skr_guid_t& value);
};

template <>
struct RUNTIME_API ReadHelper<skr_md5_t> {
    static error_code Read(simdjson::ondemand::value&& json, skr_md5_t& md5);
};

template <>
struct RUNTIME_API ReadHelper<skr_float2_t> {
    static error_code Read(simdjson::ondemand::value&& json, skr_float2_t& value);
};

template <>
struct RUNTIME_API ReadHelper<skr_float3_t> {
    static error_code Read(simdjson::ondemand::value&& json, skr_float3_t& value);
};

template <>
struct RUNTIME_API ReadHelper<skr_float4_t> {
    static error_code Read(simdjson::ondemand::value&& json, skr_float4_t& value);
};

template <>
struct RUNTIME_API ReadHelper<skr_float4x4_t> {
    static error_code Read(simdjson::ondemand::value&& json, skr_float4x4_t& value);
};

template <>
struct RUNTIME_API ReadHelper<skr_rotator_t> {
    static error_code Read(simdjson::ondemand::value&& json, skr_rotator_t& value);
};

template <>
struct RUNTIME_API ReadHelper<skr_quaternion_t> {
    static error_code Read(simdjson::ondemand::value&& json, skr_quaternion_t& value);
};

template <>
struct RUNTIME_API ReadHelper<skr_resource_handle_t> {
    static error_code Read(simdjson::ondemand::value&& json, skr_resource_handle_t& value);
};

template <class K, class V, class Hash, class Eq>
struct ReadHelper<skr::flat_hash_map<K, V, Hash, Eq>> {
    static error_code Read(simdjson::ondemand::value&& json, skr::flat_hash_map<K, V, Hash, Eq>& map)
    {
        auto object = json.get_object();
        if (object.error() != simdjson::SUCCESS)
            return (error_code)object.error();
        for (auto pair : object.value_unsafe())
        {
            auto key = pair.key();
            if (key.error() != simdjson::SUCCESS)
                return (error_code)key.error();
            auto value = pair.value();
            if (value.error() != simdjson::SUCCESS)
                return (error_code)value.error();
            V v;
            error_code error = skr::json::Read<V>(std::move(value).value_unsafe(), v);
            if (error != SUCCESS)
                return error;

            const char* key_str = key.value_unsafe().raw();
            if constexpr (std::is_same_v<std::decay_t<K>, skr::string>)
            {
                map.insert(std::make_pair(K{key_str}, std::move(v)));
            }
            else
            {
                map.insert(std::make_pair(K::from_string(key_str), std::move(v)));
            }
        }
        return SUCCESS;
    }
};

template <class V, class Allocator>
struct ReadHelper<eastl::vector<V, Allocator>> {
    static error_code Read(simdjson::ondemand::value&& json, eastl::vector<V, Allocator>& vec)
    {
        auto array = json.get_array();
        if (array.error() != simdjson::SUCCESS)
            return (error_code)array.error();
        vec.reserve(array.value_unsafe().count_elements().value_unsafe());
        for (auto value : array.value_unsafe())
        {
            if (value.error() != simdjson::SUCCESS)
                return (error_code)value.error();
            V v;
            error_code error = skr::json::Read<V>(std::move(value).value_unsafe(), v);
            if (error != SUCCESS)
                return error;
            vec.push_back(std::move(v));
        }
        return SUCCESS;
    }
};

template <class... Ts>
struct ReadHelper<skr::variant<Ts...>> {
    template <class T>
    static error_code ReadByIndex(simdjson::ondemand::value&& json, skr::variant<Ts...>& value, skr_guid_t index)
    {
        if (index == skr::type::type_id<T>::get())
        {
            T t;
            error_code ret = skr::json::Read(std::move(json), t);
            if (ret != error_code::SUCCESS)
                return ret;
            value = std::move(t);
            return error_code::SUCCESS;
        }
        return error_code::VARIANT_ERROR;
    }

    static error_code Read(simdjson::ondemand::value&& json, skr::variant<Ts...>& value)
    {
        auto object = json.get_object();
        if (object.error() != simdjson::SUCCESS)
            return (error_code)object.error();
        auto type = object.value_unsafe()["type"];
        if (type.error() != simdjson::SUCCESS)
            return (error_code)type.error();
        skr_guid_t index;
        error_code ret = skr::json::Read<skr_guid_t>(std::move(type).value_unsafe(), index);
        if (ret != error_code::SUCCESS)
            return ret;
        auto data = object.value_unsafe()["value"];
        if (data.error() != simdjson::SUCCESS)
            return (error_code)data.error();
        (void)(((ret = ReadByIndex<Ts>(std::move(data).value_unsafe(), value, index)) != error_code::SUCCESS) && ...);
        return ret;
    }
};

template <class T>
struct ReadHelper<TEnumAsByte<T>>
{
    static error_code Read(simdjson::ondemand::value&& json, TEnumAsByte<T>& value)
    {
        return skr::json::Read<typename TEnumAsByte<T>::UT>(std::move(json), value.as_byte());
    }
};

template <class T>
struct ReadHelper<skr::resource::TResourceHandle<T>> {
    static error_code Read(simdjson::ondemand::value&& json, skr::resource::TResourceHandle<T>& handle)
    {
        return skr::json::Read<skr_resource_handle_t>(std::move(json), (skr_resource_handle_t&)handle);
    }
};

template <class T>
error_code Read(simdjson::ondemand::value&& json, T& value)
{
    return ReadHelper<T>::Read(std::move(json), value);
}
} // namespace json

template <class K, class V, class Hash, class Eq>
struct SerdeCompleteChecker<json::ReadHelper<skr::flat_hash_map<K, V, Hash, Eq>>> 
: std::bool_constant<is_complete_serde_v<json::ReadHelper<K>> && is_complete_serde_v<json::ReadHelper<V>>> {};

template <class V, class Allocator>
struct SerdeCompleteChecker<json::ReadHelper<eastl::vector<V, Allocator>>>
: std::bool_constant<is_complete_serde_v<json::ReadHelper<V>>> {};

template <class... Ts>
struct SerdeCompleteChecker<json::ReadHelper<skr::variant<Ts...>>>
: std::bool_constant<(is_complete_serde_v<json::ReadHelper<Ts>> && ...)> {};

} // namespace skr
#else
typedef struct skr_json_reader_t skr_json_reader_t;
#endif