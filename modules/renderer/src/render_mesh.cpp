#include "utils/io.hpp"
#include "utils/log.h"
#include "platform/debug.h"
#include "cgpu/io.hpp"
#include "platform/vfs.h"
#include "utils/make_zeroed.hpp"
#include "skr_renderer/skr_renderer.h"
#include "skr_renderer/render_mesh.h"
#include <platform/filesystem.hpp>

#include "tracy/Tracy.hpp"

void setup_render_mesh(skr_render_mesh_id render_mesh, skr_mesh_resource_id mesh_resource)
{
    uint32_t ibv_c = 0;
    uint32_t vbv_c = 0;
    for (uint32_t i = 0; i < mesh_resource->sections.size(); i++)
    {
        const auto& section = mesh_resource->sections[i];
        for (auto prim_idx : section.primive_indices)
        {
            auto& prim = mesh_resource->primitives[prim_idx];
            vbv_c += (uint32_t)prim.vertex_buffers.size();
            ibv_c++;
        }
    }
    render_mesh->index_buffer_views.reserve(ibv_c);
    render_mesh->vertex_buffer_views.reserve(vbv_c);
    for (uint32_t i = 0; i < mesh_resource->sections.size(); i++)
    {
        const auto& section = mesh_resource->sections[i];
        for (auto prim_idx : section.primive_indices)
        {
            SKR_ASSERT(render_mesh->index_buffer_views.capacity() >= render_mesh->index_buffer_views.size());
            SKR_ASSERT(render_mesh->vertex_buffer_views.capacity() >= render_mesh->vertex_buffer_views.size());
            auto& draw_cmd = render_mesh->primitive_commands.emplace_back();
            auto& prim = mesh_resource->primitives[prim_idx];
            auto& mesh_ibv = render_mesh->index_buffer_views.emplace_back();
            auto vbv_start = render_mesh->vertex_buffer_views.size();
            for (uint32_t j = 0; j < prim.vertex_buffers.size(); j++)
            {
                auto& mesh_vbv = render_mesh->vertex_buffer_views.emplace_back();
                const auto buffer_index = prim.vertex_buffers[j].buffer_index;
                mesh_vbv.buffer = render_mesh->buffer_destinations[buffer_index].buffer;
                mesh_vbv.offset = prim.vertex_buffers[j].offset;
                mesh_vbv.stride = prim.vertex_buffers[j].stride;
            }
            const auto buffer_index = prim.index_buffer.buffer_index;
            mesh_ibv.buffer = render_mesh->buffer_destinations[buffer_index].buffer;
            mesh_ibv.offset = prim.index_buffer.index_offset;
            mesh_ibv.stride = prim.index_buffer.stride;
            mesh_ibv.index_count = prim.index_buffer.index_count;
            mesh_ibv.first_index = prim.index_buffer.first_index;

            draw_cmd.ibv = &mesh_ibv;
            draw_cmd.vbvs = skr::span(render_mesh->vertex_buffer_views.data() + vbv_start, prim.vertex_buffers.size());
        }
    }
}

void skr_render_mesh_create_from_gltf(SRenderDeviceId render_device, skr_io_ram_service_t* ram_service, skr_io_vram_service_t* vram_service, const char* path, skr_render_mesh_request_t* request)
{
    ZoneScopedN("ioRAM & VRAM Mesh Request");

    struct CallbackData
    {
        CGPUDStorageQueueId dstorage_queue;
        skr_render_mesh_request_t* request = nullptr;
        SRenderDeviceId render_device = nullptr;
        skr_io_ram_service_t* ram_service = nullptr;
        skr_io_vram_service_t* vram_service = nullptr;
        // buffer io
        eastl::vector<std::string> buffer_paths;
        uint32_t buffers_count = 0;
        SAtomic32 created_buffers = 0;
        SAtomic32 finished_buffers = 0;
        bool allReady() const
        {
            return skr_atomic32_load_acquire(&finished_buffers) == buffers_count;
        }
    };
    SKR_ASSERT((!request->queue_override || !request->dstorage_queue_override) && "only one type of override queue should be set!");
    SKR_ASSERT(request->mesh_resource_request.vfs_override && "vfs_override is null, only support override mode now!");
    auto dstorage_queue = (request->dstorage_source == CGPU_DSTORAGE_SOURCE_FILE) ?
        render_device->get_file_dstorage_queue() : 
        render_device->get_memory_dstorage_queue();
    auto cbData = SkrNew<CallbackData>();
    cbData->render_device = render_device;
    cbData->ram_service = ram_service;
    cbData->vram_service = vram_service;
    cbData->dstorage_queue = dstorage_queue;
    cbData->request = request;
    request->mesh_resource_request.load_bin_to_memory =  
        request->queue_override ? true : !( (request->dstorage_source == CGPU_DSTORAGE_SOURCE_FILE) && dstorage_queue );
    request->mesh_resource_request.callback_data = cbData;
    request->mesh_resource_request.callback = +[](skr_gltf_ram_io_request_t* gltf_request, void* data)
    {
        auto cbData = (CallbackData*)data;
        auto device = cbData->render_device->get_cgpu_device();
        auto request = cbData->request;
        auto vram_service = cbData->vram_service;

        skr_atomic32_store_release(&request->buffers_io_status, SKR_ASYNC_IO_STATUS_CREATING_RESOURCE);
        auto render_mesh = request->render_mesh = SkrNew<skr_render_mesh_t>();
        auto mesh_resource = render_mesh->mesh_resource_id = gltf_request->mesh_resource;
        // resize gpu request
        render_mesh->vio_requests.resize(mesh_resource->bins.size());
        render_mesh->buffer_destinations.resize(mesh_resource->bins.size());
        cbData->buffers_count = (uint32_t)mesh_resource->bins.size();
        cbData->buffer_paths.resize(cbData->buffers_count);
        for (uint32_t i = 0; i < render_mesh->buffer_destinations.size(); i++)
        {
            auto mesh_buffer_io = make_zeroed<skr_vram_buffer_io_t>();
            mesh_buffer_io.device = device;
            mesh_buffer_io.transfer_queue = request->queue_override ? request->queue_override : cbData->render_device->get_cpy_queue();
            // dstorage
            auto gltfPath = (skr::filesystem::path(gltf_request->vfs_override->mount_dir) / mesh_resource->bins[i].uri.c_str()).u8string();
            mesh_buffer_io.dstorage.source_type = CGPU_DSTORAGE_SOURCE_FILE;
            mesh_buffer_io.dstorage.path = gltfPath.c_str();
            mesh_buffer_io.dstorage.queue = request->queue_override ? nullptr : cbData->dstorage_queue;
            mesh_buffer_io.dstorage.source_type = request->dstorage_source;
            mesh_buffer_io.dstorage.uncompressed_size = mesh_resource->bins[i].bin.size;
            // buffer
            mesh_buffer_io.vbuffer.resource_types = CGPU_RESOURCE_TYPE_INDEX_BUFFER | CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
            mesh_buffer_io.vbuffer.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
            mesh_buffer_io.vbuffer.buffer_size = mesh_resource->bins[i].bin.size;
            mesh_buffer_io.src_memory.bytes = mesh_resource->bins[i].bin.bytes;
            mesh_buffer_io.src_memory.size =  mesh_resource->bins[i].bin.size;
            
            if (request->mesh_name)
            {
                mesh_buffer_io.vbuffer.buffer_name = request->mesh_name;
            }
            else
            {
                const char8_t* gltf_buffer_default_name = "gltf_buffer";
                mesh_buffer_io.vbuffer.buffer_name = gltf_buffer_default_name;
            }
            mesh_buffer_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* io, void* data){
                auto cbData = (CallbackData*)data;
                auto request = cbData->request;
                skr_atomic32_add_relaxed(&cbData->finished_buffers, 1);

                if (skr_atomic32_load_acquire(&cbData->finished_buffers) == cbData->buffers_count)
                {
                    setup_render_mesh(request->render_mesh, request->render_mesh->mesh_resource_id);
                    skr_atomic32_store_release(&request->buffers_io_status, SKR_ASYNC_IO_STATUS_OK);
                }
                if (cbData->allReady())
                {
                    SkrDelete(cbData);
                }
            };
            mesh_buffer_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = cbData;
            mesh_buffer_io.callbacks[SKR_ASYNC_IO_STATUS_VRAM_LOADING] = +[](skr_async_io_request_t* io, void* data){
                auto cbData = (CallbackData*)data;
                auto request = cbData->request;
                skr_atomic32_add_relaxed(&cbData->created_buffers, 1);
                if (skr_atomic32_load_acquire(&cbData->created_buffers) == cbData->buffers_count)
                {
                    skr_atomic32_store_release(&request->buffers_io_status, SKR_ASYNC_IO_STATUS_VRAM_LOADING);
                }
            };
            mesh_buffer_io.callback_datas[SKR_ASYNC_IO_STATUS_VRAM_LOADING] = cbData;
            vram_service->request(&mesh_buffer_io, &render_mesh->vio_requests[i], &render_mesh->buffer_destinations[i]);
        }
    };
    skr_mesh_resource_create_from_gltf(ram_service, path, &request->mesh_resource_request);
}

void skr_render_mesh_free(skr_render_mesh_id render_mesh)
{
    if (render_mesh->mesh_resource_id)
    {
        skr_mesh_resource_free(render_mesh->mesh_resource_id);
    }
    for (auto&& destination : render_mesh->buffer_destinations)
    {
        cgpu_free_buffer(destination.buffer);
    }
    SkrDelete(render_mesh);
}
