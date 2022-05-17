#pragma once
#include "platform/configure.h"
#include <EASTL/vector.h>
#include "math/vectormath.hpp"
#include "platform/guid.h"

typedef struct skr_mesh_section_t {
    // attachment?
    // skr_mesh_section_t* parent;
    // eastl::vector<skr_mesh_section_t> children;
    uint32_t index;
    skr::math::Vector3f translation;
    skr::math::Vector3f scale;
    skr::math::Quaternion rotation;
} skr_mesh_section_t;

typedef struct skr_mesh_primitive_t {
    uint32_t index_offset;
    uint32_t first_index;
    uint32_t index_count;
    uint32_t vertex_layout_id;
    skr_guid_t material_inst;
} skr_mesh_primitive_t;

struct reflect attr(
    "guid" : "3f01f94e-bd88-44a0-95e8-94ff74d18fca",
    "serialize" : true
)
skr_vertex_bin_t {
    skr_guid_t blob;
};

struct reflect attr(
    "guid" : "6ac5f946-dd65-4710-8725-ab4273fe13e6",
    "serialize" : true
)
skr_index_bin_t {
    skr_guid_t blob;
};

struct reflect attr(
    "guid" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1",
    "serialize" : true
)
skr_mesh_resource_t
{
    eastl::vector<skr_mesh_section_t> sections;
    eastl::vector<skr_mesh_primitive_t> primitives;
    eastl::vector<struct skr_vertex_bin_t> vertex_buffers;
    struct skr_index_bin_t index_buffer;
    uint32_t index_stride;
};
typedef struct skr_mesh_resource_t skr_mesh_resource_t;