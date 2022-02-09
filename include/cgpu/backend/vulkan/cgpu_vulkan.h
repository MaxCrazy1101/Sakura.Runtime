#pragma once
#include "cgpu/api.h"
#if defined(_WIN32) || defined(_WIN64)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "cgpu/backend/vulkan/volk.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBAL_VkAllocationCallbacks CGPU_NULLPTR
#define MAX_PLANE_COUNT 3

#ifndef VK_USE_VOLK_DEVICE_TABLE
    #define VK_USE_VOLK_DEVICE_TABLE
#endif

RUNTIME_API const CGpuProcTable* CGPU_VulkanProcTable();
RUNTIME_API const CGpuSurfacesProcTable* CGPU_VulkanSurfacesProcTable();

// Instance APIs
RUNTIME_API CGpuInstanceId cgpu_create_instance_vulkan(CGpuInstanceDescriptor const* descriptor);
RUNTIME_API void cgpu_query_instance_features_vulkan(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance_vulkan(CGpuInstanceId instance);

// Adapter APIs
RUNTIME_API void cgpu_enum_adapters_vulkan(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);
RUNTIME_API const CGpuAdapterDetail* cgpu_query_adapter_detail_vulkan(const CGpuAdapterId adapter);
RUNTIME_API uint32_t cgpu_query_queue_count_vulkan(const CGpuAdapterId adapter, const ECGpuQueueType type);

// Device APIs
RUNTIME_API CGpuDeviceId cgpu_create_device_vulkan(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device_vulkan(CGpuDeviceId device);

// API Object APIs
RUNTIME_API CGpuFenceId cgpu_create_fence_vulkan(CGpuDeviceId device);
RUNTIME_API void cgpu_wait_fences_vulkan(const CGpuFenceId* fences, uint32_t fence_count);
ECGpuFenceStatus cgpu_query_fence_status_vulkan(CGpuFenceId fence);
RUNTIME_API void cgpu_free_fence_vulkan(CGpuFenceId fence);
RUNTIME_API CGpuSemaphoreId cgpu_create_semaphore_vulkan(CGpuDeviceId device);
RUNTIME_API void cgpu_free_semaphore_vulkan(CGpuSemaphoreId semaphore);
RUNTIME_API CGpuRootSignatureId cgpu_create_root_signature_vulkan(CGpuDeviceId device, const struct CGpuRootSignatureDescriptor* desc);
RUNTIME_API void cgpu_free_root_signature_vulkan(CGpuRootSignatureId signature);
RUNTIME_API CGpuDescriptorSetId cgpu_create_descriptor_set_vulkan(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc);
RUNTIME_API void cgpu_update_descriptor_set_vulkan(CGpuDescriptorSetId set, const struct CGpuDescriptorData* datas, uint32_t count);
RUNTIME_API void cgpu_free_descriptor_set_vulkan(CGpuDescriptorSetId set);
RUNTIME_API CGpuComputePipelineId cgpu_create_compute_pipeline_vulkan(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc);
RUNTIME_API void cgpu_free_compute_pipeline_vulkan(CGpuComputePipelineId pipeline);
RUNTIME_API CGpuRenderPipelineId cgpu_create_render_pipeline_vulkan(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc);
RUNTIME_API void cgpu_free_render_pipeline_vulkan(CGpuRenderPipelineId pipeline);

// Queue APIs
RUNTIME_API CGpuQueueId cgpu_get_queue_vulkan(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_submit_queue_vulkan(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc);
RUNTIME_API void cgpu_wait_queue_idle_vulkan(CGpuQueueId queue);
RUNTIME_API void cgpu_queue_present_vulkan(CGpuQueueId queue, const struct CGpuQueuePresentDescriptor* desc);
RUNTIME_API void cgpu_free_queue_vulkan(CGpuQueueId queue);

// Command APIs
RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool_vulkan(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc);
RUNTIME_API CGpuCommandBufferId cgpu_create_command_buffer_vulkan(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc);
RUNTIME_API void cgpu_reset_command_pool_vulkan(CGpuCommandPoolId pool);
RUNTIME_API void cgpu_free_command_buffer_vulkan(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_free_command_pool_vulkan(CGpuCommandPoolId pool);

// Shader APIs
RUNTIME_API CGpuShaderLibraryId cgpu_create_shader_library_vulkan(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_library_vulkan(CGpuShaderLibraryId shader_module);

// Buffer APIs
RUNTIME_API CGpuBufferId cgpu_create_buffer_vulkan(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc);
RUNTIME_API void cgpu_map_buffer_vulkan(CGpuBufferId buffer, const struct CGpuBufferRange* range);
RUNTIME_API void cgpu_unmap_buffer_vulkan(CGpuBufferId buffer);
RUNTIME_API void cgpu_free_buffer_vulkan(CGpuBufferId buffer);

// Sampler APIs
RUNTIME_API CGpuSamplerId cgpu_create_sampler_vulkan(CGpuDeviceId device, const struct CGpuSamplerDescriptor* desc);
RUNTIME_API void cgpu_free_sampler_vulkan(CGpuSamplerId sampler);

// Texture/TextureView APIs
RUNTIME_API CGpuTextureId cgpu_create_texture_vulkan(CGpuDeviceId device, const struct CGpuTextureDescriptor* desc);
RUNTIME_API void cgpu_free_texture_vulkan(CGpuTextureId texture);
RUNTIME_API CGpuTextureViewId cgpu_create_texture_view_vulkan(CGpuDeviceId device, const struct CGpuTextureViewDescriptor* desc);
RUNTIME_API void cgpu_free_texture_view_vulkan(CGpuTextureViewId render_target);

// Swapchain APIs
RUNTIME_API CGpuSwapChainId cgpu_create_swapchain_vulkan(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc);
RUNTIME_API uint32_t cgpu_acquire_next_image_vulkan(CGpuSwapChainId swapchain, const struct CGpuAcquireNextDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain_vulkan(CGpuSwapChainId swapchain);

// CMDs
RUNTIME_API void cgpu_cmd_begin_vulkan(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_buffer_vulkan(CGpuCommandBufferId cmd, const struct CGpuBufferToBufferTransfer* desc);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_texture_vulkan(CGpuCommandBufferId cmd, const struct CGpuBufferToTextureTransfer* desc);
RUNTIME_API void cgpu_cmd_resource_barrier_vulkan(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc);
RUNTIME_API void cgpu_cmd_end_vulkan(CGpuCommandBufferId cmd);

// Compute CMDs
RUNTIME_API CGpuComputePassEncoderId cgpu_cmd_begin_compute_pass_vulkan(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc);
RUNTIME_API void cgpu_compute_encoder_bind_descriptor_set_vulkan(CGpuComputePassEncoderId encoder, CGpuDescriptorSetId descriptor);
RUNTIME_API void cgpu_compute_encoder_bind_pipeline_vulkan(CGpuComputePassEncoderId encoder, CGpuComputePipelineId pipeline);
RUNTIME_API void cgpu_compute_encoder_dispatch_vulkan(CGpuComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
RUNTIME_API void cgpu_cmd_end_compute_pass_vulkan(CGpuCommandBufferId cmd, CGpuComputePassEncoderId encoder);

// Render CMDs
RUNTIME_API CGpuRenderPassEncoderId cgpu_cmd_begin_render_pass_vulkan(CGpuCommandBufferId cmd, const struct CGpuRenderPassDescriptor* desc);
RUNTIME_API void cgpu_render_encoder_bind_descriptor_set_vulkan(CGpuRenderPassEncoderId encoder, CGpuDescriptorSetId set);
RUNTIME_API void cgpu_render_encoder_set_viewport_vulkan(CGpuRenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth);
RUNTIME_API void cgpu_render_encoder_set_scissor_vulkan(CGpuRenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
RUNTIME_API void cgpu_render_encoder_bind_pipeline_vulkan(CGpuRenderPassEncoderId encoder, CGpuRenderPipelineId pipeline);
RUNTIME_API void cgpu_render_encoder_bind_vertex_buffers_vulkan(CGpuRenderPassEncoderId encoder, uint32_t buffer_count,
    const CGpuBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
RUNTIME_API void cgpu_render_encoder_bind_index_buffer_vulkan(CGpuRenderPassEncoderId encoder, CGpuBufferId buffer, uint32_t index_stride, uint64_t offset);
RUNTIME_API void cgpu_render_encoder_push_constants_vulkan(CGpuRenderPassEncoderId encoder, CGpuRootSignatureId rs, const char8_t* name, const void* data);
RUNTIME_API void cgpu_render_encoder_draw_vulkan(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex);
RUNTIME_API void cgpu_render_encoder_draw_instanced_vulkan(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance);
RUNTIME_API void cgpu_render_encoder_draw_indexed_vulkan(CGpuRenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
RUNTIME_API void cgpu_render_encoder_draw_indexed_instanced_vulkan(CGpuRenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
RUNTIME_API void cgpu_cmd_end_render_pass_vulkan(CGpuCommandBufferId cmd, CGpuRenderPassEncoderId encoder);

typedef struct CGpuInstance_Vulkan {
    CGpuInstance super;
    VkInstance pVkInstance;
    VkDebugUtilsMessengerEXT pVkDebugUtilsMessenger;
    VkDebugReportCallbackEXT pVkDebugReport;
    struct CGpuAdapter_Vulkan* pVulkanAdapters;
    uint32_t mPhysicalDeviceCount;

    // Layers of Instance
    uint32_t mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char** pLayerNames;
    // Enabled Layers Table
    struct CGpuVkLayersTable* pLayersTable;

    // Extension Properties of Instance
    uint32_t mExtensionsCount;
    const char** pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Extensions Table
    struct CGpuVkExtensionsTable* pExtensionsTable;

    // Some Extension Queries
    uint32_t device_group_creation : 1;
    uint32_t debug_utils : 1;
    uint32_t debug_report : 1;
} CGpuInstance_Vulkan;

typedef struct CGpuAdapter_Vulkan {
    CGpuAdapter super;
    VkPhysicalDevice pPhysicalDevice;
    /// Physical Device Props & Features
    VkPhysicalDeviceProperties2 mPhysicalDeviceProps;
    VkPhysicalDeviceFeatures2 mPhysicalDeviceFeatures;
    VkPhysicalDeviceSubgroupProperties mSubgroupProperties;
    /// Queue Families
    uint32_t mQueueFamiliesCount;
    int64_t mQueueFamilyIndices[QUEUE_TYPE_COUNT];
    struct VkQueueFamilyProperties* pQueueFamilyProperties;

    // Layers of Physical Device
    uint32_t mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char** pLayerNames;
    // Enabled Layers Table
    struct CGpuVkLayersTable* pLayersTable;

    // Extension Properties of Physical Device
    uint32_t mExtensionsCount;
    const char** pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Device Extensions Table
    struct CGpuVkExtensionsTable* pExtensionsTable;

    // Some Extension Queries
    uint32_t debug_marker : 1;
    uint32_t dedicated_allocation : 1;
    uint32_t memory_req2 : 1;
    uint32_t external_memory : 1;
    uint32_t external_memory_win32 : 1;
    uint32_t draw_indirect_count : 1;
    uint32_t amd_draw_indirect_count : 1;
    uint32_t amd_gcn_shader : 1;
    uint32_t descriptor_indexing : 1;
    uint32_t sampler_ycbcr : 1;
    uint32_t nv_diagnostic_checkpoints : 1;
    uint32_t nv_diagnostic_config : 1;

    CGpuAdapterDetail adapter_detail;
} CGpuAdapter_Vulkan;

typedef struct CGpuDevice_Vulkan {
    const CGpuDevice super;
    VkDevice pVkDevice;
    VkPipelineCache pPipelineCache;
    struct VkUtil_DescriptorPool* pDescriptorPool;
    struct VmaAllocator_T* pVmaAllocator;
    struct VolkDeviceTable mVkDeviceTable;
    // Created renderpass table
    struct CGpuVkPassTable* pPassTable;
} CGpuDevice_Vulkan;

typedef struct CGpuFence_Vulkan {
    CGpuFence super;
    VkFence pVkFence;
    uint32_t mSubmitted : 1;
} CGpuFence_Vulkan;

typedef struct CGpuSemaphore_Vulkan {
    CGpuSemaphore super;
    VkSemaphore pVkSemaphore;
    uint8_t mSignaled : 1;
} CGpuSemaphore_Vulkan;

typedef struct CGpuQueue_Vulkan {
    const CGpuQueue super;
    VkQueue pVkQueue;
    float mTimestampPeriod;
    uint32_t mVkQueueFamilyIndex : 5;
    // Cmd pool for inner usage like resource transition
    CGpuCommandPoolId pInnerCmdPool;
    CGpuCommandBufferId pInnerCmdBuffer;
    CGpuFenceId pInnerFence;
    /// Lock for multi-threaded descriptor allocations
    struct SMutex* pMutex;
} CGpuQueue_Vulkan;

typedef struct CGpuCommandPool_Vulkan {
    CGpuCommandPool super;
    VkCommandPool pVkCmdPool;
} CGpuCommandPool_Vulkan;

typedef struct CGpuCommandBuffer_Vulkan {
    CGpuCommandBuffer super;
    VkCommandBuffer pVkCmdBuf;
    VkPipelineLayout pBoundPipelineLayout;
    VkRenderPass pRenderPass;
    uint32_t mNodeIndex : 4;
    uint32_t mType : 3;
} CGpuCommandBuffer_Vulkan;

typedef struct CGpuBuffer_Vulkan {
    CGpuBuffer super;
    VkBuffer pVkBuffer;
    VkBufferView pVkStorageTexelView;
    VkBufferView pVkUniformTexelView;
    struct VmaAllocation_T* pVkAllocation;
    uint64_t mOffset;
} CGpuBuffer_Vulkan;

typedef struct CGpuTexture_Vulkan {
    CGpuTexture super;
    VkImageType mImageType;
    VkImage pVkImage;
    union
    {
        /// Contains resource allocation info such as parent heap, offset in heap
        struct VmaAllocation_T* pVkAllocation;
        VkDeviceMemory pVkDeviceMemory;
    };
} CGpuTexture_Vulkan;

typedef struct CGpuTextureView_Vulkan {
    CGpuTextureView super;
    VkImageView pVkRTVDSVDescriptor;
    VkImageView pVkSRVDescriptor;
    VkImageView pVkUAVDescriptor;
} CGpuTextureView_Vulkan;

typedef struct CGpuSampler_Vulkan {
    CGpuSampler super;
    VkSampler pVkSampler;
} CGpuSampler_Vulkan;

typedef struct CGpuShaderLibrary_Vulkan {
    CGpuShaderLibrary super;
    VkShaderModule mShaderModule;
    struct SpvReflectShaderModule* pReflect;
} CGpuShaderLibrary_Vulkan;

typedef struct CGpuSwapChain_Vulkan {
    CGpuSwapChain super;
    VkSurfaceKHR pVkSurface;
    VkSwapchainKHR pVkSwapChain;
} CGpuSwapChain_Vulkan;

typedef struct SetLayout_Vulkan {
    VkDescriptorSetLayout layout;
    VkDescriptorUpdateTemplate pUpdateTemplate;
    uint32_t mUpdateEntriesCount;
    VkDescriptorSet pEmptyDescSet;
} SetLayout_Vulkan;

typedef struct CGpuRootSignature_Vulkan {
    CGpuRootSignature super;
    VkPipelineLayout pPipelineLayout;
    SetLayout_Vulkan* pSetLayouts;
    VkPushConstantRange* pPushConstRanges;
    uint32_t mPushConstRangesCount;
} CGpuRootSignature_Vulkan;

typedef union VkDescriptorUpdateData
{
    VkDescriptorImageInfo mImageInfo;
    VkDescriptorBufferInfo mBufferInfo;
    VkBufferView mBuferView;
} VkDescriptorUpdateData;

typedef struct CGpuDescriptorSet_Vulkan {
    CGpuDescriptorSet super;
    VkDescriptorSet pVkDescriptorSet;
    union VkDescriptorUpdateData* pUpdateData;
} CGpuDescriptorSet_Vulkan;

typedef struct CGpuComputePipeline_Vulkan {
    CGpuComputePipeline super;
    VkPipeline pVkPipeline;
} CGpuComputePipeline_Vulkan;

typedef struct CGpuRenderPipeline_Vulkan {
    CGpuRenderPipeline super;
    VkPipeline pVkPipeline;
} CGpuRenderPipeline_Vulkan;

static const VkPipelineBindPoint gPipelineBindPoint[PIPELINE_TYPE_COUNT] = {
    VK_PIPELINE_BIND_POINT_MAX_ENUM,
    VK_PIPELINE_BIND_POINT_COMPUTE,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
#ifdef ENABLE_RAYTRACING
    VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
#endif
};

static const VkAttachmentStoreOp gVkAttachmentStoreOpTranslator[STORE_ACTION_COUNT] = {
    VK_ATTACHMENT_STORE_OP_STORE,
    VK_ATTACHMENT_STORE_OP_DONT_CARE
};
static const VkAttachmentLoadOp gVkAttachmentLoadOpTranslator[LOAD_ACTION_COUNT] = {
    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    VK_ATTACHMENT_LOAD_OP_LOAD,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
};

static const VkCompareOp gVkComparisonFuncTranslator[CMP_COUNT] = {
    VK_COMPARE_OP_NEVER,
    VK_COMPARE_OP_LESS,
    VK_COMPARE_OP_EQUAL,
    VK_COMPARE_OP_LESS_OR_EQUAL,
    VK_COMPARE_OP_GREATER,
    VK_COMPARE_OP_NOT_EQUAL,
    VK_COMPARE_OP_GREATER_OR_EQUAL,
    VK_COMPARE_OP_ALWAYS,
};

static const VkStencilOp gVkStencilOpTranslator[STENCIL_OP_COUNT] = {
    VK_STENCIL_OP_KEEP,
    VK_STENCIL_OP_ZERO,
    VK_STENCIL_OP_REPLACE,
    VK_STENCIL_OP_INVERT,
    VK_STENCIL_OP_INCREMENT_AND_WRAP,
    VK_STENCIL_OP_DECREMENT_AND_WRAP,
    VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    VK_STENCIL_OP_DECREMENT_AND_CLAMP,
};

#ifdef __cplusplus
} // end extern "C"
#endif

#ifdef __cplusplus
extern "C" {
#endif

FORCEINLINE static VkFormat VkUtil_FormatTranslateToVk(const ECGpuFormat format);

#include "cgpu_vulkan.inl"
#ifdef __cplusplus
} // end extern "C"
#endif