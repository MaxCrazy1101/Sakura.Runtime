#include "ecs/dual.h"
#include "utils/io.hpp"
#include "mesh_asset.hpp"
#include "skr_renderer/resources/mesh_resource.h"
#include "resource/resource_factory.h"
#include "utils/defer.hpp"
#include "utils/log.hpp"

#define MAGIC_SIZE_GLTF_PARSE_READY ~0

void* skd::asset::SGltfMeshImporter::Import(skr::io::RAMService* ioService, SCookContext* context) 
{
    skr::filesystem::path relPath = assetPath.c_str();
    const auto assetRecord = context->GetAssetRecord();
    auto ext = relPath.extension();
    if (ext != ".gltf")
    {
        return nullptr;
    }
    auto path = context->AddFileDependency(relPath);
    // prepare callback
    auto u8Path = path.u8string();
    skr::task::event_t counter;
    struct CallbackData
    {
        skr_async_ram_destination_t destination;
        skr::task::event_t* pCounter;   
        eastl::string u8Path;
    } callbackData;
    callbackData.pCounter = &counter;
    callbackData.u8Path = u8Path.c_str();
    // prepare io
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        cgltf_options options = {};
        struct cgltf_data* gltf_data_ = nullptr;
        if (cbData->destination.bytes)
        {
            cgltf_result result = cgltf_parse(&options, cbData->destination.bytes, cbData->destination.size, &gltf_data_);
            if (result != cgltf_result_success)
            {
                gltf_data_ = nullptr;
            }
            else
            {
                result = cgltf_load_buffers(&options, gltf_data_, cbData->u8Path.c_str());
                result = cgltf_validate(gltf_data_);
                if (result != cgltf_result_success)
                {
                    gltf_data_ = nullptr;
                }
            }
        }
        sakura_free(cbData->destination.bytes);
        cbData->destination.bytes = (uint8_t*)gltf_data_;
        cbData->destination.size = MAGIC_SIZE_GLTF_PARSE_READY;
        cbData->pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&callbackData;
    skr_async_io_request_t ioRequest = {};
    ioService->request(assetRecord->project->vfs, &ramIO, &ioRequest, &callbackData.destination);
    counter.wait(false);
    // parse
    if (callbackData.destination.size == MAGIC_SIZE_GLTF_PARSE_READY)
    {
        cgltf_data* data = (cgltf_data*)callbackData.destination.bytes;
        auto mesh = SkrNew<skr_mesh_resource_t>();
        mesh->name = data->meshes[0].name;
        if (mesh->name.empty())
        {
            mesh->name = "gltfMesh";
        }
        cgltf_free(data);
        return mesh;
    }
    return nullptr;
}

void skd::asset::SGltfMeshImporter::Destroy(void* resource)
{
    auto mesh = (skr_mesh_resource_t*)resource;
    SkrDelete(mesh);
}

bool skd::asset::SMeshCooker::Cook(SCookContext* ctx)
{ 
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    auto resource = ctx->Import<skr_mesh_resource_t>();
    if(!resource)
    {
        return false;
    }
    SKR_DEFER({ ctx->Destroy(resource); });
    //-----write resource header
    eastl::vector<uint8_t> buffer;
    //TODO: 公共化 VectorWriter
    struct VectorWriter
    {
        eastl::vector<uint8_t>* buffer;
        int write(const void* data, size_t size)
        {
            buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
            return 0;
        }
    } writer{&buffer};
    skr_binary_writer_t archive(writer);
    //------write resource object
    skr::binary::Archive(&archive, resource->name);
    // archive(resource->sections);
    //------save resource to disk
    auto file = fopen(outputPath.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", 
            assetRecord->guid, assetRecord->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(buffer.data(), 1, buffer.size(), file);
    return true;
}

uint32_t skd::asset::SMeshCooker::Version() 
{ 
    return kDevelopmentVersion; 
}