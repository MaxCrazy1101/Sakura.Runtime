#pragma once
#include "SkrRenderer/module.configure.h"
#include "utils/types.h"

struct skr_io_ram_service_t;
struct skr_io_vram_service_t;
struct skr_threaded_service_t;
#ifdef __cplusplus
namespace skr { struct RendererDevice; }
typedef struct skr::RendererDevice SRenderDevice;
namespace skr::resource { template <class T> struct TResourceHandle; }
class SkrRendererModule;
#else
typedef struct SRenderDevice SRenderDevice;
#endif

typedef SRenderDevice* SRenderDeviceId;
typedef skr_guid_t skr_vertex_layout_id;

typedef struct skr_stable_shader_hash_t skr_stable_shader_hash_t;
typedef struct skr_platform_shader_hash_t skr_platform_shader_hash_t;
typedef struct skr_platform_shader_identifier_t skr_platform_shader_identifier_t;
typedef struct skr_multi_shader_resource_t skr_multi_shader_resource_t;
typedef struct skr_shader_collection_resource_t skr_shader_collection_resource_t;

typedef struct skr_shader_option_instance_t skr_shader_option_instance_t;
typedef struct skr_shader_option_t skr_shader_option_t;
typedef struct skr_shader_options_resource_t skr_shader_options_resource_t;

typedef struct skr_material_type_resource_t skr_material_type_resource_t;
typedef struct skr_material_resource_t skr_material_resource_t;
typedef struct skr_material_value_t skr_material_value_t;

typedef struct skr_mesh_buffer_t skr_mesh_buffer_t;
typedef struct skr_mesh_primitive_t skr_mesh_primitive_t;
typedef struct skr_mesh_section_t skr_mesh_section_t;
typedef struct skr_mesh_resource_t skr_mesh_resource_t;
typedef struct skr_mesh_resource_t* skr_mesh_resource_id;

typedef struct skr_shader_map_t skr_shader_map_t;
typedef struct skr_shader_map_t* skr_shader_map_id;
typedef struct skr_shader_map_root_t skr_shader_map_root_t;

typedef struct skr_pso_map_key_t skr_pso_map_key_t;
typedef struct skr_pso_map_key_t* skr_pso_map_key_id;
typedef struct skr_pso_map_t* skr_pso_map_id;
typedef struct skr_pso_map_root_t skr_pso_map_root_t;

#ifdef __cplusplus
    using skr_shader_resource_handle_t = skr::resource::TResourceHandle<skr_multi_shader_resource_t>;
    using skr_material_type_handle_t = skr::resource::TResourceHandle<skr_material_type_resource_t>;
    using skr_shader_collection_handle_t = skr::resource::TResourceHandle<skr_shader_collection_resource_t>;
    enum class ESkrShaderOptionType : uint32_t;
#else
    typedef struct skr_resource_handle_t skr_shader_resource_handle_t;
    typedef struct skr_resource_handle_t skr_material_type_handle_t;
    typedef struct skr_resource_handle_t skr_shader_collection_handle_t;
#endif