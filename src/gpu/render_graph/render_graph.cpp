#include "render_graph/backend/graph_backend.hpp"

namespace sakura
{
namespace render_graph
{
RenderGraph* RenderGraph::create(const RenderGraphSetupFunction& setup)
{
    RenderGraphBuilder builder = {};
    RenderGraph* graph = nullptr;
    setup(builder);
    if (builder.no_backend)
        graph = new RenderGraph();
    else
    {
        if (!builder.gfx_queue) assert(0 && "not supported!");
        graph = new RenderGraphBackend(builder.gfx_queue, builder.device);
    }
    graph->initialize();
    return graph;
}

void RenderGraph::destroy(RenderGraph* g)
{
    g->finalize();
    delete g;
}

const ResourceNode::LifeSpan ResourceNode::lifespan() const
{
    uint32_t from = UINT32_MAX, to = 0;
    foreach_neighbors([&](const DependencyGraphNode* node) {
        auto rg_node = static_cast<const RenderGraphNode*>(node);
        if (rg_node->type == EObjectType::Pass)
        {
            auto pass_node = static_cast<const RenderPassNode*>(node);
            from = (from <= pass_node->order) ? from : pass_node->order;
            to = (to >= pass_node->order) ? to : pass_node->order;
        }
    });
    foreach_inv_neighbors([&](const DependencyGraphNode* node) {
        auto rg_node = static_cast<const RenderGraphNode*>(node);
        if (rg_node->type == EObjectType::Pass)
        {
            auto pass_node = static_cast<const RenderPassNode*>(node);
            from = (from <= pass_node->order) ? from : pass_node->order;
            to = (to >= pass_node->order) ? to : pass_node->order;
        }
    });
    return { from, to };
}

bool RenderGraph::compile()
{
    // TODO: cull
    // 1.init resources states
    for (auto& resource : resources)
    {
        switch (resource->type)
        {
            case EObjectType::Texture: {
                auto texture = static_cast<TextureNode*>(resource);
                // texture->init_state = RESOURCE_STATE_COMMON;
                (void)texture;
            }
            break;
            default:
                break;
        }
    }
    // 2.calc lifespan
    for (auto& pass : passes)
    {
        graph->foreach_incoming_edges(pass,
            [=](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* edge) {
                auto rg_from = static_cast<RenderGraphNode*>(from);
                // auto rg_to = static_cast<RenderGraphNode*>(to);
                if (rg_from->type == EObjectType::Texture)
                {
                    // auto texture_from = static_cast<TextureNode*>(rg_from);
                }
            });
    }
    return true;
}

uint32_t RenderGraph::foreach_writer_passes(TextureHandle texture,
    eastl::function<void(PassNode*, TextureNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_incoming_edges(
        texture,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(from);
            TextureNode* texture = static_cast<TextureNode*>(to);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, texture, edge);
        });
}

uint32_t RenderGraph::foreach_reader_passes(TextureHandle texture,
    eastl::function<void(PassNode*, TextureNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_outgoing_edges(
        texture,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(to);
            TextureNode* texture = static_cast<TextureNode*>(from);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, texture, edge);
        });
}

const ECGpuResourceState RenderGraph::get_lastest_state(
    const TextureNode* texture, const PassNode* pending_pass) const
{
    if (passes[0] == pending_pass)
        return texture->init_state;
    PassNode* pass_iter = nullptr;
    auto result = texture->init_state;
    foreach_writer_passes(texture->get_handle(),
        [&](PassNode* pass, TextureNode* texture, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::TextureWrite)
            {
                auto write_edge = static_cast<TextureRenderEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = write_edge->requested_state;
                }
            }
        });
    foreach_reader_passes(texture->get_handle(),
        [&](PassNode* pass, TextureNode* texture, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::TextureRead)
            {
                auto read_edge = static_cast<TextureRenderEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = read_edge->requested_state;
                }
            }
        });
    return result;
}

uint64_t RenderGraph::execute()
{
    graph->clear();
    return frame_index++;
}

void RenderGraph::initialize()
{
}

void RenderGraph::finalize()
{
}

void RenderGraphBackend::initialize()
{
    backend = device->adapter->instance->backend;
    for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].initialize(gfx_queue, device);
    }
    texture_pool.initialize(device);
    texture_view_pool.initialize(device);
}

void RenderGraphBackend::finalize()
{
    for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].finalize();
    }
    texture_pool.finalize();
    texture_view_pool.finalize();
    for (auto desc_set_heap : desc_set_pool)
    {
        desc_set_heap.second->destroy();
    }
}

CGpuTextureId RenderGraphBackend::resolve(const TextureNode& node)
{
    if (!node.frame_texture)
    {
        auto& wnode = const_cast<TextureNode&>(node);
        auto allocated = texture_pool.allocate(node.descriptor, frame_index);
        wnode.frame_texture = node.imported ?
                                  node.frame_texture :
                                  allocated.first;
        wnode.init_state = allocated.second;
    }
    return node.frame_texture;
}

void RenderGraphBackend::execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass)
{
    RenderPassStack stack = {};
    auto read_edges = pass->read_edges();
    auto write_edges = pass->write_edges();
    CGpuRootSignatureId root_sig = pass->pipeline->root_signature;
    // resource de-virtualize
    eastl::vector<CGpuTextureBarrier> tex_barriers = {};
    for (auto& read_edge : read_edges)
    {
        auto texture_readed = read_edge->get_texture_node();
        auto tex_resolved = resolve(*texture_readed);
        const auto current_state = get_lastest_state(texture_readed, pass);
        const auto dst_state = read_edge->requested_state;
        if (current_state == dst_state) continue;
        CGpuTextureBarrier barrier = {};
        barrier.src_state = current_state;
        barrier.dst_state = dst_state;
        barrier.texture = tex_resolved;
        tex_barriers.emplace_back(barrier);
    }
    for (auto& write_edge : write_edges)
    {
        auto texture_target = write_edge->get_texture_node();
        auto tex_resolved = resolve(*texture_target);
        const auto current_state = get_lastest_state(texture_target, pass);
        const auto dst_state = write_edge->requested_state;
        if (current_state == dst_state) continue;
        CGpuTextureBarrier barrier = {};
        barrier.src_state = current_state;
        barrier.dst_state = dst_state;
        barrier.texture = tex_resolved;
        tex_barriers.emplace_back(barrier);
    }
    // allocate & update descriptor sets
    auto&& desc_set_heap = desc_set_pool.find(root_sig);
    if (desc_set_heap == desc_set_pool.end())
        desc_set_pool.insert({ root_sig, new DescSetHeap(root_sig) });
    stack.desc_sets = desc_set_pool[pass->pipeline->root_signature]->pop();
    for (uint32_t set_idx = 0; set_idx < stack.desc_sets.size(); set_idx++)
    {
        eastl::vector<CGpuDescriptorData> desc_set_updates;
        eastl::vector<CGpuTextureViewId> srvs(read_edges.size());
        for (uint32_t e_idx = 0; e_idx < read_edges.size(); e_idx++)
        {
            auto& read_edge = read_edges[e_idx];
            if (read_edge->set == set_idx)
            {
                auto texture_readed = read_edge->get_texture_node();
                CGpuDescriptorData update = {};
                update.count = 1;
                update.binding = read_edge->binding;
                update.binding_type = RT_TEXTURE;
                CGpuTextureViewDescriptor view_desc = {};
                view_desc.texture = resolve(*texture_readed);
                view_desc.base_array_layer = read_edge->get_array_base();
                view_desc.array_layer_count = read_edge->get_array_count();
                view_desc.base_mip_level = read_edge->get_mip_base();
                view_desc.mip_level_count = read_edge->get_mip_count();
                view_desc.aspects = TVA_COLOR;
                view_desc.format = (ECGpuFormat)view_desc.texture->format;
                view_desc.usages = TVU_SRV;
                view_desc.dims = read_edge->get_dimension();
                srvs[e_idx] = texture_view_pool.allocate(view_desc, frame_index);
                update.textures = &srvs[e_idx];
                desc_set_updates.emplace_back(update);
            }
        }
        auto update_count = desc_set_updates.size();
        if (update_count)
        {
            cgpu_update_descriptor_set(stack.desc_sets[set_idx],
                desc_set_updates.data(), desc_set_updates.size());
        }
    }
    // color attachments
    eastl::vector<CGpuColorAttachment> color_attachments = {};
    for (auto& write_edge : write_edges)
    {
        CGpuColorAttachment attachment = {};
        auto texture_target = write_edge->get_texture_node();
        // TODO: MSAA
        CGpuTextureViewDescriptor view_desc = {};
        view_desc.texture = resolve(*texture_target);
        view_desc.base_array_layer = write_edge->get_array_base();
        view_desc.array_layer_count = write_edge->get_array_count();
        view_desc.base_mip_level = write_edge->get_mip_level();
        view_desc.mip_level_count = 1;
        view_desc.aspects = TVA_COLOR;
        view_desc.format = (ECGpuFormat)view_desc.texture->format;
        view_desc.usages = TVU_RTV_DSV;
        view_desc.dims = TEX_DIMENSION_2D;
        attachment.view = texture_view_pool.allocate(view_desc, frame_index);
        attachment.load_action = pass->load_actions[write_edge->mrt_index];
        attachment.store_action = pass->store_actions[write_edge->mrt_index];
        color_attachments.emplace_back(attachment);
    }
    // call cgpu apis
    if (!tex_barriers.empty())
    {
        CGpuResourceBarrierDescriptor barriers = {};
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = tex_barriers.size();
        cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    }
    // TODO: MSAA
    CGpuRenderPassDescriptor pass_desc = {};
    pass_desc.render_target_count = write_edges.size();
    pass_desc.sample_count = SAMPLE_COUNT_1;
    pass_desc.name = pass->get_name();
    pass_desc.color_attachments = color_attachments.data();
    pass_desc.depth_stencil = nullptr;
    stack.encoder = cgpu_cmd_begin_render_pass(executor.gfx_cmd_buf, &pass_desc);
    cgpu_render_encoder_bind_pipeline(stack.encoder, pass->pipeline);
    for (auto desc_set : stack.desc_sets)
    {
        cgpu_render_encoder_bind_descriptor_set(stack.encoder, desc_set);
    }
    pass->executor(*this, stack);
    cgpu_cmd_end_render_pass(executor.gfx_cmd_buf, stack.encoder);
    // deallocate
    for (auto& read_edge : read_edges)
    {
        auto texture_readed = read_edge->get_texture_node();
        bool is_last_user = true;
        texture_readed->foreach_neighbors(
            [&](DependencyGraphNode* neig) {
                RenderGraphNode* rg_node = (RenderGraphNode*)neig;
                if (rg_node->type == EObjectType::Pass)
                {
                    PassNode* other_pass = (PassNode*)rg_node;
                    is_last_user = is_last_user && (pass->order >= other_pass->order);
                }
            });
        if (is_last_user)
            texture_pool.deallocate(texture_readed->descriptor,
                texture_readed->frame_texture,
                read_edge->requested_state,
                frame_index);
    }
    for (auto& write_edge : write_edges)
    {
        auto texture_target = write_edge->get_texture_node();
        bool is_last_user = true;
        texture_target->foreach_neighbors(
            [&](DependencyGraphNode* neig) {
                RenderGraphNode* rg_node = (RenderGraphNode*)neig;
                if (rg_node->type == EObjectType::Pass)
                {
                    PassNode* other_pass = (PassNode*)rg_node;
                    is_last_user = is_last_user && (pass->order >= other_pass->order);
                }
            });
        if (is_last_user)
            texture_pool.deallocate(texture_target->descriptor,
                texture_target->frame_texture,
                write_edge->requested_state,
                frame_index);
    }
}

void RenderGraphBackend::execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass)
{
    auto read_edges = pass->read_edges();
    auto&& read_edge = read_edges[0];
    auto texture_target = read_edge->get_texture_node();
    CGpuTextureBarrier present_barrier = {};
    present_barrier.texture = pass->descriptor.swapchain->back_buffers[pass->descriptor.index];
    present_barrier.src_state = get_lastest_state(texture_target, pass);
    present_barrier.dst_state = RESOURCE_STATE_PRESENT;
    CGpuResourceBarrierDescriptor barriers = {};
    barriers.texture_barriers = &present_barrier;
    barriers.texture_barriers_count = 1;
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
}

uint64_t RenderGraphBackend::execute()
{
    RenderGraphFrameExecutor& executor = executors[0];
    cgpu_reset_command_pool(executor.gfx_cmd_pool);
    cgpu_cmd_begin(executor.gfx_cmd_buf);
    for (auto& pass : passes)
    {
        if (pass->pass_type == EPassType::Render)
        {
            execute_render_pass(executor, static_cast<RenderPassNode*>(pass));
        }
        else if (pass->pass_type == EPassType::Present)
        {
            execute_present_pass(executor, static_cast<PresentPassNode*>(pass));
        }
    }
    cgpu_cmd_end(executor.gfx_cmd_buf);
    // submit
    CGpuQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &executor.gfx_cmd_buf;
    submit_desc.cmds_count = 1;
    cgpu_submit_queue(gfx_queue, &submit_desc);
    graph->clear();
    blackboard.clear();
    for (auto pass : passes)
    {
        delete pass;
    }
    passes.clear();
    for (auto resource : resources)
    {
        delete resource;
    }
    resources.clear();
    // cleanup internal resources
    for (auto desc_heap : desc_set_pool)
    {
        desc_heap.second->reset();
    }
    return frame_index++;
}
} // namespace render_graph
} // namespace sakura