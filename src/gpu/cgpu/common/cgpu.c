#include "cgpu/api.h"
#include "cgpu/flags.h"
#include "common_utils.h"
#include <stdint.h>
#ifdef CGPU_USE_VULKAN
    #include "cgpu/backend/vulkan/cgpu_vulkan.h"
#endif
#ifdef CGPU_USE_D3D12
    #include "cgpu/backend/d3d12/cgpu_d3d12.h"
#endif
#ifdef CGPU_USE_METAL
    #include "cgpu/backend/metal/cgpu_metal.h"
#endif
#ifdef __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC
        #define _MACOS
        #if defined(ARCH_ARM64)
            #define TARGET_APPLE_ARM64
        #endif
    #endif
    #if TARGET_OS_IPHONE
        #define TARGET_IOS
    #endif
    #if TARGET_IPHONE_SIMULATOR
        #define TARGET_IOS_SIMULATOR
    #endif
#elif defined _WIN32 || defined _WIN64
#endif

CGpuInstanceId cgpu_create_instance(const CGpuInstanceDescriptor* desc)
{
    cgpu_assert((desc->backend == CGPU_BACKEND_VULKAN || desc->backend == CGPU_BACKEND_D3D12 || desc->backend == CGPU_BACKEND_METAL) && "CGPU support only vulkan & d3d12 & metal currently!");
    const CGpuProcTable* tbl = CGPU_NULLPTR;
    const CGpuSurfacesProcTable* s_tbl = CGPU_NULLPTR;

    if (desc->backend == CGPU_BACKEND_COUNT)
    {
    }
#ifdef CGPU_USE_VULKAN
    else if (desc->backend == CGPU_BACKEND_VULKAN)
    {
        tbl = CGPU_VulkanProcTable();
        s_tbl = CGPU_VulkanSurfacesProcTable();
    }
#endif
#ifdef CGPU_USE_METAL
    else if (desc->backend == CGPU_BACKEND_METAL)
    {
        tbl = CGPU_MetalProcTable();
        s_tbl = CGPU_MetalSurfacesProcTable();
    }
#endif
#ifdef CGPU_USE_D3D12
    else if (desc->backend == CGPU_BACKEND_D3D12)
    {
        tbl = CGPU_D3D12ProcTable();
        s_tbl = CGPU_D3D12SurfacesProcTable();
    }
#endif
    CGpuInstance* instance = (CGpuInstance*)tbl->create_instance(desc);
    *(bool*)&instance->enable_set_name = desc->enable_set_name;
    instance->backend = desc->backend;
    instance->proc_table = tbl;
    instance->surfaces_table = s_tbl;
    instance->runtime_table = cgpu_create_runtime_table();
    return instance;
}

RUNTIME_API ECGpuBackend cgpu_instance_get_backend(CGpuInstanceId instance)
{
    return instance->backend;
}

RUNTIME_API void cgpu_query_instance_features(CGpuInstanceId instance, struct CGpuInstanceFeatures* features)
{
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    cgpu_assert(instance->proc_table->query_instance_features && "query_instance_features Proc Missing!");

    instance->proc_table->query_instance_features(instance, features);
}

void cgpu_free_instance(CGpuInstanceId instance)
{
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    cgpu_assert(instance->proc_table->free_instance && "free_instance Proc Missing!");

    cgpu_free_runtime_table(instance->runtime_table);
    instance->proc_table->free_instance(instance);
}

void cgpu_enum_adapters(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    cgpu_assert(instance->proc_table->enum_adapters && "enum_adapters Proc Missing!");

    instance->proc_table->enum_adapters(instance, adapters, adapters_num);
    // ++ proc_table_cache
    if (adapters != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < *adapters_num; i++)
        {
            *(const CGpuProcTable**)&adapters[i]->proc_table_cache = instance->proc_table;
            *(CGpuInstanceId*)&adapters[i]->instance = instance;
        }
    }
    // -- proc_table_cache
}

const char* unknownAdapterName = "UNKNOWN";
const struct CGpuAdapterDetail* cgpu_query_adapter_detail(const CGpuAdapterId adapter)
{
    cgpu_assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(adapter->proc_table_cache->query_adapter_detail && "query_adapter_detail Proc Missing!");

    CGpuAdapterDetail* detail = (CGpuAdapterDetail*)adapter->proc_table_cache->query_adapter_detail(adapter);
    return detail;
}

uint32_t cgpu_query_queue_count(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    cgpu_assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(adapter->proc_table_cache->query_queue_count && "query_queue_count Proc Missing!");

    return adapter->proc_table_cache->query_queue_count(adapter, type);
}

CGpuDeviceId cgpu_create_device(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
    cgpu_assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(adapter->proc_table_cache->create_device && "create_device Proc Missing!");

    CGpuDeviceId device = adapter->proc_table_cache->create_device(adapter, desc);
    // ++ proc_table_cache
    if (device != CGPU_NULLPTR)
    {
        *(const CGpuProcTable**)&device->proc_table_cache = adapter->proc_table_cache;
    }
    // -- proc_table_cache
    return device;
}

CGpuFenceId cgpu_create_fence(CGpuDeviceId device)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_fence && "create_fence Proc Missing!");
    CGpuFence* fence = (CGpuFence*)device->proc_table_cache->create_fence(device);
    fence->device = device;
    return fence;
}

void cgpu_wait_fences(const CGpuFenceId* fences, uint32_t fence_count)
{
    if (fences == CGPU_NULLPTR || fence_count <= 0)
    {
        return;
    }
    CGpuFenceId fence = fences[0];
    cgpu_assert(fence != CGPU_NULLPTR && "fatal: call on NULL fence!");
    cgpu_assert(fence->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcWaitFences fn_wait_fences = fence->device->proc_table_cache->wait_fences;
    cgpu_assert(fn_wait_fences && "wait_fences Proc Missing!");
    fn_wait_fences(fences, fence_count);
}

ECGpuFenceStatus cgpu_query_fence_status(CGpuFenceId fence)
{
    cgpu_assert(fence != CGPU_NULLPTR && "fatal: call on NULL fence!");
    cgpu_assert(fence->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcQueryFenceStatus fn_query_fence = fence->device->proc_table_cache->query_fence_status;
    cgpu_assert(fn_query_fence && "query_fence_status Proc Missing!");
    return fn_query_fence(fence);
}

void cgpu_free_fence(CGpuFenceId fence)
{
    cgpu_assert(fence != CGPU_NULLPTR && "fatal: call on NULL fence!");
    cgpu_assert(fence->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeFence fn_free_fence = fence->device->proc_table_cache->free_fence;
    cgpu_assert(fn_free_fence && "free_fence Proc Missing!");
    fn_free_fence(fence);
}

CGpuSemaphoreId cgpu_create_semaphore(CGpuDeviceId device)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_semaphore && "create_semaphore Proc Missing!");
    CGpuSemaphore* semaphore = (CGpuSemaphore*)device->proc_table_cache->create_semaphore(device);
    semaphore->device = device;
    return semaphore;
}

void cgpu_free_semaphore(CGpuSemaphoreId semaphore)
{
    cgpu_assert(semaphore != CGPU_NULLPTR && "fatal: call on NULL semaphore!");
    cgpu_assert(semaphore->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeSemaphore fn_free_semaphore = semaphore->device->proc_table_cache->free_semaphore;
    cgpu_assert(fn_free_semaphore && "free_semaphore Proc Missing!");
    fn_free_semaphore(semaphore);
}

CGpuRootSignatureId cgpu_create_root_signature(CGpuDeviceId device, const struct CGpuRootSignatureDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_root_signature && "create_root_signature Proc Missing!");
    CGpuRootSignature* signature = (CGpuRootSignature*)device->proc_table_cache->create_root_signature(device, desc);
    signature->device = device;
    return signature;
}

void cgpu_free_root_signature(CGpuRootSignatureId signature)
{
    cgpu_assert(signature != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = signature->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_root_signature && "free_root_signature Proc Missing!");
    device->proc_table_cache->free_root_signature(signature);
}

CGpuDescriptorSetId cgpu_create_descriptor_set(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_descriptor_set && "create_descriptor_set Proc Missing!");
    CGpuDescriptorSet* set = (CGpuDescriptorSet*)device->proc_table_cache->create_descriptor_set(device, desc);
    set->root_signature = desc->root_signature;
    set->index = desc->set_index;
    return set;
}

void cgpu_update_descriptor_set(CGpuDescriptorSetId set, const struct CGpuDescriptorData* datas, uint32_t count)
{
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL descriptor set!");
    const CGpuDeviceId device = set->root_signature->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->update_descriptor_set && "update_descriptor_set Proc Missing!");
    device->proc_table_cache->update_descriptor_set(set, datas, count);
}

void cgpu_free_descriptor_set(CGpuDescriptorSetId set)
{
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = set->root_signature->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_descriptor_set && "free_descriptor_set Proc Missing!");
    device->proc_table_cache->free_descriptor_set(set);
}

CGpuComputePipelineId cgpu_create_compute_pipeline(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_compute_pipeline && "create_compute_pipeline Proc Missing!");
    CGpuComputePipeline* pipeline = (CGpuComputePipeline*)device->proc_table_cache->create_compute_pipeline(device, desc);
    pipeline->device = device;
    pipeline->root_signature = desc->root_signature;
    return pipeline;
}

void cgpu_free_compute_pipeline(CGpuComputePipelineId pipeline)
{
    cgpu_assert(pipeline != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = pipeline->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_compute_pipeline && "free_compute_pipeline Proc Missing!");
    device->proc_table_cache->free_compute_pipeline(pipeline);
}

static const CGpuBlendStateDescriptor defaultBlendStateDesc = {
    .src_factors[0] = BLEND_CONST_ONE,
    .dst_factors[0] = BLEND_CONST_ZERO,
    .blend_modes[0] = BLEND_MODE_ADD,
    .src_alpha_factors[0] = BLEND_CONST_ONE,
    .dst_alpha_factors[0] = BLEND_CONST_ZERO,
    .masks[0] = COLOR_MASK_ALL,
    .independent_blend = false
};
static const CGpuRasterizerStateDescriptor defaultRasterStateDesc = {
    .cull_mode = CULL_MODE_BACK,
    .fill_mode = FILL_MODE_SOLID,
    .front_face = FRONT_FACE_CCW,
    .slope_scaled_depth_bias = 0.f,
    .enable_depth_clamp = false,
    .enable_scissor = false,
    .enable_multi_sample = false,
    .depth_bias = 0
};
static const CGpuDepthStateDescriptor defaultDepthStateDesc = {
    .depth_test = false,
    .depth_write = false,
    .stencil_test = false
};
CGpuRenderPipelineId cgpu_create_render_pipeline(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_render_pipeline && "create_render_pipeline Proc Missing!");
    CGpuRenderPipelineDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(CGpuRenderPipelineDescriptor));
    CGpuRenderPipeline* pipeline = CGPU_NULLPTR;
    if (desc->sample_count == 0)
        new_desc.sample_count = 1;
    if (desc->blend_state == CGPU_NULLPTR)
        new_desc.blend_state = &defaultBlendStateDesc;
    if (desc->depth_state == CGPU_NULLPTR)
        new_desc.depth_state = &defaultDepthStateDesc;
    if (desc->rasterizer_state == CGPU_NULLPTR)
        new_desc.rasterizer_state = &defaultRasterStateDesc;
    pipeline = (CGpuRenderPipeline*)device->proc_table_cache->create_render_pipeline(device, &new_desc);
    pipeline->device = device;
    pipeline->root_signature = desc->root_signature;
    return pipeline;
}

void cgpu_free_render_pipeline(CGpuRenderPipelineId pipeline)
{
    cgpu_assert(pipeline != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = pipeline->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_render_pipeline && "free_render_pipeline Proc Missing!");
    device->proc_table_cache->free_render_pipeline(pipeline);
}

CGpuQueryPoolId cgpu_create_query_pool(CGpuDeviceId device, const struct CGpuQueryPoolDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcCreateQueryPool fn_create_query_pool = device->proc_table_cache->create_query_pool;
    cgpu_assert(fn_create_query_pool && "create_query_pool Proc Missing!");
    CGpuQueryPool* query_pool = (CGpuQueryPool*)fn_create_query_pool(device, desc);
    query_pool->device = device;
    return query_pool;
}

void cgpu_free_query_pool(CGpuQueryPoolId pool)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcFreeQueryPool fn_free_query_pool = pool->device->proc_table_cache->free_query_pool;
    cgpu_assert(fn_free_query_pool && "free_query_pool Proc Missing!");
    fn_free_query_pool(pool);
}

void cgpu_free_device(CGpuDeviceId device)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    device->proc_table_cache->free_device(device);
    return;
}

CGpuQueueId cgpu_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    CGpuQueueId created = cgpu_runtime_table_try_get_queue(device, type, index);
    if (created != NULL)
    {
        cgpu_warn("You should not call cgpu_get_queue "
                  "with a specific index & type for multiple times!\n"
                  "       Please get for only once and reuse the handle!\n");
        return created;
    }
    CGpuQueue* queue = (CGpuQueue*)device->proc_table_cache->get_queue(device, type, index);
    queue->index = index;
    queue->type = type;
    queue->device = device;
    cgpu_runtime_table_add_queue(queue, type, index);
    return queue;
}

void cgpu_submit_queue(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc)
{
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL desc!");
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcSubmitQueue submit_queue = queue->device->proc_table_cache->submit_queue;
    cgpu_assert(submit_queue && "submit_queue Proc Missing!");

    submit_queue(queue, desc);
}

void cgpu_queue_present(CGpuQueueId queue, const struct CGpuQueuePresentDescriptor* desc)
{
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL desc!");
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcQueuePresent fn_queue_present = queue->device->proc_table_cache->queue_present;
    cgpu_assert(fn_queue_present && "queue_present Proc Missing!");

    fn_queue_present(queue, desc);
}

void cgpu_wait_queue_idle(CGpuQueueId queue)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcWaitQueueIdle wait_queue_idle = queue->device->proc_table_cache->wait_queue_idle;
    cgpu_assert(wait_queue_idle && "wait_queue_idle Proc Missing!");

    wait_queue_idle(queue);
}

float cgpu_queue_get_timestamp_period_ns(CGpuQueueId queue)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcQueueGetTimestampPeriodNS fn_get_timestamp_period = queue->device->proc_table_cache->queue_get_timestamp_period;
    cgpu_assert(fn_get_timestamp_period && "queue_get_timestamp_period Proc Missing!");

    return fn_get_timestamp_period(queue);
}

void cgpu_free_queue(CGpuQueueId queue)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->free_queue && "free_queue Proc Missing!");

    queue->device->proc_table_cache->free_queue(queue);
    return;
}

RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool(CGpuQueueId queue,
    const CGpuCommandPoolDescriptor* desc)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->create_command_pool && "create_command_pool Proc Missing!");

    CGpuCommandPool* pool = (CGpuCommandPool*)queue->device->proc_table_cache->create_command_pool(queue, desc);
    pool->queue = queue;
    return pool;
}

RUNTIME_API CGpuCommandBufferId cgpu_create_command_buffer(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    const CGpuDeviceId device = pool->queue->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCreateCommandBuffer fn_create_cmd = device->proc_table_cache->create_command_buffer;
    cgpu_assert(fn_create_cmd && "create_command_buffer Proc Missing!");

    CGpuCommandBuffer* cmd = (CGpuCommandBuffer*)fn_create_cmd(pool, desc);
    cmd->pool = pool;
    cmd->device = device;
    return cmd;
}

RUNTIME_API void cgpu_reset_command_pool(CGpuCommandPoolId pool)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(pool->queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(pool->queue->device->proc_table_cache->reset_command_pool && "reset_command_pool Proc Missing!");

    pool->queue->device->proc_table_cache->reset_command_pool(pool);
    return;
}

RUNTIME_API void cgpu_free_command_buffer(CGpuCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    CGpuCommandPoolId pool = cmd->pool;
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    const CGpuDeviceId device = pool->queue->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeCommandBuffer fn_free_cmd = device->proc_table_cache->free_command_buffer;
    cgpu_assert(fn_free_cmd && "free_command_buffer Proc Missing!");

    fn_free_cmd(cmd);
}

RUNTIME_API void cgpu_free_command_pool(CGpuCommandPoolId pool)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(pool->queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(pool->queue->device->proc_table_cache->free_command_pool && "free_command_pool Proc Missing!");

    pool->queue->device->proc_table_cache->free_command_pool(pool);
    return;
}

// CMDs
void cgpu_cmd_begin(CGpuCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBegin fn_cmd_begin = cmd->device->proc_table_cache->cmd_begin;
    cgpu_assert(fn_cmd_begin && "cmd_begin Proc Missing!");
    fn_cmd_begin(cmd);
}

void cgpu_cmd_transfer_buffer_to_buffer(CGpuCommandBufferId cmd, const struct CGpuBufferToBufferTransfer* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == PIPELINE_TYPE_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL cpy_desc!");
    cgpu_assert(desc->src != CGPU_NULLPTR && "fatal: call on NULL cpy_src!");
    cgpu_assert(desc->dst != CGPU_NULLPTR && "fatal: call on NULL cpy_dst!");
    const CGPUProcCmdTransferBufferToBuffer fn_cmd_transfer_buffer_to_buffer = cmd->device->proc_table_cache->cmd_transfer_buffer_to_buffer;
    cgpu_assert(fn_cmd_transfer_buffer_to_buffer && "cmd_transfer_buffer_to_buffer Proc Missing!");
    fn_cmd_transfer_buffer_to_buffer(cmd, desc);
}

void cgpu_cmd_transfer_buffer_to_texture(CGpuCommandBufferId cmd, const struct CGpuBufferToTextureTransfer* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == PIPELINE_TYPE_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL cpy_desc!");
    cgpu_assert(desc->src != CGPU_NULLPTR && "fatal: call on NULL cpy_src!");
    cgpu_assert(desc->dst != CGPU_NULLPTR && "fatal: call on NULL cpy_dst!");
    const CGPUProcCmdTransferBufferToTexture fn_cmd_transfer_buffer_to_texture = cmd->device->proc_table_cache->cmd_transfer_buffer_to_texture;
    cgpu_assert(fn_cmd_transfer_buffer_to_texture && "cmd_transfer_buffer_to_texture Proc Missing!");
    fn_cmd_transfer_buffer_to_texture(cmd, desc);
}

void cgpu_cmd_transfer_texture_to_texture(CGpuCommandBufferId cmd, const struct CGpuTextureToTextureTransfer* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == PIPELINE_TYPE_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL cpy_desc!");
    cgpu_assert(desc->src != CGPU_NULLPTR && "fatal: call on NULL cpy_src!");
    cgpu_assert(desc->dst != CGPU_NULLPTR && "fatal: call on NULL cpy_dst!");
    const CGPUProcCmdTransferTextureToTexture fn_cmd_transfer_texture_to_texture = cmd->device->proc_table_cache->cmd_transfer_texture_to_texture;
    cgpu_assert(fn_cmd_transfer_texture_to_texture && "cmd_transfer_texture_to_texture Proc Missing!");
    fn_cmd_transfer_texture_to_texture(cmd, desc);
}

void cgpu_cmd_resource_barrier(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == PIPELINE_TYPE_NONE && "fatal: can't call resource barriers in render/dispatch passes!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdResourceBarrier fn_cmd_resource_barrier = cmd->device->proc_table_cache->cmd_resource_barrier;
    cgpu_assert(fn_cmd_resource_barrier && "cmd_resource_barrier Proc Missing!");
    fn_cmd_resource_barrier(cmd, desc);
}

void cgpu_cmd_begin_query(CGpuCommandBufferId cmd, CGpuQueryPoolId pool, const struct CGpuQueryDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginQuery fn_cmd_begin_query = cmd->device->proc_table_cache->cmd_begin_query;
    cgpu_assert(fn_cmd_begin_query && "cmd_begin_query Proc Missing!");
    fn_cmd_begin_query(cmd, pool, desc);
}

void cgpu_cmd_end_query(CGpuCommandBufferId cmd, CGpuQueryPoolId pool, const struct CGpuQueryDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdEndQuery fn_cmd_end_query = cmd->device->proc_table_cache->cmd_end_query;
    cgpu_assert(fn_cmd_end_query && "cmd_end_query Proc Missing!");
    fn_cmd_end_query(cmd, pool, desc);
}

void cgpu_cmd_reset_query_pool(CGpuCommandBufferId cmd, CGpuQueryPoolId pool, uint32_t start_query, uint32_t query_count)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcCmdResetQueryPool fn_reset_query_pool = pool->device->proc_table_cache->cmd_reset_query_pool;
    cgpu_assert(fn_reset_query_pool && "reset_query_pool Proc Missing!");
    fn_reset_query_pool(cmd, pool, start_query, query_count);
}

void cgpu_cmd_resolve_query(CGpuCommandBufferId cmd, CGpuQueryPoolId pool, CGpuBufferId readback, uint32_t start_query, uint32_t query_count)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdResolveQuery fn_cmd_resolve_query = cmd->device->proc_table_cache->cmd_resolve_query;
    cgpu_assert(fn_cmd_resolve_query && "cmd_resolve_query Proc Missing!");
    fn_cmd_resolve_query(cmd, pool, readback, start_query, query_count);
}

void cgpu_cmd_end(CGpuCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdEnd fn_cmd_end = cmd->device->proc_table_cache->cmd_end;
    cgpu_assert(fn_cmd_end && "cmd_end Proc Missing!");
    fn_cmd_end(cmd);
}

// Compute CMDs
CGpuComputePassEncoderId cgpu_cmd_begin_compute_pass(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginComputePass fn_begin_compute_pass = cmd->device->proc_table_cache->cmd_begin_compute_pass;
    cgpu_assert(fn_begin_compute_pass && "cmd_begin_compute_pass Proc Missing!");
    CGpuComputePassEncoderId ecd = (CGpuComputePassEncoderId)fn_begin_compute_pass(cmd, desc);
    CGpuCommandBuffer* Cmd = (CGpuCommandBuffer*)cmd;
    Cmd->current_dispatch = PIPELINE_TYPE_COMPUTE;
    return ecd;
}

void cgpu_compute_encoder_bind_descriptor_set(CGpuComputePassEncoderId encoder, CGpuDescriptorSetId set)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL descriptor!");
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderBindDescriptorSet fn_bind_descriptor_set = device->proc_table_cache->compute_encoder_bind_descriptor_set;
    cgpu_assert(fn_bind_descriptor_set && "compute_encoder_bind_descriptor_set Proc Missing!");
    fn_bind_descriptor_set(encoder, set);
}

void cgpu_compute_encoder_push_constants(CGpuComputePassEncoderId encoder, CGpuRootSignatureId rs, const char8_t* name, const void* data)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderPushConstants fn_push_constants = device->proc_table_cache->compute_encoder_push_constants;
    cgpu_assert(fn_push_constants && "compute_encoder_push_constants Proc Missing!");
    fn_push_constants(encoder, rs, name, data);
}

void cgpu_compute_encoder_bind_pipeline(CGpuComputePassEncoderId encoder, CGpuComputePipelineId pipeline)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderBindPipeline fn_compute_bind_pipeline = device->proc_table_cache->compute_encoder_bind_pipeline;
    cgpu_assert(fn_compute_bind_pipeline && "compute_encoder_bind_pipeline Proc Missing!");
    fn_compute_bind_pipeline(encoder, pipeline);
}

void cgpu_compute_encoder_dispatch(CGpuComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderDispatch fn_compute_dispatch = device->proc_table_cache->compute_encoder_dispatch;
    cgpu_assert(fn_compute_dispatch && "compute_encoder_dispatch Proc Missing!");
    fn_compute_dispatch(encoder, X, Y, Z);
}

void cgpu_cmd_end_compute_pass(CGpuCommandBufferId cmd, CGpuComputePassEncoderId encoder)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(cmd->current_dispatch == PIPELINE_TYPE_COMPUTE && "fatal: can't call end command pass on commnd buffer while not dispatching compute!");
    const CGPUProcCmdEndComputePass fn_end_compute_pass = cmd->device->proc_table_cache->cmd_end_compute_pass;
    cgpu_assert(fn_end_compute_pass && "cmd_end_compute_pass Proc Missing!");
    fn_end_compute_pass(cmd, encoder);
    CGpuCommandBuffer* Cmd = (CGpuCommandBuffer*)cmd;
    Cmd->current_dispatch = PIPELINE_TYPE_NONE;
}

// Render CMDs
CGpuRenderPassEncoderId cgpu_cmd_begin_render_pass(CGpuCommandBufferId cmd, const struct CGpuRenderPassDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginRenderPass fn_begin_render_pass = cmd->device->proc_table_cache->cmd_begin_render_pass;
    cgpu_assert(fn_begin_render_pass && "cmd_begin_render_pass Proc Missing!");
    CGpuRenderPassEncoderId ecd = (CGpuRenderPassEncoderId)fn_begin_render_pass(cmd, desc);
    CGpuCommandBuffer* Cmd = (CGpuCommandBuffer*)cmd;
    Cmd->current_dispatch = PIPELINE_TYPE_GRAPHICS;
    return ecd;
}

void cgpu_render_encoder_bind_descriptor_set(CGpuRenderPassEncoderId encoder, CGpuDescriptorSetId set)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL descriptor!");
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderBindDescriptorSet fn_bind_descriptor_set = device->proc_table_cache->render_encoder_bind_descriptor_set;
    cgpu_assert(fn_bind_descriptor_set && "render_encoder_bind_descriptor_set Proc Missing!");
    fn_bind_descriptor_set(encoder, set);
}

void cgpu_render_encoder_bind_vertex_buffers(CGpuRenderPassEncoderId encoder, uint32_t buffer_count,
    const CGpuBufferId* buffers, const uint32_t* strides, const uint32_t* offsets)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    cgpu_assert(buffers != CGPU_NULLPTR && "fatal: call on NULL buffers!");
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRendeEncoderBindVertexBuffers fn_bind_vertex_buffers = device->proc_table_cache->render_encoder_bind_vertex_buffers;
    cgpu_assert(fn_bind_vertex_buffers && "render_encoder_bind_vertex_buffers Proc Missing!");
    fn_bind_vertex_buffers(encoder, buffer_count, buffers, strides, offsets);
}

RUNTIME_API void cgpu_render_encoder_bind_index_buffer(CGpuRenderPassEncoderId encoder, CGpuBufferId buffer, uint32_t index_stride, uint64_t offset)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRendeEncoderBindIndexBuffer fn_bind_index_buffer = device->proc_table_cache->render_encoder_bind_index_buffer;
    cgpu_assert(fn_bind_index_buffer && "render_encoder_bind_index_buffer Proc Missing!");
    fn_bind_index_buffer(encoder, buffer, index_stride, offset);
}

void cgpu_render_encoder_set_viewport(CGpuRenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderSetViewport fn_render_set_viewport = device->proc_table_cache->render_encoder_set_viewport;
    cgpu_assert(fn_render_set_viewport && "render_encoder_set_viewport Proc Missing!");
    fn_render_set_viewport(encoder, x, y, width, height, min_depth, max_depth);
}

void cgpu_render_encoder_set_scissor(CGpuRenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderSetScissor fn_render_set_scissor = device->proc_table_cache->render_encoder_set_scissor;
    cgpu_assert(fn_render_set_scissor && "render_encoder_set_scissor Proc Missing!");
    fn_render_set_scissor(encoder, x, y, width, height);
}

void cgpu_render_encoder_bind_pipeline(CGpuRenderPassEncoderId encoder, CGpuRenderPipelineId pipeline)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderBindPipeline fn_render_bind_pipeline = device->proc_table_cache->render_encoder_bind_pipeline;
    cgpu_assert(fn_render_bind_pipeline && "render_encoder_bind_pipeline Proc Missing!");
    fn_render_bind_pipeline(encoder, pipeline);
}

void cgpu_render_encoder_push_constants(CGpuRenderPassEncoderId encoder, CGpuRootSignatureId rs, const char8_t* name, const void* data)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderPushConstants fn_push_constants = device->proc_table_cache->render_encoder_push_constants;
    cgpu_assert(fn_push_constants && "render_encoder_push_constants Proc Missing!");
    fn_push_constants(encoder, rs, name, data);
}

void cgpu_render_encoder_draw(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderDraw fn_draw = device->proc_table_cache->render_encoder_draw;
    cgpu_assert(fn_draw && "render_encoder_draw Proc Missing!");
    fn_draw(encoder, vertex_count, first_vertex);
}

void cgpu_render_encoder_draw_instanced(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderDrawInstanced fn_draw_instanced = device->proc_table_cache->render_encoder_draw_instanced;
    cgpu_assert(fn_draw_instanced && "render_encoder_draw_instanced Proc Missing!");
    fn_draw_instanced(encoder, vertex_count, first_vertex, instance_count, first_instance);
}

void cgpu_render_encoder_draw_indexed(CGpuRenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderDrawIndexed fn_draw_indexed = device->proc_table_cache->render_encoder_draw_indexed;
    cgpu_assert(fn_draw_indexed && "render_encoder_draw_indexed Proc Missing!");
    fn_draw_indexed(encoder, index_count, first_index, first_vertex);
}

void cgpu_render_encoder_draw_indexed_instanced(CGpuRenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderDrawIndexedInstanced fn_draw_indexed_instanced = device->proc_table_cache->render_encoder_draw_indexed_instanced;
    cgpu_assert(fn_draw_indexed_instanced && "render_encoder_draw_indexed_instanced Proc Missing!");
    fn_draw_indexed_instanced(encoder, index_count, first_index, instance_count, first_instance, first_vertex);
}

void cgpu_cmd_end_render_pass(CGpuCommandBufferId cmd, CGpuRenderPassEncoderId encoder)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(cmd->current_dispatch == PIPELINE_TYPE_GRAPHICS && "fatal: can't call end command pass on commnd buffer while not dispatching graphics!");
    const CGPUProcCmdEndRenderPass fn_end_render_pass = cmd->device->proc_table_cache->cmd_end_render_pass;
    cgpu_assert(fn_end_render_pass && "cmd_end_render_pass Proc Missing!");
    fn_end_render_pass(cmd, encoder);
    CGpuCommandBuffer* Cmd = (CGpuCommandBuffer*)cmd;
    Cmd->current_dispatch = PIPELINE_TYPE_NONE;
}

// Events
void cgpu_cmd_begin_event(CGpuCommandBufferId cmd, const CGpuEventInfo* event)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginEvent fn_begin_event = cmd->device->proc_table_cache->cmd_begin_event;
    fn_begin_event(cmd, event);
}

void cgpu_cmd_set_marker(CGpuCommandBufferId cmd, const CGpuMarkerInfo* marker)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdSetMarker fn_cmd_set_marker = cmd->device->proc_table_cache->cmd_set_marker;
    fn_cmd_set_marker(cmd, marker);
}

void cgpu_cmd_end_event(CGpuCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdEndEvent fn_end_event = cmd->device->proc_table_cache->cmd_end_event;
    fn_end_event(cmd);
}

// Shader APIs
CGpuShaderLibraryId cgpu_create_shader_library(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_shader_library && "create_shader_library Proc Missing!");

    CGPUProcCreateShaderLibrary fn_create_shader_library = device->proc_table_cache->create_shader_library;
    CGpuShaderLibrary* shader = (CGpuShaderLibrary*)fn_create_shader_library(device, desc);
    shader->device = device;
    // handle name string
    const size_t str_len = strlen(desc->name);
    const size_t str_size = str_len + 1;
    shader->name = (char8_t*)cgpu_calloc(1, str_size * sizeof(char8_t));
    memcpy((void*)shader->name, desc->name, str_size);
    return shader;
}

void cgpu_free_shader_library(CGpuShaderLibraryId library)
{
    cgpu_assert(library != CGPU_NULLPTR && "fatal: call on NULL shader library!");
    const CGpuDeviceId device = library->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    // handle name string
    cgpu_free((void*)library->name);

    CGPUProcFreeShaderLibrary fn_free_shader_library = device->proc_table_cache->free_shader_library;
    cgpu_assert(fn_free_shader_library && "free_shader_library Proc Missing!");
    fn_free_shader_library(library);
}

// Buffer APIs
CGpuBufferId cgpu_create_buffer(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_buffer && "create_buffer Proc Missing!");
    CGpuBufferDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(CGpuBufferDescriptor));
    if (desc->flags == 0)
    {
        new_desc.flags |= BCF_NONE;
    }
    CGPUProcCreateBuffer fn_create_buffer = device->proc_table_cache->create_buffer;
    CGpuBuffer* buffer = (CGpuBuffer*)fn_create_buffer(device, &new_desc);
    buffer->device = device;
    return buffer;
}

void cgpu_map_buffer(CGpuBufferId buffer, const struct CGpuBufferRange* range)
{
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGpuDeviceId device = buffer->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->map_buffer && "map_buffer Proc Missing!");

    CGPUProcMapBuffer fn_map_buffer = device->proc_table_cache->map_buffer;
    fn_map_buffer(buffer, range);
}

void cgpu_unmap_buffer(CGpuBufferId buffer)
{
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGpuDeviceId device = buffer->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->unmap_buffer && "unmap_buffer Proc Missing!");

    CGPUProcUnmapBuffer fn_unmap_buffer = device->proc_table_cache->unmap_buffer;
    fn_unmap_buffer(buffer);
}

void cgpu_free_buffer(CGpuBufferId buffer)
{
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGpuDeviceId device = buffer->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeBuffer fn_free_buffer = device->proc_table_cache->free_buffer;
    cgpu_assert(fn_free_buffer && "free_buffer Proc Missing!");
    fn_free_buffer(buffer);
}

// Texture/TextureView APIs
CGpuTextureId cgpu_create_texture(CGpuDeviceId device, const struct CGpuTextureDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_texture && "create_texture Proc Missing!");
    CGpuTextureDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(CGpuTextureDescriptor));
    if (desc->array_size == 0) new_desc.array_size = 1;
    if (desc->mip_levels == 0) new_desc.mip_levels = 1;
    if (desc->depth == 0) new_desc.depth = 1;
    if (desc->sample_count == 0) new_desc.sample_count = 1;
    CGPUProcCreateTexture fn_create_texture = device->proc_table_cache->create_texture;
    CGpuTexture* texture = (CGpuTexture*)fn_create_texture(device, &new_desc);
    texture->device = device;
    texture->sample_count = desc->sample_count;
    return texture;
}

void cgpu_free_texture(CGpuTextureId texture)
{
    cgpu_assert(texture != CGPU_NULLPTR && "fatal: call on NULL texture!");
    const CGpuDeviceId device = texture->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeTexture fn_free_texture = device->proc_table_cache->free_texture;
    cgpu_assert(fn_free_texture && "free_texture Proc Missing!");
    fn_free_texture(texture);
}

CGpuSamplerId cgpu_create_sampler(CGpuDeviceId device, const struct CGpuSamplerDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_sampler && "create_sampler Proc Missing!");
    CGPUProcCreateSampler fn_create_sampler = device->proc_table_cache->create_sampler;
    CGpuSampler* sampler = (CGpuSampler*)fn_create_sampler(device, desc);
    sampler->device = device;
    return sampler;
}

void cgpu_free_sampler(CGpuSamplerId sampler)
{
    cgpu_assert(sampler != CGPU_NULLPTR && "fatal: call on NULL sampler!");
    const CGpuDeviceId device = sampler->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeSampler fn_free_sampler = device->proc_table_cache->free_sampler;
    cgpu_assert(fn_free_sampler && "free_sampler Proc Missing!");
    fn_free_sampler(sampler);
}

CGpuTextureViewId cgpu_create_texture_view(CGpuDeviceId device, const struct CGpuTextureViewDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_texture_view && "create_texture_view Proc Missing!");
    CGpuTextureViewDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(CGpuTextureViewDescriptor));
    if (desc->array_layer_count == 0) new_desc.array_layer_count = 1;
    if (desc->mip_level_count == 0) new_desc.mip_level_count = 1;
    CGPUProcCreateTextureView fn_create_texture_view = device->proc_table_cache->create_texture_view;
    CGpuTextureView* texture_view = (CGpuTextureView*)fn_create_texture_view(device, &new_desc);
    texture_view->device = device;
    texture_view->info = *desc;
    return texture_view;
}

void cgpu_free_texture_view(CGpuTextureViewId render_target)
{
    cgpu_assert(render_target != CGPU_NULLPTR && "fatal: call on NULL render_target!");
    const CGpuDeviceId device = render_target->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeTextureView fn_free_texture_view = device->proc_table_cache->free_texture_view;
    cgpu_assert(fn_free_texture_view && "free_texture_view Proc Missing!");
    fn_free_texture_view(render_target);
}

bool cgpu_try_bind_aliasing_texture(CGpuDeviceId device, const struct CGpuTextureAliasingBindDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcTryBindAliasingTexture fn_try_bind_aliasing = device->proc_table_cache->try_bind_aliasing_texture;
    cgpu_assert(fn_try_bind_aliasing && "try_bind_aliasing_texture Proc Missing!");
    return fn_try_bind_aliasing(device, desc);
}

// SwapChain APIs
CGpuSwapChainId cgpu_create_swapchain(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    if (desc->presentQueues == CGPU_NULLPTR)
    {
        cgpu_assert(desc->presentQueuesCount <= 0 &&
                    "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    }
    else
    {
        cgpu_assert(desc->presentQueuesCount > 0 &&
                    "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    }
    CGpuSwapChain* swapchain = (CGpuSwapChain*)device->proc_table_cache->create_swapchain(device, desc);
    cgpu_assert(swapchain && "fatal cgpu_create_swapchain: NULL swapchain id returned from backend.");
    swapchain->device = device;
    return swapchain;
}

uint32_t cgpu_acquire_next_image(CGpuSwapChainId swapchain, const struct CGpuAcquireNextDescriptor* desc)
{
    cgpu_assert(swapchain != CGPU_NULLPTR && "fatal: call on NULL swapchain!");
    cgpu_assert(swapchain->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(swapchain->device->proc_table_cache->acquire_next_image && "acquire_next_image Proc Missing!");

    return swapchain->device->proc_table_cache->acquire_next_image(swapchain, desc);
}

void cgpu_free_swapchain(CGpuSwapChainId swapchain)
{
    cgpu_assert(swapchain != CGPU_NULLPTR && "fatal: call on NULL swapchain!");
    cgpu_assert(swapchain->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(swapchain->device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    swapchain->device->proc_table_cache->free_swapchain(swapchain);
}

// cgpux helpers
CGpuBufferId cgpux_create_mapped_constant_buffer(CGpuDeviceId device,
    uint64_t size, const char8_t* name, bool device_local_preferred)
{
    DECLARE_ZERO(CGpuBufferDescriptor, buf_desc)
    buf_desc.descriptors = RT_BUFFER;
    buf_desc.size = size;
    buf_desc.name = name;
    const CGpuAdapterDetail* detail = cgpu_query_adapter_detail(device->adapter);
    buf_desc.memory_usage = MEM_USAGE_CPU_TO_GPU;
    buf_desc.flags = BCF_PERSISTENT_MAP_BIT | BCF_HOST_VISIBLE;
    buf_desc.start_state = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (device_local_preferred && detail->support_host_visible_vram)
    {
        buf_desc.memory_usage = MEM_USAGE_GPU_ONLY;
    }
    return cgpu_create_buffer(device, &buf_desc);
}

RUNTIME_API CGpuBufferId cgpux_create_mapped_upload_buffer(CGpuDeviceId device,
    uint64_t size, const char8_t* name)
{
    DECLARE_ZERO(CGpuBufferDescriptor, buf_desc)
    buf_desc.descriptors = RT_NONE;
    buf_desc.size = size;
    buf_desc.name = name;
    buf_desc.memory_usage = MEM_USAGE_CPU_ONLY;
    buf_desc.flags = BCF_PERSISTENT_MAP_BIT;
    buf_desc.start_state = RESOURCE_STATE_COPY_DEST;
    return cgpu_create_buffer(device, &buf_desc);
}

// surfaces
#if defined(_WIN32) || defined(_WIN64)
CGpuSurfaceId cgpu_surface_from_hwnd(CGpuDeviceId device, HWND window)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    cgpu_assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    cgpu_assert(device->adapter->instance->surfaces_table->from_hwnd != CGPU_NULLPTR && "free_instance Proc Missing!");

    return device->adapter->instance->surfaces_table->from_hwnd(device, window);
}
#elif defined(_MACOS)
CGpuSurfaceId cgpu_surface_from_ns_view(CGpuDeviceId device, CGpuNSView* window)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    cgpu_assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    cgpu_assert(device->adapter->instance->surfaces_table->from_ns_view != CGPU_NULLPTR && "free_instance Proc Missing!");

    return device->adapter->instance->surfaces_table->from_ns_view(device, window);
}
#endif

CGpuSurfaceId cgpu_surface_from_native_view(CGpuDeviceId device, void* view)
{
#ifdef SAKURA_TARGET_PLATFORM_MACOS
    return cgpu_surface_from_ns_view(device, (CGpuNSView*)view);
#elif defined(SAKURA_TARGET_PLATFORM_WIN)
    return cgpu_surface_from_hwnd(device, (HWND)view);
#endif
}

void cgpu_free_surface(CGpuDeviceId device, CGpuSurfaceId surface)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    cgpu_assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    cgpu_assert(device->adapter->instance->surfaces_table->free_surface != CGPU_NULLPTR && "free_instance Proc Missing!");

    device->adapter->instance->surfaces_table->free_surface(device, surface);
    return;
}