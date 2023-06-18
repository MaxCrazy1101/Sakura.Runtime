#pragma once
#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/math/layout.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct HitTestRecord {
};

struct SKR_GUI_API RenderBox : public RenderObject {
    SKR_GUI_TYPE(RenderBox, "01a2eb19-1299-4069-962f-88db0c719134", RenderObject);

public:
    RenderBox();
    ~RenderBox();

    // getter & setter
    inline Size          size() const SKR_NOEXCEPT { return _size; }
    inline void          set_size(Size size) SKR_NOEXCEPT { _size = size; }
    inline BoxConstraint constraints() const SKR_NOEXCEPT { return _constraints; }
    inline void          set_constraints(BoxConstraint constraints) SKR_NOEXCEPT
    {
        if (_constraints != constraints)
        {
            _constraints = constraints;
            _set_force_relayout_boundary(_constraints.is_tight());
            _set_constraints_changed(true);
        }
    }

    // intrinsic size
    float get_min_intrinsic_width(float height) const SKR_NOEXCEPT;
    float get_max_intrinsic_width(float height) const SKR_NOEXCEPT;
    float get_min_intrinsic_height(float width) const SKR_NOEXCEPT;
    float get_max_intrinsic_height(float width) const SKR_NOEXCEPT;

    // dry layout
    Size get_dry_layout(BoxConstraint constraints) const SKR_NOEXCEPT;

    // TODO.
    // global_to_local
    // local_to_global
    // paint_bounds
    // hit_test
    // handle_event

protected:
    // intrinsic size
    virtual float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT;
    virtual float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT;
    virtual float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT;
    virtual float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT;

    // dry layout
    virtual Size compute_dry_layout(BoxConstraint constraints) const SKR_NOEXCEPT;

private:
    void perform_resize() SKR_NOEXCEPT override; // override compute_dry_layout

private:
    Size          _size;
    BoxConstraint _constraints;

    // TODO. cached data
};

} // namespace skr::gui