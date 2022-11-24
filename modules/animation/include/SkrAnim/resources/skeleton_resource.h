#pragma once
#include "SkrAnim/module.configure.h"
#include "platform/configure.h"
#ifndef __meta__
    #include "SkrAnim/resources/skeleton_resource.generated.h"
#endif

typedef struct skr_skeleton_resource_t skr_skeleton_resource_t;

#ifdef __cplusplus
#include "SkrAnim/ozz/skeleton.h"

sreflect_struct("guid" : "1876BF35-E4DC-450B-B9D4-09259397F4BA")
sattr("rtti" : true)
skr_skeleton_resource_t
{
    sattr("no-rtti" : true)
    ozz::animation::Skeleton skeleton;
};

#include "resource/resource_factory.h"
namespace skr sreflect
{
namespace resource sreflect
{
sreflect_struct("guid" : "03212f68-3db3-4080-a074-b855cd21e32f")
SSkelFactory : public SResourceFactory
{
public:
    ~SSkelFactory() noexcept = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override { return true; }
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;
    void DestroyResource(skr_resource_record_t* record) override;
};
} // namespace resource
} // namespace skr
#endif