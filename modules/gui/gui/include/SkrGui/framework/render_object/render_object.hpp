#pragma once
#include "SkrGui/framework/diagnostics.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/matrix.hpp"
#include "SkrGui/framework/slot.hpp"

namespace skr::gui
{
enum class ERenderObjectLifecycle : uint8_t
{
    Initial,
    Mounted,
    Unmounted,
    Destroyed,
};

struct SKR_GUI_API RenderObject : public DiagnosticableTreeNode {
    SKR_GUI_TYPE(RenderObject, "74844fa6-8994-4915-8f8e-ec944a1cbea4", DiagnosticableTreeNode);
    using VisitFuncRef = FunctionRef<void(NotNull<RenderObject*>)>;

    RenderObject() SKR_NOEXCEPT;
    virtual ~RenderObject();

    // lifecycle & tree
    // ctor -> mount <-> unmount -> destroy
    void                          mount(NotNull<RenderObject*> parent) SKR_NOEXCEPT;  // 挂到父节点下
    void                          unmount() SKR_NOEXCEPT;                             // 从父节点卸载
    virtual void                  destroy() SKR_NOEXCEPT;                             // 销毁，生命周期结束
    virtual void                  attach(NotNull<PipelineOwner*> owner) SKR_NOEXCEPT; // 自身或子树挂载到带有 pipeline owner 的树下
    virtual void                  detach() SKR_NOEXCEPT;                              // 自身或子树从带有 pipeline owner 的树下卸载
    inline ERenderObjectLifecycle lifecycle() const SKR_NOEXCEPT { return _lifecycle; }
    inline PipelineOwner*         owner() const SKR_NOEXCEPT { return _owner; }
    virtual void                  visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT = 0;

    // layout & paint marks
    virtual void mark_needs_layout() SKR_NOEXCEPT;
    virtual void mark_needs_paint() SKR_NOEXCEPT;

    // layout process
    // 1. 传递 constraints 并标记 _is_constraints_changed
    // 2. 调用 layout()，并解析 layout_boundary 信息
    //    2.1 [if !_needs_layout && !_is_constraints_changed] 向下传递 layout_boundary 信息
    //    2.2 [if is_sized_by_parent()] 说明 child 尺寸计算完全由传递的约束决定，调用 perform_resize()
    //    2.3 调用 perform_layout()
    // 3. 清除 _is_relayout_boundary 与 _is_constraints_changed 标记
    // Note: parent_uses_size 主要作用是影响 child 重新布局向父亲的信息传播
    virtual bool is_sized_by_parent() const SKR_NOEXCEPT;
    void         layout(bool parent_uses_size = false) SKR_NOEXCEPT;
    virtual void perform_resize() SKR_NOEXCEPT;
    virtual void perform_layout() SKR_NOEXCEPT;

    // paint process
    // paint 流程由 layer 发起，其调用流程被 PaintingContext 严格封装，不允许直接调用，而是调用 PaintingContext::paintChild
    virtual void debug_paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT;
    virtual void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT;
    virtual bool is_repaint_boundary() const SKR_NOEXCEPT;

    // transform
    // 用于做坐标点转换，通常用于 hit-test
    virtual bool    paints_child(NotNull<RenderObject*> child) const SKR_NOEXCEPT;
    virtual void    apply_paint_transform(NotNull<RenderObject*> child, Matrix4& transform) const SKR_NOEXCEPT;
    virtual Matrix4 get_transform_to(RenderObject* ancestor) const SKR_NOEXCEPT;

    // TODO
    // invoke_layout_callback：用于在 layout 过程中创建 child，通常用于 Sliver
    // layer：repaint_boundary 存储对应 layer 用于局部重绘
    // _paint_with_context：call by PaintingContext
    // handle_event：处理输入事件
    // show_on_screen：或许可以实现，用于 ScrollView 的目标追踪

    //==> Begin DiagnosticableTreeNode API
    void visit_diagnostics_children(FunctionRef<void(DiagnosticableTreeNode*)> visitor) SKR_NOEXCEPT override;
    //==> End DiagnosticableTreeNode API

    // getter
    inline RenderObject* parent() const SKR_NOEXCEPT { return _parent; }
    inline int32_t       depth() const SKR_NOEXCEPT { return _depth; }
    inline Slot          slot() const SKR_NOEXCEPT { return _slot; }

    // setter
    inline void set_slot(Slot slot) SKR_NOEXCEPT { _slot = slot; }

protected:
    void        _mark_parent_needs_layout() SKR_NOEXCEPT;
    inline void _set_force_relayout_boundary(bool v) SKR_NOEXCEPT { _force_relayout_boundary = v; }
    inline void _set_constraints_changed(bool v) SKR_NOEXCEPT { _is_constraints_changed = v; }

private:
    void _flush_relayout_boundary() SKR_NOEXCEPT;

private:
    // render object tree
    RenderObject*  _parent = nullptr;
    PipelineOwner* _owner = nullptr;
    int32_t        _depth = 0;

    // dirty marks
    bool _needs_layout = true;
    bool _needs_paint = true;

    // layout temporal data
    bool _force_relayout_boundary = false; // 强制自己称为 layout_boundary
    bool _is_constraints_changed = false;  // 约束发生变化，在 layout 结束后被清理

    // layout & paint boundary
    RenderObject* _relayout_boundary = nullptr;
    void*         _layer = nullptr; // TODO. layer

    // 用于 invoke_layout_callback()
    bool _doing_this_layout_with_callback = false;

    // 生命周期
    ERenderObjectLifecycle _lifecycle = ERenderObjectLifecycle::Initial;

    // parent 分配的实际 Slot 位置
    Slot _slot = {};
};

} // namespace skr::gui