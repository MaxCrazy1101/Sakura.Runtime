#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/fwd_types.h"
#include <containers/variant.hpp>
#include "SkrRenderer/resources/shader_resource.hpp"
#include "resource/resource_handle.h"

#ifndef __meta__
#include "SkrRenderer/resources/material_resource.generated.h"
#endif

sreflect_enum_class("guid" : "4003703a-dde4-4f11-93a6-6c460bac6357")
sattr("rtti": true, "serialize": ["json", "bin"])
ESkrMaterialPropertyType : uint32_t
{
    FLOAT,
    DOUBLE,
    TEXTURE,
    BUFFER,
    SAMPLER,
    COUNT
};

sreflect_struct("guid": "46de11b4-6beb-4ab9-b9f8-f5c07ceeb8a5")
sattr("rtti": true, "serialize": ["json", "bin"])
skr_material_value_t
{
    using res_handle_t = skr_resource_handle_t;
    ESkrMaterialPropertyType prop_type;
    skr::string slot_name;
    skr::variant<res_handle_t, float, double> value;
};

sreflect_struct("guid": "7cbbb808-20d9-4bff-b72d-3c23d5b00f2b")
sattr("rtti": true, "serialize": ["json", "bin"])
skr_material_shader_variant
{
    skr_guid_t shader_guid;
    skr_stable_shader_hash_t variant_hash;
};

sreflect_struct("guid" : "83264b35-3fde-4fff-8ee1-89abce2e445b")
sattr("rtti": true, "serialize" : ["json", "bin"])
skr_material_type_resource_t
{
    uint32_t version;
    eastl::vector<skr_material_shader_variant> shader_resources;
    eastl::vector<skr_material_value_t> default_values;
};

sreflect_struct("guid" : "2efad635-b331-4fc6-8c52-2f8ca954823e")
sattr("rtti": true, "serialize" : ["json", "bin"])
skr_material_resource_t
{
    uint32_t material_type_version;
    skr_material_type_handle_t material_type;
    eastl::vector<skr_material_value_t> override_values;
};

namespace skr sreflect
{
namespace resource sreflect
{
struct SKR_RENDERER_API SMaterialTypeFactory : public SResourceFactory {
    virtual ~SMaterialTypeFactory() = default;

    struct Root {
        SRenderDeviceId render_device = nullptr;
    };
    [[nodiscard]] static SMaterialTypeFactory* Create(const Root& root);
    static void Destroy(SMaterialTypeFactory* factory); 
};
} // namespace resource
} // namespace skr