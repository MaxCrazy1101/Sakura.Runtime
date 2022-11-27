#include "SkrAnimTool/skin_asset.h"
#include "SkrGLTFTool/gltf_utils.hpp"
#include "SkrAnim/resources/skin_resource.h"
#include "cgltf/cgltf.h"

namespace skd::asset
{
bool SSkinCooker::Cook(SCookContext* ctx)
{
    cgltf_data* rawMesh = ctx->Import<cgltf_data>();
    if (!rawMesh)
    {
        return false;
    }
    SKR_DEFER({ctx->Destroy(rawMesh);});
    //TODO; indexing skin
    cgltf_skin* rawSkin = &rawMesh->skins[0];
    skr_skin_resource_t resource;
    auto blob = skr::make_blob_builder<skr_skin_blob_view_t>();
    blob.name = rawSkin->name;
    blob.joint_remaps.reserve(rawSkin->joints_count);
    for (auto i = 0; i < rawSkin->joints_count; ++i)
        blob.joint_remaps.push_back(rawSkin->joints[i]->name);
    blob.inverse_bind_poses.reserve(rawSkin->joints_count);
    auto buffer_view = rawSkin->inverse_bind_matrices->buffer_view;
    const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
    auto matrix = (cgltf_float*)(buffer_data + buffer_view->offset);
    auto components = cgltf_num_components(cgltf_type_mat4);
    SKR_ASSERT(components == 16);
    for (auto i = 0; i < rawSkin->joints_count; ++i)
    {
        blob.inverse_bind_poses.push_back(skr_float4x4_t{ { { matrix[0], matrix[1], matrix[2], matrix[3] },
        { matrix[4], matrix[5], matrix[6], matrix[7] },
        { matrix[8], matrix[9], matrix[10], matrix[11] },
        { matrix[12], matrix[13], matrix[14], matrix[15] } } });
        matrix += components;
    }
    resource.arena = skr::binary::make_arena<skr_skin_blob_view_t>(resource.blob, blob);
    return ctx->Save(resource);
}
} // namespace skd::asset