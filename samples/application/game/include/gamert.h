#pragma once
#include "GameRT/module.configure.h"
#include "platform/vfs.h"
#include "utils/log.h"
#include "skr_renderer/skr_renderer.h"
#ifdef __cplusplus
    #include "module/module_manager.hpp"
    #include "render_graph/frontend/render_graph.hpp"
    #include "utils/io.hpp"
    #include "cgpu/io.hpp"

struct dual_storage_t;
namespace skr::resource
{
struct SLocalResourceRegistry;
}
class GAMERT_API SGameRTModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;

public:
    static SGameRTModule* Get();

public:
    skr_vfs_t* resource_vfs = nullptr;
    skr_vfs_t* tex_resource_vfs = nullptr;
    skr::io::RAMService* ram_service = nullptr;
    skr::resource::SLocalResourceRegistry* registry;

    struct dual_storage_t* game_world = nullptr;
    SRenderDeviceId game_render_device = nullptr;
    SRendererId game_renderer = nullptr;
};

namespace skg
{
struct GameContext {
};
GAMERT_API bool GameLoop(GameContext& ctx);
} // namespace skg
#endif

GAMERT_EXTERN_C GAMERT_API skr_vfs_t* skr_game_runtime_get_vfs();
GAMERT_EXTERN_C GAMERT_API skr_io_ram_service_t* skr_game_runtime_get_ram_service();
GAMERT_EXTERN_C GAMERT_API dual_storage_t* skr_game_runtime_get_world();
GAMERT_EXTERN_C GAMERT_API SRenderDeviceId skr_game_runtime_get_render_device();
GAMERT_EXTERN_C GAMERT_API SRendererId skr_game_runtime_get_renderer();