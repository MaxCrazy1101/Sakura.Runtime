#include "SkrGui/render_objects/render_flex.hpp"

namespace skr::gui
{

RenderFlex::RenderFlex(IGDIDevice* gdi_device)
    : RenderBox(gdi_device)
{
    diagnostic_builder.add_properties(
    SkrNew<TextDiagnosticProperty>(u8"type", u8"flex", u8"layout children"));
}

void RenderFlex::layout(BoxConstraint constraints, bool needSize)
{
    // Determine main axis and cross axis based on flex direction
    Size size = constraints.max_size();
    bool isRow = flex_direction == FlexDirection::Row ||
                 flex_direction == FlexDirection::RowReverse;
    float mainAxisSize = isRow ? size.width : size.height;
    float crossAxisSize = isRow ? size.height : size.width;

    // Calculate total flex factor of all children
    int totalFlex = 0;
    for (int i = 0; i < get_child_count(); i++)
    {
        totalFlex += get_flex(i).flex;
    }

    float availableSpace = mainAxisSize;
    // Calculate non-flexible children's size
    for (int i = 0; i < get_child_count(); i++)
    {
        RenderBox* child = get_child_as_box(i);
        Flexible   flex = get_flex(i);
        if (flex.flex == 0)
        {
            child->layout(constraints, true);
            availableSpace -= isRow ? child->get_size().width : child->get_size().height;
        }
    }

    // Calculate size of each child based on flex factor and available space
    for (int i = 0; i < get_child_count(); i++)
    {
        RenderBox*    child = get_child_as_box(i);
        Flexible      flex = get_flex(i);
        BoxConstraint childConstraints = constraints;
        childConstraints.set_min_size({ 0, 0 });
        if (flex.flex > 0)
        {
            float childMainAxisSize = availableSpace * (float)flex.flex / (float)totalFlex;

            if (isRow)
            {
                childConstraints.max_width = childMainAxisSize;
            }
            else
            {
                childConstraints.max_height = childMainAxisSize;
            }
            if (align_items == AlignItems::Stretch)
            {
                if (isRow)
                {
                    childConstraints.min_height = crossAxisSize;
                }
                else
                {
                    childConstraints.min_width = crossAxisSize;
                }
            }
            if (flex.flex_fit == FlexFit::Tight)
            {
                if (isRow)
                {
                    childConstraints.min_width = childConstraints.max_width;
                }
                else
                {
                    childConstraints.min_height = childConstraints.max_height;
                }
            }
            child->layout(childConstraints, true);
        }
    }

    float mainAxisOffset = 0.0f;
    for (int i = 0; i < get_child_count(); i++)
    {
        RenderBox* child = get_child_as_box(i);
        Size       childSize = child->get_size();
        if (isRow)
        {
            mainAxisOffset += childSize.width;
        }
        else
        {
            mainAxisOffset += childSize.height;
        }
    }

    // Calculate position of each child based on justify content and align items
    float mainAxisStartPosition = 0.0f;
    if (justify_content == JustifyContent::FlexEnd)
    {
        mainAxisStartPosition = mainAxisSize - mainAxisOffset;
    }
    else if (justify_content == JustifyContent::Center)
    {
        mainAxisStartPosition = (mainAxisSize - mainAxisOffset) / 2.0f;
    }
    for (int i = 0; i < get_child_count(); i++)
    {
        if (justify_content == JustifyContent::SpaceBetween)
        {
            if (i > 0)
            {
                mainAxisStartPosition += (mainAxisSize - mainAxisOffset) / (get_child_count() - 1);
            }
        }
        else if (justify_content == JustifyContent::SpaceAround)
        {
            auto spacing = (mainAxisSize - mainAxisOffset) / get_child_count();
            if (i > 0)
            {
                mainAxisStartPosition += spacing;
            }
            else
            {
                mainAxisStartPosition += spacing * 0.5;
            }
        }
        else if (justify_content == JustifyContent::SpaceEvenly)
        {
            mainAxisStartPosition += (mainAxisSize - mainAxisOffset) / (get_child_count() + 1);
        }
        RenderBox* child = get_child_as_box(i);
        Size       childSize = child->get_size();
        float      childMainAxisPosition = mainAxisStartPosition;
        float      childCrossAxisPosition = 0.0f;
        if (align_items == AlignItems::FlexEnd ||
            align_items == AlignItems::Baseline)
        {
            childCrossAxisPosition = crossAxisSize - childSize.height;
        }
        else if (align_items == AlignItems::Center ||
                 align_items == AlignItems::Stretch)
        {
            childCrossAxisPosition = (crossAxisSize - childSize.height) / 2.0f;
        }
        if (flex_direction == FlexDirection::RowReverse ||
            flex_direction == FlexDirection::ColumnReverse)
        {
            childMainAxisPosition = mainAxisSize - childMainAxisPosition - childSize.width;
        }
        child->set_position({ childMainAxisPosition, childCrossAxisPosition });
        mainAxisStartPosition += childSize.width;
    }

    // Set size of flex container
    if (needSize)
    {
        float flexWidth = mainAxisSize;
        float flexHeight = crossAxisSize;
        if (flex_direction == FlexDirection::Column ||
            flex_direction == FlexDirection::ColumnReverse)
        {
            std::swap(flexWidth, flexHeight);
        }
        set_size({ flexWidth, flexHeight });
    }
}

void RenderFlex::add_child(RenderObject* child)
{
    RenderObject::add_child(child);
    flexible_slots.emplace_back();
}

void RenderFlex::insert_child(RenderObject* child, int index)
{
    RenderObject::insert_child(child, index);
    flexible_slots.insert(flexible_slots.begin() + index, Flexible{});
}

void RenderFlex::remove_child(RenderObject* child)
{
    flexible_slots.erase(flexible_slots.begin() + get_child_index(child));
    RenderObject::remove_child(child);
}

void RenderFlex::set_flexible(int index, Flexible flexible)
{
    flexible_slots[index] = flexible;
}

Flexible RenderFlex::get_flex(int index)
{
    return flexible_slots[index];
}

void RenderFlex::set_flex_direction(FlexDirection direction)
{
    flex_direction = direction;
    markLayoutDirty();
}

FlexDirection RenderFlex::get_flex_direction()
{
    return flex_direction;
}

void RenderFlex::set_justify_content(JustifyContent content)
{
    justify_content = content;
    markLayoutDirty();
}

JustifyContent RenderFlex::get_justify_content()
{
    return justify_content;
}

void RenderFlex::set_align_items(AlignItems items)
{
    align_items = items;
    markLayoutDirty();
}

AlignItems RenderFlex::get_align_items()
{
    return align_items;
}

} // namespace skr::gui