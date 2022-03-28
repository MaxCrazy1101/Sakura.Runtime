#pragma once
#include "render_graph/frontend/render_graph.hpp"

#define MAX_FRAME_IN_FLIGHT 3

namespace sakura
{
namespace render_graph
{
class RenderGraphFrameExecutor
{
public:
    friend class RenderGraphBackend;

protected:
    RenderGraphFrameExecutor() = default;
    RenderGraphFrameExecutor(CGpuQueueId gfx_queue, CGpuDeviceId deivce)
    {
        CGpuCommandPoolDescriptor pool_desc = {};
        gfx_cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
        CGpuCommandBufferDescriptor cmd_desc = {};
        cmd_desc.is_secondary = false;
        gfx_cmd_buf = cgpu_create_command_buffer(gfx_cmd_pool, &cmd_desc);
    }
    ~RenderGraphFrameExecutor()
    {
        cgpu_free_command_buffer(gfx_cmd_buf);
        cgpu_free_command_pool(gfx_cmd_pool);
    }
    CGpuCommandPoolId gfx_cmd_pool;
    CGpuCommandBufferId gfx_cmd_buf;
};

class RenderGraphBackend : public RenderGraph
{
public:
    RenderGraphBackend(CGpuQueueId gfx_queue, CGpuDeviceId device)
        : gfx_queue(gfx_queue)
        , device(device)
    {
        backend = device->adapter->instance->backend;
        for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
        {
            executors[i] = RenderGraphFrameExecutor(gfx_queue, device);
        }
    }
    void devirtualize(TextureNode* node);
    void devirtualize(PassNode* node);

    virtual uint64_t execute() final;

protected:
    CGpuQueueId gfx_queue;
    CGpuDeviceId device;
    ECGpuBackend backend;
    RenderGraphFrameExecutor executors[MAX_FRAME_IN_FLIGHT];
};
} // namespace render_graph
} // namespace sakura