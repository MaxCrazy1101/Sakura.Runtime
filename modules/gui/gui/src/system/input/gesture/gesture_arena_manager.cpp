#include "SkrGui/system/input/gesture/gesture_arena_manager.hpp"

namespace skr::gui
{
GestureArenaManager::~GestureArenaManager()
{
    for (auto& pair : _arenas)
    {
        SkrDelete(pair.value);
    }
}

GestureArena* GestureArenaManager::open_if_resolved(CombinePointerId pointer)
{
    GestureArena* arena = find_arena_or_add(pointer);
    if (arena->state() == GestureArenaState::Resolved)
    {
        arena->open();
    }
    return arena;
}
GestureArena* GestureArenaManager::add_gesture(CombinePointerId pointer, GestureRecognizer* gesture)
{
    GestureArena* arena = find_arena_or_add(pointer);
    arena->add(gesture);
    return arena;
}

GestureArena* GestureArenaManager::find_arena(CombinePointerId pointer)
{
    return _arenas.find(pointer)->value;
}
GestureArena* GestureArenaManager::find_arena_or_add(CombinePointerId pointer)
{
    auto result = _arenas.add_unsafe(pointer);
    if (!result.already_exist)
    {
        result->value = SkrNew<GestureArena>(pointer);
    }
    return result->value;
}
} // namespace skr::gui