#include "../common/io_runnner.hpp"
#include "async/thread_job.hpp"

namespace skr {
namespace io {

namespace IORunner
{
using Future = skr::IFuture<bool>;
using JobQueueFuture = skr::ThreadedJobQueueFuture<bool>;
using SerialFuture = skr::SerialFuture<bool>;
struct FutureLauncher
{
    FutureLauncher(skr::JobQueue* q) : job_queue(q) {}
    template<typename F, typename... Args>
    Future* async(F&& f, Args&&... args)
    {
        if (job_queue)
            return SkrNew<JobQueueFuture>(job_queue, std::forward<F>(f), std::forward<Args>(args)...);
        else
            return SkrNew<SerialFuture>(std::forward<F>(f), std::forward<Args>(args)...);
    }
    skr::JobQueue* job_queue = nullptr;
};
}

void RunnerBase::recycle() SKR_NOEXCEPT
{
    ZoneScopedN("recycle");
    
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        for (auto& future : finish_futures)
        {
            auto status = future->wait_for(0);
            if (status == skr::FutureStatus::Ready)
            {
                SkrDelete(future);
                future = nullptr;
            }
        }
        auto it = eastl::remove_if(finish_futures.begin(), finish_futures.end(), 
            [](skr::IFuture<bool>* future) {
                return (future == nullptr);
            });
        finish_futures.erase(it, finish_futures.end());
    }

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        reader->recycle((SkrAsyncServicePriority)i);
    }
}

void RunnerBase::resolve() SKR_NOEXCEPT
{
    SKR_ASSERT(reader);
    ZoneScopedN("resolve");

    const uint64_t NBytes = reader->get_prefer_batch_size();
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        uint64_t bytes = 0;
        BatchPtr batch = nullptr;
        while ((bytes <= NBytes) && batch_queues[i].try_dequeue(batch))
        {
            const auto priority = (SkrAsyncServicePriority)i;
            uint64_t batch_size = 0;
            if (bool sucess = resolver_chain->fetch(priority, batch))
            {
                SKR_ASSERT(sucess);
                for (auto&& request : batch->get_requests())
                {
                    for (auto block : request->get_blocks())
                        batch_size += block.size;
                }
                bytes += batch_size;
            }
            resolver_chain->dispatch(priority);
        }
    }
}

uint64_t RunnerBase::fetch() SKR_NOEXCEPT
{
    SKR_ASSERT(reader);
    ZoneScopedN("fetch");

    uint64_t N = 0;
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        const auto priority = (SkrAsyncServicePriority)i;
        while (IOBatchId batch = resolver_chain->poll_processed_batch(priority))
        {
            reader->fetch((SkrAsyncServicePriority)i, batch);

            skr_atomic64_add_relaxed(&reading_batch_counts[i], 1);
            skr_atomic64_add_relaxed(&queued_batch_counts[i], -1);
            N++;
        }
    }
    return N;
}

void RunnerBase::dispatch() SKR_NOEXCEPT
{
    ZoneScopedN("dispatch");
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        reader->dispatch((SkrAsyncServicePriority)i);
    }
}

bool RunnerBase::cancelFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    rq->setStatus(SKR_IO_STAGE_CANCELLED);
    if (rq->needPollFinish())
    {
        finish_queues[priority].enqueue(rq);
        rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_WAIT_CALLBACK_POLLING);
    }
    else
    {
        rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_DONE);
    }
    skr_atomic64_add_relaxed(&processing_request_counts[priority], -1);
    return true;
}

const bool async_cancel = false;
const bool async_finish = false;

bool RunnerBase::try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT
{
    const auto status = rq->getStatus();
    if (status >= SKR_IO_STAGE_LOADING) return false;

    if (bool cancel_requested = skr_atomicu32_load_acquire(&rq->future->request_cancel))
    {
        if (rq->getFinishStep() == SKR_ASYNC_IO_FINISH_STEP_NONE)
        {
            if (async_cancel)
            {
                auto& future = finish_futures.emplace_back();
                future = IORunner::FutureLauncher(job_queue).async(
                [this, priority, rq = rq](){
                    return cancelFunction(rq, priority);
                });
            }
            else
            {
                cancelFunction(rq, priority);
            }
        }
        return true;
    }
    return false;
}

bool RunnerBase::finishFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    rq->setStatus(SKR_IO_STAGE_COMPLETED);
    if (rq->needPollFinish())
    {
        finish_queues[priority].enqueue(rq);
        rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_WAIT_CALLBACK_POLLING);
    }
    else
    {
        rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_DONE);
    }
    skr_atomic64_add_relaxed(&processing_request_counts[priority], -1);
    return true;
}

bool RunnerBase::route_decompress(SkrAsyncServicePriority priority, skr::SObjectPtr<IORequestBase> rq) SKR_NOEXCEPT
{
    const bool need_decompress = false;
    if (!need_decompress)
        return false;
    // decompressor->decompress();
    return true;
}

void RunnerBase::route_finish(SkrAsyncServicePriority priority, skr::SObjectPtr<IORequestBase> rq) SKR_NOEXCEPT
{
    if (async_finish)
    {
        auto& future = finish_futures.emplace_back();
        future = IORunner::FutureLauncher(job_queue).async(
        [this, priority, rq = rq](){
            return finishFunction(rq, priority);
        });
    }
    else
    {
        finishFunction(rq, priority);
    }
}

void RunnerBase::route_loaded() SKR_NOEXCEPT
{
    ZoneScopedN("route_loaded");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        const SkrAsyncServicePriority priority = (SkrAsyncServicePriority)i;
        while (auto loaded = reader->poll_processed_request(priority))
        {
            auto&& rq = skr::static_pointer_cast<IORequestBase>(loaded);
            auto need_decompress = route_decompress(priority, rq);
            if (!need_decompress)
            {
                route_finish(priority, rq);
            }
        }
        while (auto batch = reader->poll_processed_batch(priority))
        {
            skr_atomic64_add_relaxed(&reading_batch_counts[i], -1);
        }
    }
}

} // namespace io
} // namespace skr