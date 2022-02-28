#pragma once
#include "cgpu/cgpux.hpp"
#include "platform/thread.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/vector_map.h>
#include <EASTL/unordered_map.h>
#include <atomic>

// AsyncI/O & AsyncUpload
// 1.1 CreateRenderMemeoryObject  |                                              Aux Thread
// 1.2 LoadDiskObject             |                                              I/O Thread
//                    (wait resource_handle_ready_)
//                              2.1 AsyncTransfer                                Cmd Thread
//                              2.2 Acquire/Release Barrier                      Cmd Thread
//                    (wait upload_ready_fence_/semaphore_)
// Then handles are available for drawcalls.
// AsyncTransfer Task
// .Prepare(AuxThread, ResourceDesc*) [resource_handle_ready_]  create buffers & driver objects (on Aux Thread)
//  when resource_handle_ready_:1 mem copy data to handles (on Any Callback Thread)
// .Run(TransferThread)     after setup, dispatch async transfer tasks (on Cmd Thread)
struct AsyncRenderResource {
    AsyncRenderResource() = default;
    AsyncRenderResource(AsyncRenderResource&& rhs)
        : resource_handle_ready_(rhs.resource_handle_ready_.load())
    {
    }
    AsyncRenderResource& operator=(AsyncRenderResource&& rhs)
    {
        this->resource_handle_ready_ = rhs.resource_handle_ready_.load();
        return *this;
    }
    inline virtual void Wait()
    {
        while (!resource_handle_ready_) {}
    }
    inline virtual bool Ready()
    {
        return resource_handle_ready_;
    }
    std::atomic_bool resource_handle_ready_ = false;
};
using AuxTaskCallback = eastl::function<void()>;
using AuxThreadTask = eastl::function<void(CGpuDeviceId)>;
using AuxThreadTaskWithCallback = eastl::pair<AuxThreadTask, AuxTaskCallback>;
static const AuxTaskCallback defaultAuxCallback = +[]() {};

struct AsyncRenderMemoryResource : public AsyncRenderResource {
    AsyncRenderMemoryResource() = default;
    AsyncRenderMemoryResource(AsyncRenderMemoryResource&& rhs)
        : AsyncRenderResource(eastl::move(rhs))
        , upload_started_(rhs.upload_started_.load())
        , queue_released_(rhs.queue_released_.load())
    {
    }
    AsyncRenderMemoryResource& operator=(AsyncRenderMemoryResource&& rhs)
    {
        this->resource_handle_ready_ = rhs.resource_handle_ready_.load();
        this->upload_started_ = rhs.upload_started_.load();
        return *this;
    }
    std::atomic_bool upload_started_ = false;
    std::atomic_bool queue_released_ = false;
};

struct AsyncRenderBuffer final : public AsyncRenderMemoryResource {
    AsyncRenderBuffer() = default;
    AsyncRenderBuffer(AsyncRenderBuffer&&) = default;

    void Initialize(struct RenderAuxThread* aux_thread, const CGpuBufferDescriptor& buffer_desc, const AuxTaskCallback& cb = defaultAuxCallback);
    void Destroy(struct RenderAuxThread* aux_thread = nullptr, const AuxTaskCallback& cb = defaultAuxCallback);

    CGpuBufferId buffer_;
};

struct AsyncRenderTexture final : public AsyncRenderMemoryResource {
    AsyncRenderTexture() = default;
    AsyncRenderTexture(AsyncRenderTexture&&) = default;

    void Initialize(struct RenderAuxThread* aux_thread, const CGpuTextureDescriptor& tex_desc, const AuxTaskCallback& cb = defaultAuxCallback, bool default_srv = true);
    void Initialize(struct RenderAuxThread* aux_thread, const CGpuTextureDescriptor& tex_desc, const CGpuTextureViewDescriptor& tex_view_desc, const AuxTaskCallback& cb = defaultAuxCallback);
    void Initialize(struct RenderAuxThread* aux_thread, const eastl::string name, const eastl::string disk_file, ECGpuFormat format, const AuxTaskCallback& cb = defaultAuxCallback);
    void Destroy(struct RenderAuxThread* aux_thread = nullptr, const AuxTaskCallback& cb = defaultAuxCallback);

    CGpuTextureId texture_;
    CGpuTextureViewId view_;
    unsigned char* image_bytes_ = nullptr;
    CGpuBufferId upload_buffer_ = nullptr;
};

struct AsyncRenderShader final : public AsyncRenderResource {
    AsyncRenderShader() = default;
    AsyncRenderShader(AsyncRenderShader&&) = default;

    void Initialize(struct RenderAuxThread* aux_thread, const CGpuShaderLibraryDescriptor& desc, const AuxTaskCallback& cb = defaultAuxCallback);
    void Destroy(struct RenderAuxThread* aux_thread = nullptr, const AuxTaskCallback& cb = defaultAuxCallback);

    CGpuShaderLibraryId shader_;
};

struct PipelineKey {
    uint32_t vertex_layout_id_;
    CGpuRootSignatureId root_sig_;
    bool wireframe_mode_;
    ECGpuFormat screen_format_;
};

namespace eastl
{
template <>
struct hash<PipelineKey> {
    size_t operator()(const PipelineKey& val) const { return skr_hash(&val, sizeof(PipelineKey), 0); }
};
template <>
struct equal_to<PipelineKey> {
    size_t operator()(const PipelineKey& a, const PipelineKey& b) const
    {
        const bool equal =
            (a.vertex_layout_id_ == b.vertex_layout_id_) &&
            (a.wireframe_mode_ == b.wireframe_mode_) &&
            (a.root_sig_ == b.root_sig_) &&
            (a.screen_format_ == b.screen_format_);
        return equal;
    }
};
} // namespace eastl

struct AsyncRenderPipeline final : public AsyncRenderResource {
    AsyncRenderPipeline() = default;
    AsyncRenderPipeline(AsyncRenderPipeline&&) = default;

    void Initialize(struct RenderAuxThread* aux_thread, const CGpuRenderPipelineDescriptor& desc, const AuxTaskCallback& cb = defaultAuxCallback);
    void Destroy(struct RenderAuxThread* aux_thread = nullptr, const AuxTaskCallback& cb = defaultAuxCallback);

    CGpuRenderPipelineId pipeline_;
};

namespace eastl
{
template <typename T>
using cached_hashset = hash_set<T, eastl::hash<T>, eastl::equal_to<T>, EASTLAllocatorType, true>;
}

struct RenderBlackboard {
    static void Initialize();
    static void Finalize(struct RenderAuxThread* aux_thread = nullptr);

    static const eastl::cached_hashset<CGpuVertexLayout>* GetVertexLayouts();
    static size_t AddVertexLayout(const CGpuVertexLayout& layout);

    static AsyncRenderTexture* GetTexture(const char* name);
    static AsyncRenderTexture* AddTexture(const char* name, struct RenderAuxThread* aux_thread, uint32_t width, uint32_t height, ECGpuFormat format = PF_R8G8B8A8_UNORM, const AuxTaskCallback& cb = defaultAuxCallback);
    static AsyncRenderTexture* AddTexture(const char* name, const char* disk_file, struct RenderAuxThread* aux_thread, ECGpuFormat format, const AuxTaskCallback& cb = defaultAuxCallback);

    static AsyncRenderPipeline* AddRenderPipeline(RenderAuxThread* aux_thread, const PipelineKey& key, const AuxTaskCallback& cb = defaultAuxCallback);
    static AsyncRenderPipeline* GetRenderPipeline(const PipelineKey& key);

protected:
    static eastl::vector_map<eastl::string, AsyncRenderTexture*> textures_;
    static eastl::unordered_map<PipelineKey, AsyncRenderPipeline*> pipelines_;
    static eastl::cached_hashset<CGpuVertexLayout> vertex_layouts_;
};

FORCEINLINE const eastl::cached_hashset<CGpuVertexLayout>* RenderBlackboard::GetVertexLayouts()
{
    return &vertex_layouts_;
}

FORCEINLINE size_t RenderBlackboard::AddVertexLayout(const CGpuVertexLayout& layout)
{
    const auto hash = vertex_layouts_.get_hash_code(layout);
    auto iter = vertex_layouts_.find_by_hash(hash);
    if (iter == vertex_layouts_.end())
    {
        vertex_layouts_.insert(layout);
        return hash;
    }
    return hash;
}