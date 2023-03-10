#pragma once
#include "SkrGui/render_elements/element.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, SGDIDevice, skr_gdi_device)

namespace skr {
namespace gui {

struct SKR_GUI_API RenderWindow : public RenderElement
{
public:
    RenderWindow(skr_gdi_device_id gdi_device);
    virtual ~RenderWindow();

    virtual void layout(struct Constraints* constraints, bool needSize = false) override;
    virtual void draw(skr_gdi_viewport_id viewport, skr_gdi_canvas_id canvas) override;

    virtual skr_float2_t get_size() const;
    virtual void set_size(const skr_float2_t& size);

    skr_gdi_viewport_id get_gdi_viewport() { return gdi_viewport; }

protected:
    skr_gdi_device_id gdi_device = nullptr;
    skr_gdi_viewport_id gdi_viewport = nullptr;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderWindow, skr_gui_render_window);