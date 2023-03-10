#pragma once
#include "SkrGui/render_elements/element.hpp"

namespace skr
{
namespace gui
{
class RenderBox : public RenderElement
{
public:
    RenderBox();
    virtual ~RenderBox();

    virtual void layout(struct Constraints* constraints, bool needSize = false) override;
    virtual void draw(gdi::SGDIViewport* canvas) override;

    virtual skr_float2_t get_size() const;
    virtual void set_size(const skr_float2_t& size);
};
}
}