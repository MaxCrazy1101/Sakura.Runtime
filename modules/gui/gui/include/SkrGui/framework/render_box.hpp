#pragma once
#include "SkrGui/framework/render_element.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIDevice, skr_gdi_device)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIElement, skr_gdi_element)

namespace skr {
namespace gui {

using RenderBoxSizeType = skr_float2_t;
struct SKR_GUI_API RenderBox : public RenderElement
{
public:
    RenderBox(skr_gdi_device_id gdi_device);
    virtual ~RenderBox();

    virtual void layout(struct Constraints* constraints, bool needSize = false) override;
    virtual void before_draw(const DrawParams* params) override;
    virtual void draw(const DrawParams* params) override;

    virtual RenderBoxSizeType get_size() const;
    virtual void set_size(const RenderBoxSizeType& size);

    virtual void enable_debug_draw(bool enable);

protected:
    bool draw_debug_rect = false;
    RenderBoxSizeType pos = { 0, 0 };
    RenderBoxSizeType size = { 0, 0 };

    skr_gdi_device_id gdi_device = nullptr;
    skr_gdi_element_id debug_element = nullptr;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderBox, skr_gui_render_box);