#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct SKR_GUI_API PaintingContext final {
    using ChildPaintingCallback = FunctionRef<void(const PaintingContext&, Offsetf)>;

    PaintingContext(NotNull<ContainerLayer*> container_layer) SKR_NOEXCEPT;

    ICanvas* canvas() SKR_NOEXCEPT;
    void     paint_child(NotNull<RenderObject*> child, Offsetf offset) SKR_NOEXCEPT;
    void     add_layer(NotNull<Layer*> layer) SKR_NOEXCEPT;
    void     push_layer(NotNull<ContainerLayer*> layer, ChildPaintingCallback callback, Offsetf offset) SKR_NOEXCEPT;

private:
    // help functions
    bool _is_recording() const SKR_NOEXCEPT;
    void _start_recording() SKR_NOEXCEPT;
    void _stop_recording() SKR_NOEXCEPT;

private:
    ContainerLayer* _container_layer = nullptr;
    GeometryLayer*  _current_layer   = nullptr;
};
} // namespace skr::gui