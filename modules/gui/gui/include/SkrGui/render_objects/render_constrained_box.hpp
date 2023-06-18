#pragma once
#include "SkrGui/framework/render_object/render_proxy_box.hpp"

namespace skr::gui
{
struct RenderConstrainedBox : public RenderProxyBox {
    SKR_GUI_TYPE(RenderConstrainedBox, "9a72f533-5afd-46dc-b78e-ef943957ecd4", RenderProxyBox)
    using Super = RenderProxyBox;

    // getter setter
    inline const BoxConstraint& additional_constraint() const SKR_NOEXCEPT { return _additional_constraint; }
    inline void                 set_additional_constraint(const BoxConstraint& constraint) SKR_NOEXCEPT
    {
        if (_additional_constraint != constraint)
        {
            _additional_constraint = constraint;
            mark_needs_layout();
        }
    }

    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // dry layout
    Size compute_dry_layout(BoxConstraint constraints) const SKR_NOEXCEPT override;

    // layout
    void perform_layout() SKR_NOEXCEPT override;

private:
    BoxConstraint _additional_constraint;
};
} // namespace skr::gui