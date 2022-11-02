#pragma once
#include "GLTFTool/module.configure.h"
#include "asset/importer.hpp"
#include "platform/configure.h"
#include "cgltf/cgltf.h"

namespace skd sreflect
{
namespace asset sreflect
{
struct sreflect sattr(
    "guid" : "D72E2056-3C12-402A-A8B8-148CB8EAB922",
    "serialize" : "json"
)
GLTFTOOL_API SGltfMeshImporter final : public SImporter
{
    eastl::string assetPath;
    skr_guid_t placeHolder;
    void* Import(skr::io::RAMService*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sregister_importer();

struct sreflect
GLTFTOOL_API SMeshCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_cooker("3b8ca511-33d1-4db4-b805-00eea6a8d5e1");
} // namespace asset
} // namespace skd