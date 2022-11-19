#pragma once
#include "SkrToolCore/fwd_types.hpp"
#include "platform/filesystem.hpp"

namespace skd sreflect
{
struct TOOL_CORE_API SProject {
    skr::filesystem::path assetPath;
    skr::filesystem::path outputPath;
    skr::filesystem::path dependencyPath;
    skr_vfs_t* vfs = nullptr;
    ~SProject() noexcept;
};
}