#include "SkrRT/platform/debug.h"
#include "SkrRT/misc/make_zeroed.hpp"
#include "vram_readers.hpp"
#include <EASTL/fixed_map.h>

// VFS READER IMPLEMENTATION

namespace skr {
namespace io {

SwapableCmdPool::RC::RC() SKR_NOEXCEPT
{

}

SwapableCmdPool::RC::RC(CGPUCommandPoolId v, SAtomic32* pRC) SKR_NOEXCEPT
    : v(v), pRC(pRC)
{
    if (pRC)
        skr_atomic32_add_relaxed(pRC, 1);
}

SwapableCmdPool::RC::~RC() SKR_NOEXCEPT
{
    if (pRC)
    {
        auto prev = skr_atomic32_add_relaxed(pRC, -1);
        if (prev == 1)
        {
            cgpu_reset_command_pool(v);
        }
    }
}

SwapableCmdPool::SwapableCmdPool() SKR_NOEXCEPT
{

}

SwapableCmdPool::~SwapableCmdPool() SKR_NOEXCEPT
{
    SKR_ASSERT(pools[0] == nullptr);
    SKR_ASSERT(pools[1] == nullptr);
}

void SwapableCmdPool::initialize(CGPUQueueId queue) SKR_NOEXCEPT
{
    CGPUCommandPoolDescriptor pdesc = { u8"VRAMIOService-CmdPool" };
    pools[0] = cgpu_create_command_pool(queue, &pdesc);
    pools[1] = cgpu_create_command_pool(queue, &pdesc);
}

void SwapableCmdPool::finalize() SKR_NOEXCEPT
{
    cgpu_free_command_pool(pools[0]);
    cgpu_free_command_pool(pools[1]);
    pools[0] = nullptr;
    pools[1] = nullptr;
}

FORCEINLINE SwapableCmdPool::RC SwapableCmdPool::get() SKR_NOEXCEPT
{
    return RC(pools[index], &rcs[index]);
}

void SwapableCmdPool::swap() SKR_NOEXCEPT
{
    index = (index == 0) ? 1 : 0;
}

GPUUploadCmd::GPUUploadCmd() SKR_NOEXCEPT
{

}

GPUUploadCmd::GPUUploadCmd(CGPUQueueId queue, IOBatchId batch) SKR_NOEXCEPT
    : batch(batch), queue(queue)
{

}

void GPUUploadCmd::start(SwapableCmdPool& swap_pool) SKR_NOEXCEPT
{
    pool = swap_pool.get();
    CGPUCommandBufferDescriptor bdesc = { /*.is_secondary = */false };
    cmdbuf = cgpu_create_command_buffer(pool, &bdesc);
    cgpu_cmd_begin(cmdbuf);
    fence = cgpu_create_fence(queue->device);
}

void GPUUploadCmd::finish() SKR_NOEXCEPT
{
    for (auto upload_buffer : upload_buffers)
        cgpu_free_buffer(upload_buffer);
    cgpu_free_command_buffer(cmdbuf);
    cgpu_free_fence(fence);
    okay = true;
}

CommonVRAMReader::CommonVRAMReader(VRAMService* service, IRAMService* ram_service) SKR_NOEXCEPT 
    : VRAMReaderBase(service), ram_service(ram_service) 
{

}

CommonVRAMReader::~CommonVRAMReader() SKR_NOEXCEPT
{
    for (auto&& [queue, pool] : cmdpools)
        pool.finalize();   
}

bool CommonVRAMReader::fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
{
    fetched_batches[priority].enqueue(batch);
    inc_processing(priority);
    return true;
}

void CommonVRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    addRAMRequests(priority);
    ensureRAMRequests(priority);
    addUploadRequests(priority);
    ensureUploadRequests(priority);
}

void CommonVRAMReader::recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{

}

bool CommonVRAMReader::poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
{
    if (processed_batches[priority].try_dequeue(batch))
    {
        dec_processed(priority);
        return batch.get();
    }
    return false;
}

void CommonVRAMReader::addRAMRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IOBatchId ram_batch = nullptr;
    IOBatchId vram_batch = nullptr;
    while (fetched_batches[priority].try_dequeue(vram_batch))
    {
    for (auto&& vram_request : vram_batch->get_requests())
    {
        if (service->runner.try_cancel(priority, vram_request))
        {
            // cancel...
        }
        else if (auto pStatus = io_component<IOStatusComponent>(vram_request.get()))
        {
            if (pStatus->getStatus() == SKR_IO_STAGE_RESOLVING)
            {
                ZoneScopedN("VRAMReader::RAMRequest");
                auto pPath = io_component<PathSrcComponent>(vram_request.get());
                auto pUpload = io_component<VRAMUploadComponent>(vram_request.get());
                if (pPath && !pPath->path.is_empty() && pUpload)
                {
                    pStatus->setStatus(SKR_IO_STAGE_LOADING);
                    auto ram_request = ram_service->open_request();
                    if (!ram_batch)
                    {
                        ram_batch = ram_service->open_batch(8);
                    }
                    if (pPath->vfs)
                        ram_request->set_vfs(pPath->vfs);
                    ram_request->set_path(pPath->path.u8_str());
                    // TODO: READ PARTIAL DATA ONLY NEEDED FROM FILE
                    ram_request->add_block({});
                    auto result = ram_batch->add_request(ram_request, &pUpload->ram_future);
                    pUpload->buffer = skr::static_pointer_cast<IRAMIOBuffer>(result);
                }
                auto pMemory = io_component<MemorySrcComponent>(vram_request.get());
                if (pMemory && pMemory->data && pMemory->size)
                {
                    pUpload->data = pMemory->data;
                    pUpload->size = pMemory->size;
                }
            }
            else
                SKR_UNREACHABLE_CODE()
        }
    }
    if(vram_batch != nullptr) 
        ramloading_batches[priority].emplace_back(vram_batch); 
    }
    if (ram_batch != nullptr)
        ram_service->request(ram_batch);        
}

void CommonVRAMReader::ensureRAMRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto&& batches = ramloading_batches[priority];
    // erase empty batches
    batches.erase(
    eastl::remove_if(batches.begin(), batches.end(), [](auto& batch) {
        return batch->get_requests().empty();
    }), batches.end());

    for (auto&& batch : batches)
    {
        uint32_t finished_cnt = 0;
        auto requests = batch->get_requests();
        for (auto&& request : requests)
        {
            auto pUpload = io_component<VRAMUploadComponent>(request.get());
            if (pUpload->buffer && pUpload->ram_future.is_ready())
            {
                pUpload->data = pUpload->buffer->get_data();
                pUpload->size = pUpload->buffer->get_size();
            }
            if (pUpload->data && pUpload->size)
            {
                finished_cnt += 1;
            }
        }
        if (finished_cnt == requests.size()) // batch is all finished
        {
            to_upload_batches[priority].emplace_back(batch);
            batch.reset();
        }
    }

    // erase empty batches
    batches.erase(
    eastl::remove_if(batches.begin(), batches.end(), [](auto& batch) {
        return (batch == nullptr);
    }), batches.end());
}

template <size_t N = 1>
struct StackCmdAllocator : public eastl::fixed_map<CGPUQueueId, GPUUploadCmd, N>
{
    auto& allocate(IOBatchId& batch, SwapableCmdPoolMap& cmdpools, VRAMUploadComponent* pUpload)
    {
        auto& cmds = *this;
        if (cmds.find(pUpload->transfer_queue) == cmds.end())
        {
            cmds.emplace(pUpload->transfer_queue, GPUUploadCmd(pUpload->transfer_queue, batch));
        }
        auto& cmd = cmds[pUpload->transfer_queue];
        auto cmdqueue = cmd.get_queue();
        if (cmdpools.find(cmdqueue) == cmdpools.end())
        {
            auto&& [iter, sucess] = cmdpools.emplace(cmdqueue, SwapableCmdPool());
            iter->second.initialize(cmdqueue);
        }
        auto&& cmdpool = cmdpools[cmdqueue];
        if (cmd.get_cmdbuf() == nullptr)
        {
            ZoneScopedN("PrepareCmd");
            cmd.start(cmdpool);
        }
        return cmd;
    }
};

void CommonVRAMReader::addUploadRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    ZoneScopedN("VRAMReader::UploadRequests");

    auto&& batches = to_upload_batches[priority];
    for (auto&& batch : batches)
    {
        StackCmdAllocator<1> cmds;
        auto requests = batch->get_requests();
        for (auto&& request : requests)
        {
            auto pUpload = io_component<VRAMUploadComponent>(request.get());
#ifdef TRACY_ENABLE
            auto pPath = io_component<PathSrcComponent>(request.get());
#endif
            auto& cmd = cmds.allocate(batch, cmdpools, pUpload);
            auto cmdqueue = cmd.get_queue();
            auto cmdbuf = cmd.get_cmdbuf();
            // record copy command
            if (auto pBuffer = io_component<VRAMBufferComponent>(request.get()))
            {
                CGPUBufferId upload_buffer = nullptr;
                {
                    // prepare upload buffer
                    ZoneScopedN("PrepareUploadBuffer");
#ifdef TRACY_ENABLE
                    skr::string Name = u8"BufferUpload-";
                    Name += pPath->path;
                    TracyMessage(Name.c_str(), Name.size());
#endif
                    skr::string name = /*pBuffer->name ? buffer_io.vbuffer.buffer_name :*/ u8"";
                    name += u8"-upload";
                    upload_buffer = cgpux_create_mapped_upload_buffer(cmdqueue->device, pUpload->size, name.u8_str());
                    cmd.upload_buffers.emplace_back(upload_buffer);

                    memcpy(upload_buffer->info->cpu_mapped_address, pUpload->data, pUpload->size);
                }
                if (upload_buffer)
                {
                    // TODO: SUPPORT BLOCKS
                    CGPUBufferToBufferTransfer buf_cpy = {};
                    buf_cpy.dst = pBuffer->buffer;
                    buf_cpy.dst_offset = pBuffer->offset;
                    buf_cpy.src = upload_buffer;
                    buf_cpy.src_offset = 0;
                    buf_cpy.size = pUpload->size;
                    cgpu_cmd_transfer_buffer_to_buffer(cmdbuf, &buf_cpy);
                }
                auto&& Artifact = skr::static_pointer_cast<VRAMBuffer>(pBuffer->artifact);
                Artifact->buffer = pBuffer->buffer;
                // TODO: RELEASE BARRIER
                auto buffer_barrier = make_zeroed<CGPUBufferBarrier>();
                buffer_barrier.buffer = pBuffer->buffer;
                buffer_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
                buffer_barrier.dst_state = CGPU_RESOURCE_STATE_COMMON;
                // release
                if (cmdqueue->type == CGPU_QUEUE_TYPE_TRANSFER)
                {
                    buffer_barrier.queue_release = true;
                    buffer_barrier.queue_type = cmdqueue->type;
                }
                CGPUResourceBarrierDescriptor barrier_desc = {};
                barrier_desc.buffer_barriers = &buffer_barrier;
                barrier_desc.buffer_barriers_count = 1;
                cgpu_cmd_resource_barrier(cmdbuf, &barrier_desc);
            }
            else if (auto pTexture = io_component<VRAMTextureComponent>(request.get()))
            {
                CGPUBufferId upload_buffer = nullptr;
                {
                    // prepare upload buffer
                    ZoneScopedN("PrepareUploadBuffer");
#ifdef TRACY_ENABLE
                    skr::string Name = u8"TextureUpload-";
                    Name += pPath->path;
                    TracyMessage(Name.c_str(), Name.size());
#endif
                    skr::string name = /*pBuffer->name ? buffer_io.vbuffer.buffer_name :*/ u8"";
                    name += u8"-upload";
                    upload_buffer = cgpux_create_mapped_upload_buffer(cmdqueue->device, pUpload->size, name.u8_str());
                    cmd.upload_buffers.emplace_back(upload_buffer);

                    memcpy(upload_buffer->info->cpu_mapped_address, pUpload->data, pUpload->size);
                }
                if (upload_buffer)
                {
                    CGPUBufferToTextureTransfer tex_cpy = {};
                    tex_cpy.dst = pTexture->texture;
                    tex_cpy.dst_subresource.aspects = CGPU_TVA_COLOR;
                    // TODO: texture array & mips
                    tex_cpy.dst_subresource.base_array_layer = 0;
                    tex_cpy.dst_subresource.layer_count = 1;
                    tex_cpy.dst_subresource.mip_level = 0;
                    tex_cpy.src = upload_buffer;
                    tex_cpy.src_offset = 0;
                    cgpu_cmd_transfer_buffer_to_texture(cmdbuf, &tex_cpy);
                }
                auto&& Artifact = skr::static_pointer_cast<VRAMTexture>(pTexture->artifact);
                Artifact->texture = pTexture->texture;
                // TODO: RELEASE BARRIER
                auto texture_barrier = make_zeroed<CGPUTextureBarrier>();
                texture_barrier.texture = pTexture->texture;
                texture_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
                texture_barrier.dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
                // release
                if (cmdqueue->type == CGPU_QUEUE_TYPE_TRANSFER)
                {
                    texture_barrier.queue_release = true;
                    texture_barrier.queue_type = cmdqueue->type;
                }
                CGPUResourceBarrierDescriptor barrier_desc = {};
                barrier_desc.texture_barriers = &texture_barrier;
                barrier_desc.texture_barriers_count = 1;
                cgpu_cmd_resource_barrier(cmdbuf, &barrier_desc);
            }
        }
        // submit all cmds
        {
            ZoneScopedN("SubmitCmds");
            for (auto&& [queue, cmd] : cmds)
            {
                gpu_uploads[priority].emplace_back(cmd);

                auto cmdbuf = cmd.get_cmdbuf();
                auto fence = cmd.get_fence();
                CGPUQueueSubmitDescriptor submit = {};
                submit.cmds = &cmdbuf;
                submit.cmds_count = 1;
                submit.signal_fence = fence;
                cgpu_cmd_end(cmdbuf);
                cgpu_submit_queue(queue, &submit);
            }
        }
    }

    // clear batches & swap all pools
    batches.clear();
    for (auto&& [queue, pool] : cmdpools)
    {
        pool.swap();
    }
}

void CommonVRAMReader::ensureUploadRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    for (auto&& upload : gpu_uploads[priority])
    {
        auto batch = upload.get_batch();
        auto fence = upload.get_fence();
        auto status = cgpu_query_fence_status(fence);
        if (status == CGPU_FENCE_STATUS_COMPLETE)
        {
            ZoneScopedN("EnsureFence");

            for (auto&& request : batch->get_requests())
            {
                auto pStatus = io_component<IOStatusComponent>(request.get());
                pStatus->setStatus(SKR_IO_STAGE_LOADED);
            }
            dec_processing(priority);
            inc_processed(priority);
            processed_batches[priority].enqueue(batch);
            upload.finish();
        }
    }
    // erase all finished uploads
    gpu_uploads[priority].erase(std::remove_if(
        gpu_uploads[priority].begin(), gpu_uploads[priority].end(), [](auto&& upload) {
            return upload.is_finished();
        }), gpu_uploads[priority].end());
}

} // namespace io
} // namespace skr

#include "../vram/vram_readers.hpp"

namespace skr {
namespace io {

static const ESkrDStoragePriority DStoragePriorityLUT_VRAM[] = 
{
    SKR_DSTORAGE_PRIORITY_LOW,
    SKR_DSTORAGE_PRIORITY_NORMAL,
    SKR_DSTORAGE_PRIORITY_HIGH
};
static_assert(sizeof(DStoragePriorityLUT_VRAM) / sizeof(DStoragePriorityLUT_VRAM[0]) == SKR_ASYNC_SERVICE_PRIORITY_COUNT);

static const char8_t* DStorageNames_VRAM[] = { u8"F2V-Low", u8"F2V-Normal", u8"F2V-High" };
static_assert(sizeof(DStorageNames_VRAM) / sizeof(DStorageNames_VRAM[0]) == SKR_ASYNC_SERVICE_PRIORITY_COUNT);

DStorageVRAMReader::DStorageVRAMReader(VRAMService* service, CGPUDeviceId device) SKR_NOEXCEPT 
    : VRAMReaderBase(service)
{
    SkrDStorageQueueDescriptor desc = {};
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        events[i] = SmartPoolPtr<DStorageEvent>::Create(kIOPoolObjectsMemoryName);
    }
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        const auto dpriority = DStoragePriorityLUT_VRAM[i];
        desc.source = SKR_DSTORAGE_SOURCE_FILE;
        desc.capacity = SKR_DSTORAGE_MAX_QUEUE_CAPACITY;
        desc.priority = dpriority;
        desc.name = DStorageNames_VRAM[i];
        desc.gpu_device = device;
        f2v_queues[i] = skr_create_dstorage_queue(&desc);
    }
    desc.source = SKR_DSTORAGE_SOURCE_MEMORY;
    desc.priority = SKR_DSTORAGE_PRIORITY_REALTIME;
    desc.name = u8"M2V";
    m2v_queue = skr_create_dstorage_queue(&desc);
};

DStorageVRAMReader::~DStorageVRAMReader() SKR_NOEXCEPT
{
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        skr_free_dstorage_queue(f2v_queues[i]);
    }
    skr_free_dstorage_queue(m2v_queue);
}

bool DStorageVRAMReader::fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
{
    auto B = static_cast<IOBatchBase*>(batch.get());
    if (B->can_use_dstorage)
    {
        fetched_batches[priority].enqueue(batch);
        inc_processing(priority);
    }
    else
    {
        processed_batches[priority].enqueue(batch);
        inc_processed(priority);
    }
    return true;
}

void DStorageVRAMReader::enqueueAndSubmit(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto queue = m2v_queue;
    auto instance = skr_get_dstorage_instnace();
    IOBatchId batch;
    skr::SObjectPtr<DStorageEvent> event;
#ifdef TRACY_ENABLE
    TracyCZoneCtx Zone;
    bool bZoneSet = false;
#endif
    while (fetched_batches[priority].try_dequeue(batch))
    {
        auto& eref = event;
        if (!eref)
        {
#ifdef TRACY_ENABLE
            TracyCZoneN(z, "DStorage::EnqueueAndSubmit", 1);
            Zone = z;
            bZoneSet = true;
#endif
            eref = skr::static_pointer_cast<DStorageEvent>(events[priority]->allocate(queue));
        }
        for (auto&& request : batch->get_requests())
        {
            auto pStatus = io_component<IOStatusComponent>(request.get());
            auto pDS = io_component<VRAMDStorageComponent>(request.get());
            auto pMemory = io_component<MemorySrcComponent>(request.get());
            if (service->runner.try_cancel(priority, request))
            {
                skr_dstorage_close_file(instance, pDS->dfile);
                pDS->dfile = nullptr;
            }
            else if (auto pBuffer = io_component<VRAMBufferComponent>(request.get()))
            {
                ZoneScopedN("DStorage::ReadBufferCmd");
                pStatus->setStatus(SKR_IO_STAGE_LOADING);
                CGPUDStorageBufferIODescriptor io = {};
                // io.name = rq->get_path();
                io.fence = nullptr;
                io.compression = SKR_DSTORAGE_COMPRESSION_NONE; // TODO: DECOMPRESS
                if (true) // memory
                {
                    io.source_type = SKR_DSTORAGE_SOURCE_MEMORY;
                    io.source_memory.bytes = pMemory->data;
                    io.source_memory.bytes_size = pMemory->size;
                    io.uncompressed_size = pMemory->size;
                }
                else // file
                {
                    SKR_ASSERT(pDS->dfile);
                    io.source_type = SKR_DSTORAGE_SOURCE_FILE;
                    io.source_file.file = pDS->dfile;
                    io.source_file.offset = 0;
                    io.source_file.size = 0;
                }
                io.buffer = pBuffer->buffer;
                io.offset = pBuffer->offset;
                cgpu_dstorage_enqueue_buffer_request(queue, &io);

                auto&& Artifact = skr::static_pointer_cast<VRAMBuffer>(pBuffer->artifact);
                Artifact->buffer = pBuffer->buffer;
            }
            else if (auto pTexture = io_component<VRAMTextureComponent>(request.get()))
            {
                ZoneScopedN("DStorage::ReadTextureCmd");
                pStatus->setStatus(SKR_IO_STAGE_LOADING);
                CGPUDStorageTextureIODescriptor io = {};
                // io.name = rq->get_path();
                io.fence = nullptr;
                io.compression = SKR_DSTORAGE_COMPRESSION_NONE; // TODO: DECOMPRESS
                if (true) // memory
                {
                    io.source_type = SKR_DSTORAGE_SOURCE_MEMORY;
                    io.source_memory.bytes = pMemory->data;
                    io.source_memory.bytes_size = pMemory->size;
                    io.uncompressed_size = pMemory->size;
                }
                else // file
                {
                    SKR_ASSERT(pDS->dfile);
                    io.source_type = SKR_DSTORAGE_SOURCE_FILE;
                    io.source_file.file = pDS->dfile;
                    io.source_file.offset = 0;
                    io.source_file.size = 0;
                }
                io.texture = pTexture->texture;
                const auto pInfo = io.texture->info;
                io.width = (uint32_t)pInfo->width;
                io.height = (uint32_t)pInfo->height;
                io.depth = (uint32_t)pInfo->depth;
                cgpu_dstorage_enqueue_texture_request(queue, &io);

                auto&& Artifact = skr::static_pointer_cast<VRAMTexture>(pTexture->artifact);
                Artifact->texture = pTexture->texture;
            }
        }
        event->batches.emplace_back(batch);
    }
    if (event)
    {
        if (const auto enqueued = event->batches.size())
        {
            skr_dstorage_queue_submit(queue, event->event);
            submitted[priority].emplace_back(event);
        }
    }
#ifdef TRACY_ENABLE
    if (bZoneSet)
        TracyCZoneEnd(Zone);
#endif
}

void DStorageVRAMReader::pollSubmitted(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    ZoneScopedN("DStorage::PollSubmitted");

    auto instance = skr_get_dstorage_instnace();
    for (auto& e : submitted[priority])
    {
        if (e->okay() || e->batches.empty())
        {
            for (auto batch : e->batches)
            {
                for (auto request : batch->get_requests())
                {
                    auto pDS = io_component<VRAMDStorageComponent>(request.get());
                    auto pStatus = io_component<IOStatusComponent>(request.get());
                    pStatus->setStatus(SKR_IO_STAGE_LOADED);
                    if (pDS->dfile)
                    {
                        skr_dstorage_close_file(instance, pDS->dfile);
                        pDS->dfile = nullptr;
                    }
                }
                processed_batches[priority].enqueue(batch);
                dec_processing(priority);
                inc_processed(priority);
            }
            e.reset();
        }
    }

    // remove empty events
    auto cleaner = eastl::remove_if(submitted[priority].begin(), submitted[priority].end(), [](const auto& e) { return !e; });
    submitted[priority].erase(cleaner, submitted[priority].end());
}

void DStorageVRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    enqueueAndSubmit(priority);
    pollSubmitted(priority);
}

void DStorageVRAMReader::recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{

}

bool DStorageVRAMReader::poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
{
    if (processed_batches[priority].try_dequeue(batch))
    {
        dec_processed(priority);
        return batch.get();
    }
    return false;
}
} // namespace io
} // namespace skr