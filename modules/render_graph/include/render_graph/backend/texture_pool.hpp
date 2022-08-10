#pragma once
#include <EASTL/unordered_map.h>
#include <EASTL/deque.h>
#include "utils/hash.h"
#include "utils/make_zeroed.hpp"
#include "cgpu/api.h"

namespace skr
{
namespace render_graph
{
class TexturePool
{
public:
    struct AllocationMark {
        uint64_t frame_index;
        uint32_t tags;
    };
    struct Key {
        const CGPUDeviceId device;
        const CGPUTextureCreationFlags flags;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t array_size;
        ECGPUFormat format;
        uint32_t mip_levels;
        ECGPUSampleCount sample_count;
        uint32_t sample_quality;
        CGPUResourceTypes descriptors;
        bool is_dedicated = 0;
        operator size_t() const;
        friend class TexturePool;

        Key(CGPUDeviceId device, const CGPUTextureDescriptor& desc);
    };
    friend class RenderGraphBackend;
    void initialize(CGPUDeviceId device);
    void finalize();
    eastl::pair<CGPUTextureId, ECGPUResourceState> allocate(const CGPUTextureDescriptor& desc, AllocationMark mark);
    void deallocate(const CGPUTextureDescriptor& desc, CGPUTextureId texture, ECGPUResourceState final_state, AllocationMark mark);

protected:
    CGPUDeviceId device;
    eastl::unordered_map<Key,
        eastl::deque<eastl::pair<
            eastl::pair<CGPUTextureId, ECGPUResourceState>, AllocationMark>>> textures;
};

FORCEINLINE TexturePool::Key::Key(CGPUDeviceId device, const CGPUTextureDescriptor& desc)
    : device(device)
    , flags(desc.flags)
    , width(desc.width)
    , height(desc.height)
    , depth(desc.depth ? desc.depth : 1)
    , array_size(desc.array_size ? desc.array_size : 1)
    , format(desc.format)
    , mip_levels(desc.mip_levels ? desc.mip_levels : 1)
    , sample_count(desc.sample_count ? desc.sample_count : CGPU_SAMPLE_COUNT_1)
    , sample_quality(desc.sample_quality)
    , descriptors(desc.descriptors)
    , is_dedicated(desc.is_dedicated)
{
}

FORCEINLINE TexturePool::Key::operator size_t() const
{
    return skr_hash(this, sizeof(*this), (size_t)device);
}

FORCEINLINE void TexturePool::initialize(CGPUDeviceId device_)
{
    device = device_;
}

inline void TexturePool::finalize()
{
    for (auto&& queue : textures)
    {
        while (!queue.second.empty())
        {
            cgpu_free_texture(queue.second.front().first.first);
            queue.second.pop_front();
        }
    }
}

inline eastl::pair<CGPUTextureId, ECGPUResourceState> TexturePool::allocate(const CGPUTextureDescriptor& desc, AllocationMark mark)
{
    eastl::pair<CGPUTextureId, ECGPUResourceState> allocated = {
        nullptr, CGPU_RESOURCE_STATE_UNDEFINED
    };
    auto key = make_zeroed<TexturePool::Key>(device, desc);
    if (textures[key].empty())
    {
        auto new_tex = cgpu_create_texture(device, &desc);
        textures[key].push_back({ { new_tex, desc.start_state }, mark });
    }
    textures[key].front().second = mark;
    allocated = textures[key].front().first;
    textures[key].pop_front();
    return allocated;
}

FORCEINLINE void TexturePool::deallocate(const CGPUTextureDescriptor& desc, CGPUTextureId texture, ECGPUResourceState final_state, AllocationMark mark)
{
    auto key = make_zeroed<TexturePool::Key>(device, desc);
    textures[key].push_back({ { texture, final_state }, mark });
}
} // namespace render_graph
} // namespace skr