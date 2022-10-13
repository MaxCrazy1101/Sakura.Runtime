#include "gamert.h"
#include "EASTL/shared_ptr.h"
#include "gainput/GainputInputDevicePad.h"
#include "platform/configure.h"
#include "ghc/filesystem.hpp"
#include "platform/memory.h"
#include "resource/resource_system.h"
#include "resource/local_resource_registry.h"
#include "ecs/dual.h"
#include "../../cgpu/common/utils.h"
#include "ghc/filesystem.hpp"
#include "platform/time.h"
#include "platform/window.h"
#include "resource/local_resource_registry.h"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/skr_imgui_rg.h"
#include "imgui/imgui.h"
#include "resource/resource_system.h"
#include "skr_input/Interactions.h"
#include "skr_input/InteractionsType.h"
#include "utils/make_zeroed.hpp"
#include "skr_scene/scene.h"
#include "skr_renderer/skr_renderer.h"
#include "gainput/gainput.h"
#include "gainput/GainputInputDeviceKeyboard.h"
#include "gainput/GainputInputDeviceMouse.h"
#include "gamert.h"
#include "ecs/callback.hpp"
#include "ecs/type_builder.hpp"
#include "skr_input/inputSystem.h"
#include "skr_renderer/render_mesh.h"
#include "math/vector.hpp"
#include "task/task.hpp"

#include "tracy/Tracy.hpp"

SWindowHandle window;
uint32_t backbuffer_index;
extern void create_imgui_resources(SRenderDeviceId render_device, skr::render_graph::RenderGraph* renderGraph);
extern void game_initialize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph);
extern void game_finalize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph);
#define lerp(a, b, t) (a) + (t) * ((b) - (a))

const bool bUseJob = true;
const bool bUseInputSystem = false;

// TODO: Refactor this
CGPUVertexLayout vertex_layout = {};

class SGameModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

    struct dual_storage_t* game_world = nullptr;

    skr::task::scheduler_t scheduler;
    SRendererId game_renderer = nullptr;
};

IMPLEMENT_DYNAMIC_MODULE(SGameModule, Game);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "Game",
    "prettyname" : "Game",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"GameRT", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
Game)

void SGameModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("game runtime loaded!");

    game_world = dualS_create();

    auto render_device = skr_get_default_render_device();
    game_renderer = skr_create_renderer(render_device, game_world);

    if (bUseJob)
    {
        auto options = make_zeroed<skr::task::scheudler_config_t>();
        options.numThreads = 0;
        scheduler.initialize(options);
        scheduler.bind();
    }
    // TODO: Refactor this
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "NORMAL", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 2, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "TANGENT", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 3, 0, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attribute_count = 3;
}

void create_test_scene(SRendererId renderer)
{
    // allocate 100 movable cubes
    auto renderableT_builder = make_zeroed<dual::type_builder_t>();
    renderableT_builder
        .with<skr_translation_t, skr_rotation_t, skr_scale_t, skr_movement_t>()
        .with<skr_render_effect_t>();
    // allocate renderable
    auto renderableT = make_zeroed<dual_entity_type_t>();
    renderableT.type = renderableT_builder.build();
    auto primSetup = [&](dual_chunk_view_t* view) {
        auto translations = (skr_translation_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_t>::get());
        auto rotations = (skr_rotation_t*)dualV_get_owned_ro(view, dual_id_of<skr_rotation_t>::get());
        auto scales = (skr_scale_t*)dualV_get_owned_ro(view, dual_id_of<skr_scale_t>::get());
        auto movements = (skr_movement_t*)dualV_get_owned_ro(view, dual_id_of<skr_movement_t>::get());
        for (uint32_t i = 0; i < view->count; i++)
        {
            if (movements)
            {
                translations[i].value = {
                    (float)(i % 10) * 1.5f, ((float)i / 10) * 1.5f + 50.f, 0.f
                };
                rotations[i].euler = { 0.f, 0.f, 0.f };
                scales[i].value = { 8.f, 8.f, 8.f };
            }
            else
            {
                translations[i].value = { 0.f, 0.f, 0.f };
                rotations[i].euler = { 3.1415926f / 2.f, 0.f, 0.f };
                scales[i].value = { 1.f, 1.f, 1.f };
            }
        }

        auto feature_arrs = dualV_get_owned_rw(view, dual_id_of<skr_render_effect_t>::get());
        if(feature_arrs)
            skr_render_effect_attach(renderer, view, "ForwardEffect");
    };
    dualS_allocate_type(renderer->get_dual_storage(), &renderableT, 512, DUAL_LAMBDA(primSetup));

    SKR_LOG_DEBUG("Create Scene 0!");

    // allocate 1 player entity
    auto playerT_builder = make_zeroed<dual::type_builder_t>();
    playerT_builder
        .with<skr_translation_t, skr_rotation_t, skr_scale_t, skr_movement_t>()
        .with<skr_camera_t>();
    auto playerT = make_zeroed<dual_entity_type_t>();
    playerT.type = playerT_builder.build();
    dualS_allocate_type(renderer->get_dual_storage(), &playerT, 1, DUAL_LAMBDA(primSetup));

    SKR_LOG_DEBUG("Create Scene 1!");

    // allocate 1 static(unmovable) gltf mesh
    auto static_renderableT_builderT = make_zeroed<dual::type_builder_t>();
    static_renderableT_builderT
        .with<skr_translation_t, skr_rotation_t, skr_scale_t>()
        .with<skr_render_effect_t>();
    auto static_renderableT = make_zeroed<dual_entity_type_t>();
    static_renderableT.type = static_renderableT_builderT.build();
    dualS_allocate_type(renderer->get_dual_storage(), &static_renderableT, 1, DUAL_LAMBDA(primSetup));

    SKR_LOG_DEBUG("Create Scene 2!");
}

void attach_mesh_on_static_ents(SRendererId renderer, skr_io_ram_service_t* ram_service, skr_io_vram_service_t* vram_service, 
    const char* path, skr_render_mesh_request_t* request)
{
    auto filter = make_zeroed<dual_filter_t>();
    auto meta = make_zeroed<dual_meta_filter_t>();
    auto renderable_type = make_zeroed<dual::type_builder_t>();
    renderable_type.with<skr_render_effect_t, skr_translation_t>();
    auto static_type = make_zeroed<dual::type_builder_t>();
    static_type.with<skr_movement_t>();
    filter.all = renderable_type.build();
    filter.none = static_type.build();
    auto attchFunc = [=](dual_chunk_view_t* view) {
        auto ents = (dual_entity_t*)dualV_get_entities(view);
        auto requestSetup = [=](dual_chunk_view_t* view) {
            auto mesh_comps = (skr_render_mesh_comp_t*)dualV_get_owned_rw(view, dual_id_of<skr_render_mesh_comp_t>::get());
            mesh_comps->async_request = *request;
            auto render_device = renderer->get_render_device();
            skr_render_mesh_create_from_gltf(render_device, ram_service, vram_service, path, &mesh_comps->async_request);
        };
        skr_render_effect_access(renderer, ents, view->count, "ForwardEffect", DUAL_LAMBDA(requestSetup));
    };
    dualS_query(renderer->get_dual_storage(), &filter, &meta, DUAL_LAMBDA(attchFunc));
}

const char* gltf_file = "scene.gltf";
const char* gltf_file2 = "scene.gltf";
void imgui_button_spawn_girl(SRendererId renderer)
{
    static bool onceGuard  = true;
    if (onceGuard)
    {
        auto render_device = skr_get_default_render_device();
        auto girl_mesh_request = make_zeroed<skr_render_mesh_request_t>();
        ImGui::Begin(u8"AsyncMesh");
        auto dstroage_queue = render_device->get_file_dstorage_queue();
        auto resource_vfs = skr_game_runtime_get_vfs();
        auto ram_service = skr_game_runtime_get_ram_service();
        auto vram_service = render_device->get_vram_service();
        girl_mesh_request.mesh_name = gltf_file2;
        girl_mesh_request.mesh_resource_request.shuffle_layout = &vertex_layout;
        if (dstroage_queue && ImGui::Button(u8"LoadMesh(DirectStorage[Disk])"))
        {
            girl_mesh_request.mesh_resource_request.vfs_override = resource_vfs;
            girl_mesh_request.dstorage_queue_override = dstroage_queue;
            girl_mesh_request.dstorage_source = CGPU_DSTORAGE_SOURCE_FILE;
            attach_mesh_on_static_ents(renderer, ram_service, vram_service, gltf_file2, &girl_mesh_request);
            onceGuard = false;
        }
        else if (dstroage_queue && ImGui::Button(u8"LoadMesh(DirectStorage[Memory])"))
        {
            girl_mesh_request.mesh_resource_request.vfs_override = resource_vfs;
            girl_mesh_request.dstorage_queue_override = dstroage_queue;
            girl_mesh_request.dstorage_source = CGPU_DSTORAGE_SOURCE_MEMORY;
            attach_mesh_on_static_ents(renderer, ram_service, vram_service, gltf_file2, &girl_mesh_request);
            onceGuard = false;
        }
        else if (ImGui::Button(u8"LoadMesh(CopyQueue)"))
        {
            girl_mesh_request.mesh_resource_request.vfs_override = resource_vfs;
            girl_mesh_request.queue_override = render_device->get_cpy_queue();
            attach_mesh_on_static_ents(renderer, ram_service, vram_service, gltf_file2, &girl_mesh_request);
            onceGuard = false;
        }
        else if (ImGui::Button(u8"LoadMesh(GraphicsQueue)"))
        {
            girl_mesh_request.mesh_resource_request.vfs_override = resource_vfs;
            girl_mesh_request.queue_override = render_device->get_cpy_queue();
            attach_mesh_on_static_ents(renderer, ram_service, vram_service, gltf_file2, &girl_mesh_request);
            onceGuard = false;
        }
        ImGui::End();  
    }
}

#ifdef _WIN32
#include "cgpu/extensions/cgpu_d3d12_exts.h"
#endif

int SGameModule::main_module_exec(int argc, char** argv)
{
    SKR_LOG_INFO("game executed as main module!");
    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
        return -1;
    auto render_device = skr_get_default_render_device();
    auto cgpu_device = render_device->get_cgpu_device();
    auto gfx_queue = render_device->get_gfx_queue();
    auto window_desc = make_zeroed<SWindowDescroptor>();
    window_desc.flags = SKR_WINDOW_CENTERED | SKR_WINDOW_RESIZABLE;// | SKR_WINDOW_BOARDLESS;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    window = skr_create_window(
        fmt::format("Game [{}]", gCGPUBackendNames[cgpu_device->adapter->instance->backend]).c_str(),
        &window_desc);
    // Initialize renderer
    auto swapchain = skr_render_device_register_window(render_device, window);
    auto present_fence = cgpu_create_fence(cgpu_device);
    namespace render_graph = skr::render_graph;
    auto renderGraph = render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(cgpu_device)
            .with_gfx_queue(gfx_queue)
            .enable_memory_aliasing();
    });
    game_initialize_render_effects(game_renderer, renderGraph);
    create_test_scene(game_renderer);
    create_imgui_resources(render_device, renderGraph);
    // Initialize Input
    skr::input::InputSystem inputSystem;
    if (bUseInputSystem)
    {
        SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
        using namespace skr::input;
        using namespace gainput;
        inputSystem.Init(window);
        inputSystem.SetDisplaySize(BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT);
        // InputAction
        {
            auto action = eastl::make_shared<InputAction<float>>();
            auto controls1 = eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeySpace);
            controls1->AddInteraction(eastl::make_shared<InteractionTap<float>>());
            action->AddControl(controls1);
            action->ListenEvent([](float value, ControlsBase<float>* _c, Interaction* i, Interaction::EvendId eventId)
            {
                SKR_LOG_DEBUG("Tap_Float %f", value);
            });
            inputSystem.AddInputAction(action);
        }
        {
            auto action = eastl::make_shared<InputAction<float>>();
            auto controls1 = eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyP);
            auto interactionPress = eastl::make_shared<InteractionPress<float>>(PressBehavior::PressAndRelease, 0.5f);
            controls1->AddInteraction(interactionPress);
            action->AddControl(controls1);
            action->ListenEvent([interactionPress](float value, ControlsBase<float>* _c, Interaction* i, Interaction::EvendId eventId)
            {
                if(interactionPress.get() == i)
                {
                    //if(((InteractionPress<float>*)i)->GetPressEventType(eventId) == PressEventType::Press)
                    //    SKR_LOG_DEBUG("Press_Float Press %f", value);
                    //else
                    //    SKR_LOG_DEBUG("Press_Float Release %f", value);
                }
            });
            inputSystem.AddInputAction(action);
        }
        {
            auto action = eastl::make_shared<InputAction<skr::math::Vector2f>>();
            auto controls1 = eastl::make_shared<Vector2Control>();
            controls1->Bind(
                Vector2Control::ButtonDirection{
                    eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyW),
                    eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyS),
                    eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyA),
                    eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyD),
                }
            );
            controls1->Bind(
                Vector2Control::StickDirection{
                    eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_PAD, PadButtonLeftStickX),
                    eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_PAD, PadButtonLeftStickY),
                }
            );
            auto interactionPress = eastl::make_shared<InteractionPress<skr::math::Vector2f>>(PressBehavior::PressAndRelease, 0.5f);
            controls1->AddInteraction(interactionPress);
            action->AddControl(controls1);
            action->ListenEvent([interactionPress](skr::math::Vector2f value, ControlsBase<skr::math::Vector2f>* _c, Interaction* i, Interaction::EvendId eventId)
            {
                if(interactionPress.get() == i)
                {
                    //if(((InteractionPress<skr::math::Vector2f>*)i)->GetPressEventType(eventId) == PressEventType::Press)
                    //    SKR_LOG_DEBUG("Press_Float Press    x:%f,y:%f", value.X, value.Y);
                    //else
                    //    SKR_LOG_DEBUG("Press_Float Release  x:%f,y:%f", value.X, value.Y);
                }
            });
            inputSystem.AddInputAction(action);
        }
    }
    // Time
    SHiresTimer tick_timer;
    int64_t elapsed_us = 0;
    int64_t elapsed_frame = 0;
    int64_t fps = 60;
    skr_init_hires_timer(&tick_timer);
    // loop
    bool quit = false;
    dual_query_t* moveQuery;
    dual_query_t* cameraQuery;
    moveQuery = dualQ_from_literal(game_world, 
        "[has]skr_movement_t, [inout]skr_translation_t, [in]skr_scale_t, !skr_camera_t");
    cameraQuery = dualQ_from_literal(game_world, 
        "[has]skr_movement_t, [inout]skr_translation_t, [inout]skr_camera_t");
    while (!quit)
    {
        FrameMark
        ZoneScopedN("LoopBody");
        static auto main_thread_id = skr_current_thread_id();
        auto current_thread_id = skr_current_thread_id();
        SKR_ASSERT(main_thread_id == current_thread_id && "This is not the main thread");

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ZoneScopedN("PollEvent");
            ImGuiIO& io = ImGui::GetIO();

            if (event.type == SDL_WINDOWEVENT)
            {
                Uint8 window_event = event.window.event;
                if (SDL_GetWindowID((SDL_Window*)window) == event.window.windowID)
                {
                    if (window_event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        cgpu_wait_queue_idle(gfx_queue);
                        cgpu_wait_fences(&present_fence, 1);
                        swapchain = skr_render_device_recreate_window_swapchain(render_device, window);
                    }
                    if (window_event == SDL_WINDOWEVENT_CLOSE)
                    {
                        quit = true;
                        break;
                    }
                }
                    
                if (window_event == SDL_WINDOWEVENT_CLOSE || window_event == SDL_WINDOWEVENT_MOVED || window_event == SDL_WINDOWEVENT_RESIZED)
                    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)SDL_GetWindowFromID(event.window.windowID)))
                    {
                        if (window_event == SDL_WINDOWEVENT_CLOSE)
                            viewport->PlatformRequestClose = true;
                        if (window_event == SDL_WINDOWEVENT_MOVED)
                            viewport->PlatformRequestMove = true;
                        if (window_event == SDL_WINDOWEVENT_RESIZED)
                            viewport->PlatformRequestResize = true;
                    }
            }

            if (event.type == SDL_SYSWMEVENT)
            {
                SDL_SysWMmsg* msg = event.syswm.msg;
                if (bUseInputSystem)
                {
#if defined(GAINPUT_PLATFORM_WIN)
                inputSystem.GetHardwareManager().HandleMessage((MSG&)msg->msg);
#elif defined(GAINPUT_PLATFORM_LINUX)
                inputSystem.GetHardwareManager().HandleMessage((XEvent&)msg->msg);
#endif
                }
            }
            if (event.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
        }
        int64_t us = skr_hires_timer_get_usec(&tick_timer, true);
        double deltaTime = (double)us / 1000 / 1000;
        elapsed_us += us;
        elapsed_frame += 1;
        if (elapsed_us > (1000 * 1000))
        {
            fps = elapsed_frame;
            elapsed_frame = 0;
            elapsed_us = 0;
        }
        // Update camera
        auto cameraUpdate = [=](dual_chunk_view_t* view){
            auto cameras = (skr_camera_t*)dualV_get_owned_rw(view, dual_id_of<skr_camera_t>::get());
            for (uint32_t i = 0; i < view->count; i++)
            {
                cameras[i].viewport_width = swapchain->back_buffers[0]->width;
                cameras[i].viewport_height = swapchain->back_buffers[0]->height;
            }
        };
        dualQ_get_views(cameraQuery, DUAL_LAMBDA(cameraUpdate));
        // Input
        if (bUseInputSystem)
        {
            ZoneScopedN("Input");
        
            inputSystem.Update(deltaTime);
        }
        {
            ZoneScopedN("ImGUI");

            skr_imgui_new_frame(window, deltaTime);
            {
                ImGui::Begin(u8"Information");
                ImGui::Text("RenderFPS: %d", (uint32_t)fps);
                ImGui::End();
            }
            imgui_button_spawn_girl(game_renderer);
            // quit |= skg::GameLoop(ctx);
        }
        // move
        // [has]skr_movement_t, [inout]skr_translation_t, [in]skr_scale_t, !skr_camera_t
        if (bUseJob)
        {
            ZoneScopedN("MoveSystem");
            auto timer = clock();
            auto total_sec = (double)timer / CLOCKS_PER_SEC;
            
            auto moveJob = SkrNewLambda([=](dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex) {
                ZoneScopedN("MoveJob");
                
                float lerps[] = { 12.5, 20 };
                auto translations = (skr_translation_t*)dualV_get_owned_rw_local(view, localTypes[0]);
                auto scales = (skr_scale_t*)dualV_get_owned_ro_local(view, localTypes[1]);
                (void)scales;
                for (uint32_t i = 0; i < view->count; i++)
                {
                    auto lscale = (float)abs(sin(total_sec * 0.5));
                    lscale = (float)lerp(lerps[0], lerps[1], lscale);
                    const auto col = (i % 10);
                    const auto row = (i / 10);
                    translations[i].value = {
                        ((float)col - 4.5f) * lscale,
                        ((float)row - 4.5f) * lscale + 50.f, 
                        0.f
                    };
                }
            });
            dualJ_schedule_ecs(moveQuery, 128, DUAL_LAMBDA_POINTER(moveJob), nullptr, nullptr);
        }
        // [has]skr_movement_t, [inout]skr_translation_t, [in]skr_camera_t
        if (bUseJob)
        {
            ZoneScopedN("PlayerSystem");

            auto playerJob = SkrNewLambda([=](dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex) {
                ZoneScopedN("PlayerJob");
                
                auto translations = (skr_translation_t*)dualV_get_owned_rw_local(view, localTypes[0]);
                auto forward = skr::math::Vector3f(0.f, 1.f, 0.f);
                auto right = skr::math::Vector3f(1.f, 0.f, 0.f);
                for (uint32_t i = 0; i < view->count; i++)
                {
                    const auto kSpeed = 15.f;
                    auto qdown = skr_key_down(EKeyCode::KEY_CODE_Q);
                    auto edown = skr_key_down(EKeyCode::KEY_CODE_E);
                    auto wdown = skr_key_down(EKeyCode::KEY_CODE_W);
                    auto sdown = skr_key_down(EKeyCode::KEY_CODE_S);
                    auto adown = skr_key_down(EKeyCode::KEY_CODE_A);
                    auto ddown = skr_key_down(EKeyCode::KEY_CODE_D);
                    if (qdown) translations[i].value.z += (float)deltaTime * kSpeed;
                    if (edown) translations[i].value.z -= (float)deltaTime * kSpeed;
                    if (wdown) translations[i].value = forward * (float)deltaTime * kSpeed + translations[i].value;
                    if (sdown) translations[i].value = -1.f * forward * (float)deltaTime * kSpeed + translations[i].value;
                    if (adown) translations[i].value = -1.f * right * (float)deltaTime * kSpeed + translations[i].value;
                    if (ddown) translations[i].value = 1.f * right * (float)deltaTime * kSpeed + translations[i].value;
                }
            });
            dualJ_schedule_ecs(cameraQuery, 128, DUAL_LAMBDA_POINTER(playerJob), nullptr, nullptr);
            {
                ZoneScopedN("DualJSync");
                dualJ_wait_all();
            }
        }
        {
            ZoneScopedN("AcquireFrame");

            // acquire frame
            cgpu_wait_fences(&present_fence, 1);
            CGPUAcquireNextDescriptor acquire_desc = {};
            acquire_desc.fence = present_fence;
            backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
        }
        // render graph setup & compile & exec
        CGPUTextureId native_backbuffer = swapchain->back_buffers[backbuffer_index];
        auto back_buffer = renderGraph->create_texture(
        [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
            builder.set_name("backbuffer")
            .import(native_backbuffer, CGPU_RESOURCE_STATE_UNDEFINED)
            .allow_render_target();
        });
        {
            ZoneScopedN("RenderScene");
            skr_renderer_render_frame(game_renderer, renderGraph);
        }
        {
            ZoneScopedN("RenderIMGUI");
            render_graph_imgui_add_render_pass(renderGraph, back_buffer, CGPU_LOAD_ACTION_LOAD);
        }
        renderGraph->add_present_pass(
        [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
            builder.set_name("present_pass")
            .swapchain(swapchain, backbuffer_index)
            .texture(back_buffer, true);
        });
        {
            ZoneScopedN("CompileRenderGraph");
            renderGraph->compile();
        }
        {
            ZoneScopedN("ExecuteRenderGraph");
            auto frame_index = renderGraph->execute();
            {
                ZoneScopedN("CollectGarbage");
                if ( (frame_index > (RG_MAX_FRAME_IN_FLIGHT * 10)) && (frame_index % (RG_MAX_FRAME_IN_FLIGHT * 10) == 0))
                    renderGraph->collect_garbage(frame_index - 10 * RG_MAX_FRAME_IN_FLIGHT);
            }
        }
        {
            ZoneScopedN("QueuePresentSwapchain");
            // present
            CGPUQueuePresentDescriptor present_desc = {};
            present_desc.index = backbuffer_index;
            present_desc.swapchain = swapchain;
            cgpu_queue_present(gfx_queue, &present_desc);
            render_graph_imgui_present_sub_viewports();
        }
    }
    // clean up
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    render_graph::RenderGraph::destroy(renderGraph);
    game_finalize_render_effects(game_renderer, renderGraph);
    render_graph_imgui_finalize();
    skr_free_renderer(game_renderer);
    skr_free_window(window);
    SDL_Quit();
    return 0;
}

void SGameModule::on_unload()
{
    SKR_LOG_INFO("game unloaded!");

    dualS_release(game_world);
}