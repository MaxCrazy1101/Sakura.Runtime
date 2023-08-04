#include "SkrRT/misc/dependency_graph.hpp"
#include "lemon/list_graph.h"

namespace skr
{
using namespace lemon;
class DependencyGraphImpl : public DependencyGraph
{
public:
    using DAGVertex = ListDigraph::Node;
    using DAGVertMap = ListDigraph::NodeMap<Node*>;
    using DAGEdge = ListDigraph::Arc;
    using DAGEdgeMap = ListDigraph::ArcMap<Edge*>;
    using DAG = ListDigraph;
    DependencyGraphImpl() SKR_NOEXCEPT
        : vert_map(graph), edge_map(graph)
    {

    }
    DAG graph;
    DAGVertMap vert_map;
    DAGEdgeMap edge_map;

    virtual dep_graph_handle_t insert(Node* node) SKR_NOEXCEPT final
    {
        const auto dag_node = graph.addNode();
        node->id = graph.id(dag_node);
        node->graph = this;
        vert_map[dag_node] = node;
        node->on_insert();
        return node->id;
    }

    virtual Node* access_node(dep_graph_handle_t handle) SKR_NOEXCEPT final
    {
        const auto dag_node = graph.nodeFromId(handle);
        return vert_map[dag_node];
    }

    virtual bool remove(dep_graph_handle_t handle) SKR_NOEXCEPT final
    {
        auto dag_node = graph.nodeFromId(handle);
        vert_map[dag_node]->on_remove();
        graph.erase(dag_node);
        return true;
    }

    virtual bool remove(Node* node) SKR_NOEXCEPT final
    {
        return remove(node->id);
    }

    virtual bool clear() SKR_NOEXCEPT final
    {
        graph.clear();
        return true;
    }

    virtual bool link(Node* from, Node* to, Edge* edge) SKR_NOEXCEPT final
    {
        const auto from_node = graph.nodeFromId(from->get_id());
        const auto to_node = graph.nodeFromId(to->get_id());
        SKR_UNUSED const auto dag_arc = graph.addArc(from_node, to_node);
        if (edge)
        {
            edge->graph = this;
            edge->from_node = from->get_id();
            edge->to_node = to->get_id();
            edge_map[dag_arc] = edge;
            edge->on_link();
            return edge;
        }
        return false;
    }

/*
    virtual Edge* linkage(Node* from, Node* to) SKR_NOEXCEPT final
    {
        return linkage(from->id, to->id);
    }

    virtual Edge* linkage(dep_graph_handle_t from, dep_graph_handle_t to) SKR_NOEXCEPT final
    {
        graph.addArc(Node s, Node t)
        auto find_edge = boost::edge((vertex_descriptor)from, (vertex_descriptor)to, *this);
        if (find_edge.second)
        {
            return (*this)[find_edge.first];
        }
        return nullptr;
    }

    virtual bool unlink(Node* from, Node* to) SKR_NOEXCEPT final
    {
        return unlink(from->id, to->id);
    }
    
    virtual bool unlink(dep_graph_handle_t from, dep_graph_handle_t to) SKR_NOEXCEPT final
    {
        auto find_edge = boost::edge((vertex_descriptor)from, (vertex_descriptor)to, *this);
        if (!find_edge.second) return false;
        (*this)[find_edge.first]->on_unlink();
        boost::remove_edge(find_edge.first, *this);
        return true;
    }
*/
    
    virtual Node* from_node(Edge* edge) SKR_NOEXCEPT final
    {
        return access_node(edge->from_node);
    }

    virtual Node* to_node(Edge* edge) SKR_NOEXCEPT final
    {
        return access_node(edge->to_node);
    }

    virtual uint32_t foreach_neighbors(dep_graph_handle_t handle, eastl::function<void(DependencyGraphNode*)> f) SKR_NOEXCEPT final
    {
        const auto node = graph.nodeFromId(handle);
        uint32_t count = 0;
        for (ListDigraph::OutArcIt arcIt(graph, node); arcIt != INVALID; ++arcIt) 
        {
            auto edge = edge_map[arcIt];
            f(edge->to());
            count++;
        }
        return count;
    }

    virtual uint32_t foreach_neighbors(const dep_graph_handle_t handle, eastl::function<void(const DependencyGraphNode*)> f) const SKR_NOEXCEPT final
    {
        const auto node = graph.nodeFromId(handle);
        uint32_t count = 0;
        for (ListDigraph::OutArcIt arcIt(graph, node); arcIt != INVALID; ++arcIt) 
        {
            const auto edge = edge_map[arcIt];
            f(edge->to());
            count++;
        }
        return count;
    }
    
    virtual uint32_t foreach_neighbors(Node* node, eastl::function<void(DependencyGraphNode*)> f) SKR_NOEXCEPT final
    {
        return foreach_neighbors(node->get_id(), f);
    }

    virtual uint32_t foreach_neighbors(const Node* node, eastl::function<void(const DependencyGraphNode*)> f) const SKR_NOEXCEPT final
    {
        return foreach_neighbors(node->get_id(), f);
    }

    virtual uint32_t foreach_inv_neighbors(dep_graph_handle_t handle, eastl::function<void(DependencyGraphNode*)> f) SKR_NOEXCEPT final
    {
        const auto node = graph.nodeFromId(handle);
        uint32_t count = 0;
        for (ListDigraph::InArcIt arcIt(graph, node); arcIt != INVALID; ++arcIt) 
        {
            const auto edge = edge_map[arcIt];
            f(edge->to());
            count++;
        }
        return count;
    }

    virtual uint32_t foreach_inv_neighbors(const dep_graph_handle_t handle, eastl::function<void(const DependencyGraphNode*)> f) const SKR_NOEXCEPT final
    {
        const auto node = graph.nodeFromId(handle);
        uint32_t count = 0;
        for (ListDigraph::InArcIt arcIt(graph, node); arcIt != INVALID; ++arcIt) 
        {
            const auto edge = edge_map[arcIt];
            f(edge->to());
            count++;
        }
        return count;
    }

    virtual uint32_t foreach_inv_neighbors(Node* node, eastl::function<void(DependencyGraphNode*)> f) SKR_NOEXCEPT final
    {
        return foreach_inv_neighbors(node->get_id(), f);
    }

    virtual uint32_t foreach_inv_neighbors(const Node* node, eastl::function<void(const DependencyGraphNode*)> f) const SKR_NOEXCEPT final
    {
        return foreach_inv_neighbors(node->get_id(), f);
    }

    virtual uint32_t foreach_outgoing_edges(dep_graph_handle_t handle, eastl::function<void(Node* from, Node* to, Edge* edge)> f) SKR_NOEXCEPT final
    {
        const auto node = graph.nodeFromId(handle);
        uint32_t count = 0;
        for (ListDigraph::OutArcIt arcIt(graph, node); arcIt != INVALID; ++arcIt) 
        {
            auto edge = edge_map[arcIt];
            f(edge->from(), edge->to(), edge);
            count++;
        }
        return count;
    }

    virtual uint32_t foreach_outgoing_edges(Node* node, eastl::function<void(Node* from, Node* to, Edge* edge)> func) SKR_NOEXCEPT final
    {
        return foreach_outgoing_edges(node->id, func);
    }

    virtual uint32_t foreach_incoming_edges(dep_graph_handle_t handle, eastl::function<void(Node* from, Node* to, Edge* edge)> f) SKR_NOEXCEPT final
    {
        const auto node = graph.nodeFromId(handle);
        uint32_t count = 0;
        for (ListDigraph::InArcIt arcIt(graph, node); arcIt != INVALID; ++arcIt) 
        {
            auto edge = edge_map[arcIt];
            f(edge->from(), edge->to(), edge);
            count++;
        }
        return count;
    }

    virtual uint32_t foreach_incoming_edges(Node* node,
        eastl::function<void(Node* from, Node* to, Edge* edge)> func) SKR_NOEXCEPT final
    {
        return foreach_incoming_edges(node->id, func);
    }

    virtual uint32_t foreach_edges(eastl::function<void(Node* from, Node* to, Edge* edge)> f) SKR_NOEXCEPT final
    {
        uint32_t count = 0;
        for (ListDigraph::ArcIt arcIt(graph); arcIt != INVALID; ++arcIt) 
        {
            auto edge = edge_map[arcIt];
            f(edge->from(), edge->to(), edge);
            count++;
        }
        return count;
    }

    virtual uint32_t outgoing_edges(dep_graph_handle_t handle) SKR_NOEXCEPT final
    {
        const auto node = graph.nodeFromId(handle);
        uint32_t count = 0;
        for (ListDigraph::OutArcIt arcIt(graph, node); arcIt != INVALID; ++arcIt) 
        {
            count++;
        }
        return count;
    }

    virtual uint32_t outgoing_edges(const Node* node) SKR_NOEXCEPT final
    {
        return outgoing_edges(node->id);
    }

    virtual uint32_t incoming_edges(const Node* node) SKR_NOEXCEPT final
    {
        return incoming_edges(node->id);
    }

    virtual uint32_t incoming_edges(dep_graph_handle_t handle) SKR_NOEXCEPT final
    {
        const auto node = graph.nodeFromId(handle);
        uint32_t count = 0;
        for (ListDigraph::InArcIt arcIt(graph, node); arcIt != INVALID; ++arcIt) 
        {
            count++;
        }
        return count;
    }
};

uint32_t DependencyGraphNode::outgoing_edges() SKR_NOEXCEPT
{
    return graph->outgoing_edges(this);
}

uint32_t DependencyGraphNode::incoming_edges() SKR_NOEXCEPT
{
    return graph->incoming_edges(this);
}

uint32_t DependencyGraphNode::foreach_neighbors(eastl::function<void(DependencyGraphNode* neig)> f) SKR_NOEXCEPT
{
    return graph->foreach_neighbors(this, f);
}

uint32_t DependencyGraphNode::foreach_neighbors(eastl::function<void(const DependencyGraphNode* neig)> f) const SKR_NOEXCEPT
{
    return graph->foreach_neighbors(this, f);
}

uint32_t DependencyGraphNode::foreach_inv_neighbors(eastl::function<void(DependencyGraphNode* inv_neig)> f) SKR_NOEXCEPT
{
    return graph->foreach_inv_neighbors(this, f);
}

uint32_t DependencyGraphNode::foreach_inv_neighbors(eastl::function<void(const DependencyGraphNode* inv_neig)> f) const SKR_NOEXCEPT
{
    return graph->foreach_inv_neighbors(this, f);
}
} // namespace skr

namespace skr
{
void DependencyGraph::Destroy(DependencyGraph* graph) SKR_NOEXCEPT
{
    delete graph;
}

DependencyGraph* DependencyGraph::Create() SKR_NOEXCEPT
{
    return new DependencyGraphImpl();
}

} // namespace skr