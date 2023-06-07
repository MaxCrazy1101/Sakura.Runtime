#pragma once
#include "SkrGui/framework/diagnostics.hpp"
#include "SkrGui/framework/key.hpp"
#include "SkrGui/framework/widget_misc.hpp"

namespace skr::gui
{
struct Element;

struct SKR_GUI_API Widget : public DiagnosticableTreeNode {
    SKR_GUI_TYPE(Widget, "9f69910d-ba18-4ff4-bf5f-3966507c56ba", DiagnosticableTreeNode);

    inline const Key& key() const SKR_NOEXCEPT { return _key; }

    // bind element
    virtual NotNull<Element*> create_element() SKR_NOEXCEPT = 0;

    // help function
    static bool can_update(NotNull<Widget*> old_widget, NotNull<Widget*> new_widget) SKR_NOEXCEPT;

protected:
    Key _key;
};
} // namespace skr::gui