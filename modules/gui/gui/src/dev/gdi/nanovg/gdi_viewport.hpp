#pragma once
#include <nanovg.h>
#include "dev/gdi/private/gdi_viewport.hpp"

namespace skr::gdi
{
struct GDIViewportNVG : public GDIViewportPrivate {
    // void add_canvas(IGDIViewport* canvas) final;
    // void remove_canvas(IGDIViewport* canvas) final;
};
} // namespace skr::gdi