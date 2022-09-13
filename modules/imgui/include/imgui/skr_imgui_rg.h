#pragma once
#include "platform/window.h"
#include "platform/input.h"
#include "skr_imgui.config.h"
#include "render_graph/frontend/render_graph.hpp"

typedef struct RenderGraphImGuiDescriptor {
    skr::render_graph::RenderGraph* render_graph;
    CGPUQueueId queue;
    CGPUSamplerId static_sampler;
    CGPUPipelineShaderDescriptor vs;
    CGPUPipelineShaderDescriptor ps;
    ECGPUFormat backbuffer_format;
} RenderGraphImGuiDescriptor;

RUNTIME_EXTERN_C SKR_IMGUI_API 
void render_graph_imgui_initialize(const RenderGraphImGuiDescriptor* desc);

RUNTIME_EXTERN_C SKR_IMGUI_API 
void render_graph_imgui_add_render_pass(skr::render_graph::RenderGraph* render_graph,
    skr::render_graph::TextureRTVHandle target, ECGPULoadAction load_action);

RUNTIME_EXTERN_C SKR_IMGUI_API 
void render_graph_imgui_present_sub_viewports();

RUNTIME_EXTERN_C SKR_IMGUI_API 
void render_graph_imgui_finalize();