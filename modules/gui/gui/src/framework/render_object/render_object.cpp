#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/dev/gdi/gdi.hpp"

namespace skr::gui
{

RenderObject::RenderObject() SKR_NOEXCEPT
{
}
RenderObject::~RenderObject()
{
}

// new lifecycle
void RenderObject::mount(NotNull<RenderObject*> parent) SKR_NOEXCEPT
{
    // validate
    if (_parent != nullptr)
    {
        unmount();
        SKR_GUI_LOG_ERROR("already mounted");
    }
    {
        RenderObject* node = parent;
        while (node->_parent)
        {
            node = node->_parent;
            if (node == this)
            {
                SKR_GUI_LOG_ERROR("cycle in the tree");
                break;
            }
        }
    }

    // mount
    _parent = parent;
    if (parent->owner())
    {
        struct _RecursiveHelper {
            NotNull<PipelineOwner*> owner;

            void operator()(NotNull<RenderObject*> obj) const SKR_NOEXCEPT
            {
                obj->attach(owner);
                obj->visit_children(_RecursiveHelper{ owner });
            }
        };
        this->visit_children(_RecursiveHelper{ make_not_null(_parent->owner()) });
    }
    _lifecycle = ERenderObjectLifecycle::Mounted;
}
void RenderObject::unmount() SKR_NOEXCEPT
{
    // validate
    if (_parent == nullptr) { SKR_GUI_LOG_ERROR("already unmounted"); }

    // unmount
    _parent = nullptr;
    if (owner())
    {
        struct _RecursiveHelper {
            void operator()(NotNull<RenderObject*> obj) const SKR_NOEXCEPT
            {
                obj->detach();
                obj->visit_children(_RecursiveHelper{});
            }
        };
        this->visit_children(_RecursiveHelper{});
    }
    _lifecycle = ERenderObjectLifecycle::Unmounted;
}
void RenderObject::destroy() SKR_NOEXCEPT
{
    // TODO. release layer
    _lifecycle = ERenderObjectLifecycle::Destroyed;
}
void RenderObject::attach(NotNull<PipelineOwner*> owner) SKR_NOEXCEPT
{
    // validate
    if (_owner != nullptr) { SKR_GUI_LOG_ERROR("already attached"); }
    if (_parent == nullptr) { SKR_GUI_LOG_ERROR("parent is nullptr"); }

    // attach
    _owner = owner;
    _depth = _parent ? _parent->_depth + 1 : 0;

    // If the node was dirtied in some way while unattached, make sure to add
    // it to the appropriate dirty list now that an owner is available
    if (_needs_layout && _relayout_boundary != nullptr)
    {
        // Don't enter this block if we've never laid out at all;
        // scheduleInitialLayout() will handle it
        _needs_layout = false;
        mark_needs_layout();
    }
    if (_needs_paint && _layer != nullptr)
    {
        // Don't enter this block if we've never painted at all;
        // scheduleInitialPaint() will handle it
        _needs_paint = false;
        mark_needs_paint();
    }

    // recursive
}
void RenderObject::detach() SKR_NOEXCEPT
{
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR("already detached"); }
    _owner = nullptr;
    if (_parent != nullptr && _owner != _parent->_owner) { SKR_GUI_LOG_ERROR("detach from owner but parent is still attached"); }
}

// layout & paint marks
void RenderObject::mark_needs_layout() SKR_NOEXCEPT
{
    if (_relayout_boundary == nullptr)
    {
        _needs_layout = true;
        if (_parent != nullptr)
        {
            _mark_parent_needs_layout();
        }
    }
    else if (_relayout_boundary != this)
    {
        _mark_parent_needs_layout();
    }
    else
    {
        _needs_layout = true;
        if (_owner != nullptr)
        {
            // TODO.
            // owner!._nodesNeedingLayout.add(this);
            // owner!.requestVisualUpdate();
        }
    }
}
void RenderObject::mark_needs_paint() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}

// layout process
bool RenderObject::is_sized_by_parent() const SKR_NOEXCEPT { return false; }
void RenderObject::layout(bool parent_uses_size) SKR_NOEXCEPT
{
    bool          is_relayout_boundary = !parent_uses_size || is_sized_by_parent() || _force_relayout_boundary || !_parent;
    RenderObject* relayout_boundary = is_relayout_boundary ? this : _parent->_relayout_boundary;

    if (!_needs_layout && !_is_constraints_changed)
    {
        if (relayout_boundary != _relayout_boundary)
        {
            _relayout_boundary = relayout_boundary;
            visit_children([](RenderObject* child) { child->_flush_relayout_boundary(); });
        }
    }
    else
    {
        _relayout_boundary = relayout_boundary;
        if (is_sized_by_parent())
        {
            perform_resize();
        }
        perform_layout();

        _needs_layout = false;
        mark_needs_paint();
    }

    // clean up flags
    _is_constraints_changed = false;
}
void RenderObject::perform_resize() SKR_NOEXCEPT {}
void RenderObject::perform_layout() SKR_NOEXCEPT {}

// paint process
void RenderObject::debug_paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT {}
void RenderObject::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT {}
bool RenderObject::is_repaint_boundary() const SKR_NOEXCEPT { return false; }

// transform
bool    RenderObject::paints_child(NotNull<RenderObject*> child) const SKR_NOEXCEPT { return true; }
void    RenderObject::apply_paint_transform(NotNull<RenderObject*> child, Matrix4& transform) const SKR_NOEXCEPT {}
Matrix4 RenderObject::get_transform_to(RenderObject* ancestor) const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
    return {};
}

// layout & paint marks
void RenderObject::_mark_parent_needs_layout() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}
void RenderObject::_flush_relayout_boundary() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}

//==> Begin DiagnosticableTreeNode API
void RenderObject::visit_diagnostics_children(FunctionRef<void(DiagnosticableTreeNode*)> visitor) SKR_NOEXCEPT
{
    visit_children([&visitor](RenderObject* o) { visitor(o); });
}
//==> End DiagnosticableTreeNode API

} // namespace skr::gui