#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API ColorPicker : public LeafRenderObjectWidget {
    SKR_GUI_TYPE(ColorPicker, "270fc1d7-19ab-4f1a-8c10-e08b69393425", LeafRenderObjectWidget)

    bool is_srgb;
};
} // namespace skr::gui