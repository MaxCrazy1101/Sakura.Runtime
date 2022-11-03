#include "gamert.h"
#include "utils/make_zeroed.hpp"
#include "platform/configure.h"
#include "platform/filesystem.hpp"
#include "platform/memory.h"
#include "resource/resource_system.h"
#include "resource/local_resource_registry.h"
#include "ecs/dual.h"
#include "runtime_module.h"
#include "platform/guid.hpp"
#include "skr_renderer/resources/texture_resource.h"

IMPLEMENT_DYNAMIC_MODULE(SGameRTModule, GameRT);


skr::resource::STextureFactory testFactory = {};

void SGameRTModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("game runtime loaded!");
    std::error_code ec = {};
    auto resourceRoot = (skr::filesystem::current_path(ec) / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);

    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = "GameRuntimeRAMIOService";
    ioServiceDesc.sleep_mode = SKR_IO_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_IO_SERVICE_SORT_METHOD_PARTIAL;
    ram_service = skr::io::RAMService::create(&ioServiceDesc);

    registry = SkrNew<skr::resource::SLocalResourceRegistry>(resource_vfs);
    skr::resource::GetResourceSystem()->Initialize(registry, ram_service);
    // 
    using namespace skr::guid::literals;
    auto resource_system = skr::resource::GetResourceSystem();
    skr::resource::STextureFactory testFactory = {};
    resource_system->RegisterFactory("f8821efb-f027-4367-a244-9cc3efb3a3bf"_guid, &testFactory);
    skr_resource_handle_t textureHdl("cb5fe6d7-5d91-4f3b-81b0-0a7afbf1a7cb"_guid);
    resource_system->LoadResource(textureHdl);
    while (textureHdl.get_status() != SKR_LOADING_STATUS_INSTALLED && 
        textureHdl.get_status() != SKR_LOADING_STATUS_ERROR)
    {
        auto status = textureHdl.get_status();
        resource_system->Update();
    }
    auto final_status = textureHdl.get_status();
    if (final_status != SKR_LOADING_STATUS_ERROR)
    {
        auto texture = (skr_texture_resource_t*)textureHdl.get_ptr();
        SKR_LOG_DEBUG("Texture Loaded: format - %d, mips - %d, data size - %d", 
            texture->format, texture->mips_count, texture->data_size); 
        resource_system->UnloadResource(textureHdl);
    }
}

void SGameRTModule::on_unload()
{
    skr::resource::GetResourceSystem()->Shutdown();
    SkrDelete(registry);

    skr::io::RAMService::destroy(ram_service);
    skr_free_vfs(resource_vfs);

    SKR_LOG_INFO("game runtime unloaded!");
}

SGameRTModule* SGameRTModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SGameRTModule*>(mm->get_module("GameRT"));
    return rm;
}

RUNTIME_EXTERN_C skr_vfs_t* skr_game_runtime_get_vfs()
{
    return SGameRTModule::Get()->resource_vfs;
}


RUNTIME_EXTERN_C skr_io_ram_service_t* skr_game_runtime_get_ram_service()
{
     return SGameRTModule::Get()->ram_service;
}
