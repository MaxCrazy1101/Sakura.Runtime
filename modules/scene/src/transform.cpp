#include "ecs/array.hpp"
#include "ecs/dual.h"
#include "ecs/callback.hpp"
#include "ecs/dual_config.h"
#include "utils/parallel_for.hpp"
#include "SkrScene/scene.h"

/*
static_assert(alignof(decltype(skr_l2r_comp_t::matrix)) == 16, "Alignment of matrix must be 16");
static_assert(sizeof(skr_l2w_comp_t::matrix) == sizeof(skr_float4x4_t), "Size of matrix must equal to skr_float4x4_t");

static_assert(alignof(decltype(skr_rotation_comp_t::euler)) == alignof(skr::math::Rotator), "Alignment of euler must equal to skr::math::Rotator");
static_assert(sizeof(skr_rotation_comp_t::euler) == sizeof(skr::math::Rotator), "Size of euler must equal to skr::math::Rotator");

static_assert(sizeof(skr_translation_comp_t::value) == sizeof(skr::math::Vector3f), "Size of translation must equal to skr::math::Vector3f");
static_assert(alignof(decltype(skr_translation_comp_t::value)) == alignof(skr::math::Vector3f), "Alignment of translation must equal to skr::math::Vector3f");

static_assert(sizeof(skr_scale_comp_t::value) == sizeof(skr::math::Vector3f), "Size of translation must equal to skr::math::Vector3f");
static_assert(alignof(decltype(skr_scale_comp_t::value)) == alignof(skr::math::Vector3f), "Alignment of scale must equal to skr::math::Vector3f");

template <class T>
static void skr_local_to_x(void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
{
    static_assert(alignof(decltype(T::matrix)) == 16, "Alignment of matrix must be 16");
    static_assert(sizeof(T::matrix) == sizeof(skr_float4x4_t), "Size of matrix must equal to skr_float4x4_t");
    auto translation = (skr_float3_t*)dualV_get_owned_ro_local(view, localTypes[0]);
    auto rotation = (skr_rotator_t*)dualV_get_owned_ro_local(view, localTypes[1]);
    auto scale = (skr_float3_t*)dualV_get_owned_ro_local(view, localTypes[2]);
    auto transform = (skr_float4x4_t*)dualV_get_owned_ro_local(view, localTypes[3]);
    const auto default_translation = skr_float3_t {0.f, 0.f, 0.f};
    const auto default_scale = skr_float3_t {1.f, 1.f, 1.f};
    if (translation)
        forloop (i, 0, view->count)
            *transform = make_transform(translation[i], default_scale);
    if (translation && scale)
        forloop (i, 0, view->count)
            *transform = make_transform(translation[i], scale[i]);
    if (translation && rotation)
        forloop (i, 0, view->count)
            *transform = make_transform(translation[i], default_scale, quaternion_from_rotator(rotation[i]));
    if (translation && scale && rotation)
        forloop (i, 0, view->count)
            *transform = make_transform(translation[i], scale[i], quaternion_from_rotator(rotation[i]));
    if (scale && rotation)
        forloop (i, 0, view->count)
            *transform = make_transform(default_translation, scale[i], quaternion_from_rotator(rotation[i]));
    if (scale)
        forloop (i, 0, view->count)
            *transform = make_transform(default_translation, scale[i]);
    if (rotation)
        forloop (i, 0, view->count)
            *transform = make_transform(default_translation, default_scale, quaternion_from_rotator(rotation[i]));
}

static void skr_relative_to_world_children(skr_children_t* children, skr_l2w_comp_t* parent, dual_storage_t* storage)
{
    auto process = [&](skr_child_comp_t child) {
        dual_chunk_view_t view;
        //TODO: consider dualS_batch?
        dualS_access(storage, child.entity, &view);
        auto relative = (skr_float4x4_t*)dualV_get_owned_ro(&view, dual_id_of<skr_l2r_comp_t>::get());
        if (!relative) 
            return;
        auto transform = (skr_float4x4_t*)dualV_get_owned_ro(&view, dual_id_of<skr_l2w_comp_t>::get());
        if (!transform) 
            return;

        *transform = multiply(*relative, *(skr_float4x4_t*)&parent->matrix);
        auto children = (skr_children_t*)dualV_get_owned_ro(&view, dual_id_of<skr_child_comp_t>::get());
        if (!children) 
            return;
        skr_relative_to_world_children(children, (skr_l2w_comp_t*)transform, storage);
    };
    if (children->size() > 256) // dispatch recursively
    {
        using iter_t = typename skr_children_t::iterator;
        skr::parallel_for(children->begin(), children->end(), 128,
        [&](iter_t begin, iter_t end) {
            for (auto i = begin; i != end; ++i)
                process(*i);
        });
    }
    else
        for (auto child : *children)
            process(child);
}

static void skr_relative_to_world_root(void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
{
    using namespace skr::math;
    auto transform = (skr_l2w_comp_t*)dualV_get_owned_ro_local(view, localTypes[0]);
    auto children = (skr_children_t*)dualV_get_owned_ro_local(view, localTypes[1]);

    forloop (i, 0, view->count)
        skr_relative_to_world_children(&children[i], &transform[i], storage);
}

void skr_transform_setup(dual_storage_t* world, skr_transform_system_t* system)
{
    // for root entities, calculate local to world
    system->localToWorld = dualQ_from_literal(world, "[in]|skr_translation_comp_t,[in]|skr_rotation_comp_t, [in]|skr_scale_comp_t,[out]skr_l2w_comp_t,!skr_parent_comp_t");

    // for node entities, calculate local to parent
    system->localToRelative = dualQ_from_literal(world, "[in]|skr_translation_comp_t,[in]|skr_rotation_comp_t,[in]|skr_scale_comp_t,[out]skr_l2r_comp_t,[has]skr_parent_comp_t");

    // then recursively calculate local to world for node entities
    system->relativeToWorld = dualQ_from_literal(world, "[inout][rand]skr_l2w_comp_t,[in][rand]skr_child_comp_t,[in][rand]?skr_l2r_comp_t,!skr_parent_comp_t");
}

void skr_transform_update(skr_transform_system_t* query)
{
    dualJ_schedule_ecs(query->localToWorld, 256, &skr_local_to_x<skr_l2w_comp_t>, nullptr, nullptr, nullptr, nullptr, nullptr);
    dualJ_schedule_ecs(query->localToRelative, 256, &skr_local_to_x<skr_l2r_comp_t>, nullptr, nullptr, nullptr, nullptr, nullptr);
    dualJ_schedule_ecs(query->relativeToWorld, 128, &skr_relative_to_world_root, nullptr, nullptr, nullptr, nullptr, nullptr);
}
*/