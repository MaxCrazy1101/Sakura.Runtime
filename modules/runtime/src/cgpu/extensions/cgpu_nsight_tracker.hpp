#pragma once
#include "cgpu/extensions/cgpu_nsight.h"
#include <SkrRT/containers_new/vector.hpp>
#include <mutex>

struct CGPUNSightSingleton {
    CGPUNSightSingleton() SKR_NOEXCEPT          = default;
    virtual ~CGPUNSightSingleton() SKR_NOEXCEPT = default;
    SKR_RUNTIME_API static CGPUNSightSingleton* Get(CGPUInstanceId instance) SKR_NOEXCEPT;

    virtual void register_tracker(CGPUNSightTrackerId tracker) SKR_NOEXCEPT;
    virtual void remove_tracker(CGPUNSightTrackerId tracker) SKR_NOEXCEPT;

    uint32_t                           rc;
    std::mutex                         trackers_mutex;
    skr::vector<CGPUNSightTrackerId> all_trackers;
};

#define CGPU_NSIGNT_SINGLETON_NAME u8"CGPUNSightSingleton"

struct CGPUNSightTrackerBase : public CGPUNSightTracker {
    CGPUNSightTrackerBase(CGPUInstanceId instance, const CGPUNSightTrackerDescriptor* pdesc) SKR_NOEXCEPT;
    virtual ~CGPUNSightTrackerBase() SKR_NOEXCEPT;

    std::mutex                  mutex;
    CGPUNSightSingleton*        singleton;
    CGPUNSightTrackerDescriptor descriptor;
};