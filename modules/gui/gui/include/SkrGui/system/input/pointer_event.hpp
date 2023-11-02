#pragma once
#include "SkrGui/system/input/event.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/pointer_event.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_enum(
    "guid": "7223961b-5309-4fac-8207-8476bf7f3b05",
    "rtti": true
)
EPointerDeviceType : int32_t
{
    Unknown = 0,
    Mouse,
    Touch,
    // Stylus,
    // InvertedStylus,
    // TrackPad,
};

sreflect_struct(
    "guid": "fa4706c4-5982-4056-a5b5-b12098c0963a",
    "rtti": true
)
PointerEvent : public Event {
    spush_attr("no-rtti": true)
    INativeWindow*     window          = nullptr;                     // 来源 window
    EPointerDeviceType device_type     = EPointerDeviceType::Unknown; // 来源设备
    Offsetf            global_position = {};                          // 位置，全局空间的逻辑坐标
    Offsetf            global_delta    = {};                          // 距离上一次触发的位置变化量
    int64_t            button_id       = 0;                           // 按键 id
    bool               is_down         = false;                       // 按键是否按下
    Matrix4            transform       = {};                          // global->local 转换矩阵
};

sreflect_struct(
    "guid": "91343bb8-4b20-4734-b491-23362b76aa17",
    "rtti": true
)
PointerDownEvent : public PointerEvent {
};

sreflect_struct(
    "guid": "8817dd2b-7e0d-47f5-8ee5-72790bbf3f09",
    "rtti": true
)
PointUpEvent : public PointerEvent {
};

sreflect_struct(
    "guid": "6d6366d3-6755-4e0c-af3f-a5487deb2efa",
    "rtti": true
)
PointerHoverEvent : public PointerEvent {
};

sreflect_struct(
    "guid": "453f7052-9740-4136-9831-55e8188827d2",
    "rtti": true
)
PointerMoveEvent : public PointerEvent {
};

sreflect_struct(
    "guid": "6cddd04b-749c-4a5e-99b0-27396ef84d50",
    "rtti": true
)
PointerEnterEvent : public PointerEvent {
};

sreflect_struct(
    "guid": "f41e2f74-b813-411d-b065-5df10f5edaeb",
    "rtti": true
)
PointerExitEvent : public PointerEvent {
};

} // namespace gui sreflect
} // namespace skr sreflect