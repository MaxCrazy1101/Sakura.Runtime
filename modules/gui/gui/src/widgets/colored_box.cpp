#include "SkrGui/widgets/colored_box.hpp"
#include "SkrGui/render_objects/render_colored_box.hpp"

namespace skr::gui
{
NotNull<RenderObject*> ColoredBox::create_render_object() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<RenderColoredBox>());
}
void ColoredBox::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}
} // namespace skr::gui