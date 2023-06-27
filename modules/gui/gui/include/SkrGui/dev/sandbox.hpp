#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct Widget;
struct ICanvasService;
struct ITextService;
struct INativeDevice;

// sandbox 是外部使用 GUI 系统的入口
// 其思想是：输入事件、Backend 等信息，输出每帧的渲染三角与命令
struct SKR_GUI_API Sandbox {
    Sandbox(INativeDevice* device, ICanvasService* canvas_service, ITextService* text_service) SKR_NOEXCEPT;

    void init(Widget* root_widget);

    // void update();
    // void animation();
    // void layout();
    // void paint();
    // void compose();
    // void finalize();

private:
    Widget* _root_widget = nullptr;

    INativeDevice*  _device = nullptr;
    ICanvasService* _canvas_service = nullptr;
    ITextService*   _text_service = nullptr;
};
} // namespace skr::gui