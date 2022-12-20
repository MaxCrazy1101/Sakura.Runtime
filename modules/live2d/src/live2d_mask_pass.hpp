#pragma once
#include "SkrRenderGraph/api.h"
#include "SkrRenderer/primitive_pass.h"
#include "live2d_helpers.hpp"

#include "tracy/Tracy.hpp"

const skr_render_pass_name_t live2d_mask_pass_name = "Live2DMaskPass";

struct MaskPassLive2D : public IPrimitiveRenderPass {
    void on_update(const skr_primitive_pass_context_t* context) override
    {
        auto renderGraph = context->render_graph;

        auto live2d_mask_msaa = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value("l2d_msaa", sample_level);

            builder.set_name("live2d_mask_msaa")
                .extent(Csm::kMaskResolution, Csm::kMaskResolution)
                .format(live2d_mask_format)
                .owns_memory()
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_render_target();
        });(void)live2d_mask_msaa;

        auto mask = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            builder.set_name("live2d_mask")
                .extent(Csm::kMaskResolution, Csm::kMaskResolution)
                .format(live2d_mask_format)
                .owns_memory()
                .allow_render_target();
        });(void)mask;
        
        auto depth = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value("l2d_msaa", sample_level);

            builder.set_name("mask_depth")
                .extent(Csm::kMaskResolution, Csm::kMaskResolution)
                .format(live2d_depth_format)
                .owns_memory()
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_depth_stencil();
        });(void)depth;
    }

    void post_update(const skr_primitive_pass_context_t* context) override
    {

    }

    void execute(const skr_primitive_pass_context_t* context, skr_primitive_draw_list_view_t drawcalls) override
    {
        auto renderGraph = context->render_graph;
        if (drawcalls.count)
        {
            renderGraph->add_render_pass(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
                double sample_level = 1.0;
                bool useMSAA = g.get_blackboard().value("l2d_msaa", sample_level); useMSAA &= (sample_level > 1.0);
                
                const auto mask = renderGraph->get_texture("live2d_mask");
                const auto mask_msaa = renderGraph->get_texture("live2d_mask_msaa");
                const auto depth_buffer = renderGraph->get_texture("mask_depth");
                builder.set_name("live2d_mask_pass")
                    // we know that the drawcalls always have a same pipeline
                    .set_pipeline(drawcalls.drawcalls->pipeline)
                    .write(0, useMSAA ? mask_msaa : mask, CGPU_LOAD_ACTION_CLEAR, fastclear_1111)
                    .set_depth_stencil(depth_buffer.clear_depth(1.f));
                if (useMSAA)
                {
                    builder.resolve_msaa(0, mask);
                }
            },
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& pass_context) {
                for (uint32_t i = 0; i < drawcalls.count; i++)
                {
                    ZoneScopedN("UpdateTextures");
                    auto&& dc = drawcalls.drawcalls[i];
                    if (dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) continue;

                    CGPUXBindTableId tables[2] = { dc.bind_table, pass_context.bind_table };
                    pass_context.merge_tables(tables, 2);
                }
                cgpu_render_encoder_set_viewport(pass_context.encoder,
                    0.0f, 0.0f,
                    (float)Csm::kMaskResolution, (float)Csm::kMaskResolution,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(pass_context.encoder, 0, 0, Csm::kMaskResolution, Csm::kMaskResolution);
                for (uint32_t i = 0; i < drawcalls.count; i++)
                {
                    ZoneScopedN("DrawCall");

                    auto&& dc = drawcalls.drawcalls[i];
                    if (dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) continue;
                    {
                        ZoneScopedN("BindTextures");

                        CGPUXBindTableId tables[2] = { dc.bind_table, pass_context.bind_table };
                        pass_context.merge_and_bind_tables(tables, 2);
                    }
                    {
                        ZoneScopedN("BindGeometry");
                        cgpu_render_encoder_bind_index_buffer(pass_context.encoder, dc.index_buffer.buffer, dc.index_buffer.stride, dc.index_buffer.offset);
                        CGPUBufferId vertex_buffers[2] = {
                            dc.vertex_buffers[0].buffer, dc.vertex_buffers[1].buffer
                        };
                        const uint32_t strides[2] = {
                            dc.vertex_buffers[0].stride, dc.vertex_buffers[1].stride
                        };
                        const uint32_t offsets[2] = {
                            dc.vertex_buffers[0].offset, dc.vertex_buffers[1].offset
                        };
                        cgpu_render_encoder_bind_vertex_buffers(pass_context.encoder, 2, vertex_buffers, strides, offsets);
                    }
                    {
                        ZoneScopedN("PushConstants");
                        cgpu_render_encoder_push_constants(pass_context.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                    }
                    {
                        ZoneScopedN("DrawIndexed");
                        cgpu_render_encoder_draw_indexed_instanced(pass_context.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0);
                    }
                }
            });
        }
    }

    skr_render_pass_name_t identity() const override
    {
        return live2d_mask_pass_name;
    }
};