#include "io_runnner.hpp"
#include "io_resolver.hpp"
#include "processors.hpp"

namespace skr {
namespace io {

IORequestResolverChain::IORequestResolverChain() SKR_NOEXCEPT 
{
    init_counters();
}

void IORequestResolverChain::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IOBatchId batch;
    while (fetched_batches[priority].try_dequeue(batch))
    {
        for (auto request : batch->get_requests())
        {
            auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
            if (!runner->try_cancel(priority, rq))
            {
                rq->setStatus(SKR_IO_STAGE_RESOLVING);
                for (auto resolver : chain)
                {
                    resolver->resolve(request);
                }
            }
        }
        processed_batches[priority].enqueue(batch);
        dec_processing(priority);
        inc_processed(priority);
    }
}

struct OpenFileResolver : public IORequestResolverBase
{
    virtual void resolve(IORequestId request) SKR_NOEXCEPT
    {
        request->open_file(); 
    }
};

IORequestResolverId IIOService::create_file_resolver() SKR_NOEXCEPT
{ 
    return SObjectPtr<OpenFileResolver>::Create();
}

} // namespace io
} // namespace skr