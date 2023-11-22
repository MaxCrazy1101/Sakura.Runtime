#pragma once
#include "SkrToolCore/fwd_types.hpp"
#include "SkrRT/misc/types.h"
#ifndef __meta__
    #include "SkrToolCore/asset/cooker.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid": "ff344604-b522-411c-b9a5-1ec4b5970c02")
TOOL_CORE_API SCooker {
    static constexpr uint32_t kDevelopmentVersion = UINT32_MAX;
    virtual ~SCooker() {}
    virtual uint32_t Version()               = 0;
    virtual bool     Cook(SCookContext* ctx) = 0;
    SCookSystem*     system;
};

TOOL_CORE_API SCookSystem* GetCookSystem();
TOOL_CORE_API void         RegisterCookerToSystem(SCookSystem* system, bool isDefault, skr_guid_t cooker, skr_guid_t type, SCooker* instance);

#define sregister_cooker(literal) sstatic_ctor(skd::asset::RegisterCooker<$T>(false, $guid, skr::guid::make_guid_unsafe(literal)))
#define sregister_default_cooker(literal) sstatic_ctor(skd::asset::RegisterCooker<$T>(true, $guid, skr::guid::make_guid_unsafe(literal)))

template <class T>
void RegisterCooker(bool isDefault, skr_guid_t cookerGuid, skr_guid_t resGuid)
{
    static T instance;
    skd::asset::RegisterCookerToSystem(GetCookSystem(), isDefault, cookerGuid, resGuid, &instance);
}
} // namespace asset sreflect
} // namespace skd sreflect