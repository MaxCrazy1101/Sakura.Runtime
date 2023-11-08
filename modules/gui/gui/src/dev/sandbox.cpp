#include "SkrGui/dev/sandbox.hpp"
#include "SkrRT/platform/memory.h"
#include "SkrGui/framework/pipeline_owner.hpp"
#include "SkrGui/framework/build_owner.hpp"
#include "SkrGui/framework/render_object/render_native_window.hpp"
#include "SkrGui/framework/element/render_native_window_element.hpp"
#include "SkrGui/framework/widget/render_native_window_widget.hpp"
#include "SkrGui/framework/layer/native_window_layer.hpp"
#include "SkrGui/backend/device/device.hpp"
#include "SkrGui/backend/device/window.hpp"
#include "SkrGui/system/input/event.hpp"
#include "SkrGui/system/input/pointer_event.hpp"

namespace skr::gui
{
Sandbox::Sandbox(INativeDevice* device) SKR_NOEXCEPT
    : _device(device)
{
}

void Sandbox::init()
{
    // init owner
    _build_owner    = SkrNew<BuildOwner>();
    _pipeline_owner = SkrNew<PipelineOwner>(_device);
}
void Sandbox::shutdown()
{
}

void Sandbox::set_content(NotNull<Widget*> content)
{
    _content = content;
}
void Sandbox::show(const WindowDesc& desc)
{
    // create native window
    auto native_window = _device->create_window()->type_cast_fast<INativeWindow>();
    native_window->init_normal(desc);

    // init root render native window
    // 这里做了倒序创建，主要目的是为了对 Root RenderObject 做先行的 Owner 处理
    // 因为在初始化过程中，无法由 Widget 给到 pipeline_owner
    _root_render_object = SkrNew<RenderNativeWindow>(native_window);
    _root_render_object->setup_owner(_pipeline_owner);
    _root_render_object->prepare_initial_frame();

    // new widget
    // 在这里创建 widget 主要是为了方便 element 进行 updateChild
    auto root_widget = SNewWidget(RenderNativeWindowWidget)
    {
        p.child                       = _content;
        p.native_window_render_object = _root_render_object;
    };

    // init element
    // 使用 root_widget 创建 root_element，并先行初始化
    _root_element = root_widget->create_element()->type_cast_fast<RenderNativeWindowElement>();
    _root_element->setup_owner(_build_owner);
    _root_element->prepare_initial_frame();

    // init layer
    _root_layer = _root_render_object->layer()->type_cast_fast<NativeWindowLayer>();
}

void Sandbox::update()
{
    _build_owner->flush_build();
}
void Sandbox::layout()
{
    _pipeline_owner->flush_layout();
}
void Sandbox::paint()
{
    _pipeline_owner->flush_paint();
}
void Sandbox::compose()
{
    _root_layer->update_window();
}

bool Sandbox::dispatch_event(Event* event)
{
    if (event->type_is<PointerDownEvent>() || event->type_is<PointerMoveEvent>())
    {
        // TODO. hit test
        auto pointer_event = event->type_cast_fast<PointerEvent>();

        // hit test
        HitTestResult result;
        hit_test(&result, pointer_event->global_position);

        // dispatch event
        PointerMoveEvent event;
        for (const auto& entry : result.path())
        {
            if (entry.target->handle_event(&event, const_cast<HitTestEntry*>(&entry)))
            {
                return true;
            }
        }
    }
    else if (event->type_is<PointerUpEvent>())
    {
        // TODO. restore hit test path and remove
    }
    else if (event->type_is<PointerMoveEvent>())
    {
        auto pointer_event = event->type_cast_fast<PointerScrollEvent>();
        SKR_LOG_INFO(u8"%f, %f", pointer_event->scroll_delta.x, pointer_event->scroll_delta.y);
    }

    return false;
}

bool Sandbox::hit_test(HitTestResult* result, Offsetf global_position)
{
    return _root_render_object->hit_test(result, _root_render_object->window()->to_relative(global_position));
}

void Sandbox::resize_window(int32_t width, int32_t height)
{
}

} // namespace skr::gui