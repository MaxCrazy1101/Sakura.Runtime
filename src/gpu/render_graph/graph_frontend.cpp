#include "render_graph/frontend/render_graph.hpp"
#include "../cgpu/common/common_utils.h"

namespace sakura
{
namespace render_graph
{
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
    // 1.cull
    resources.erase(
        eastl::remove_if(resources.begin(), resources.end(),
            [](ResourceNode* resource) {
                const bool lone = !(resource->incoming_edges() + resource->outgoing_edges());
                return lone;
            }),
        resources.end());
    passes.erase(
        eastl::remove_if(passes.begin(), passes.end(),
            [](PassNode* pass) {
                const bool lone = !(pass->incoming_edges() + pass->outgoing_edges());
                return lone;
            }),
        passes.end());
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

uint32_t RenderGraph::foreach_writer_passes(BufferHandle buffer,
    eastl::function<void(PassNode*, BufferNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_incoming_edges(
        buffer,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(from);
            BufferNode* buffer = static_cast<BufferNode*>(to);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, buffer, edge);
        });
}

uint32_t RenderGraph::foreach_reader_passes(BufferHandle buffer,
    eastl::function<void(PassNode*, BufferNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_outgoing_edges(
        buffer,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(to);
            BufferNode* buffer = static_cast<BufferNode*>(from);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, buffer, edge);
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
            else if (edge->type == ERelationshipType::TextureReadWrite)
            {
                auto rw_edge = static_cast<TextureReadWriteEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = rw_edge->requested_state;
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

const ECGpuResourceState RenderGraph::get_lastest_state(
    const BufferNode* buffer, const PassNode* pending_pass) const
{
    if (passes[0] == pending_pass)
        return buffer->init_state;
    PassNode* pass_iter = nullptr;
    auto result = buffer->init_state;
    foreach_writer_passes(buffer->get_handle(),
        [&](PassNode* pass, BufferNode* buffer, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::BufferReadWrite)
            {
                auto write_edge = static_cast<BufferReadWriteEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = write_edge->requested_state;
                }
            }
        });
    foreach_reader_passes(buffer->get_handle(),
        [&](PassNode* pass, BufferNode* buffer, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::BufferRead)
            {
                auto read_edge = static_cast<BufferReadEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = read_edge->requested_state;
                }
            }
            else if (edge->type == ERelationshipType::PipelineBuffer)
            {
                auto ppl_edge = static_cast<PipelineBufferEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = ppl_edge->requested_state;
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
} // namespace render_graph
} // namespace sakura