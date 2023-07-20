#pragma once
#include "SkrRT/async/async_service.h"
#include "SkrRT/async/condlock.hpp"
#include "io_request.hpp"
#include "io_batch.hpp"
#include "io_resolver.hpp"

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {

struct RunnerBase : public AsyncService
{
    RunnerBase(const ServiceThreadDesc& desc, skr::JobQueue* job_queue) SKR_NOEXCEPT;
    virtual ~RunnerBase() SKR_NOEXCEPT;

    bool try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT;
    uint64_t predicate() const SKR_NOEXCEPT;
    uint64_t processing_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT;
    void poll_finish_callbacks() SKR_NOEXCEPT;

    virtual void drain(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    virtual void drain() SKR_NOEXCEPT;
    virtual void destroy() SKR_NOEXCEPT;
    virtual skr::AsyncResult serve() SKR_NOEXCEPT;

protected:
    void dispatch_complete_(SkrAsyncServicePriority priority, skr::SObjectPtr<IORequestBase> rq) SKR_NOEXCEPT;
    virtual bool complete_(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    virtual bool cancel_(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    skr::vector<IOBatchProcessorId> batch_processors; 
    skr::vector<IORequestProcessorId> request_processors; 
    SAtomic64 processing_request_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

private:
    void phaseRecycle() SKR_NOEXCEPT;
    void phaseProcessBatches() SKR_NOEXCEPT;
    void phaseCompleteBatches(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void phaseCompleteRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    IORequestQueue finish_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    skr::vector<skr::IFuture<bool>*> finish_futures;
    skr::JobQueue* job_queue = nullptr;
};

} // namespace io
} // namespace skr