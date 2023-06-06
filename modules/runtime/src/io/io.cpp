#include "common/io_resolver.hpp"

bool skr_io_future_t::is_ready() const SKR_NOEXCEPT
{
    return get_status() == SKR_IO_STAGE_COMPLETED;
}
bool skr_io_future_t::is_enqueued() const SKR_NOEXCEPT
{
    return get_status() == SKR_IO_STAGE_ENQUEUED;
}
bool skr_io_future_t::is_cancelled() const SKR_NOEXCEPT
{
    return get_status() == SKR_IO_STAGE_CANCELLED;
}
bool skr_io_future_t::is_loading() const SKR_NOEXCEPT
{
    return get_status() == SKR_IO_STAGE_LOADING;
}
ESkrIOStage skr_io_future_t::get_status() const SKR_NOEXCEPT
{
    return (ESkrIOStage)skr_atomicu32_load_acquire(&status);
}

namespace skr {
namespace io {

const char* kIOPoolObjectsMemoryName = "I/O PoolObjects";
const char* kIOConcurrentQueueName = "IOConcurrentQueue";

IIOBatchResolver::~IIOBatchResolver() SKR_NOEXCEPT
{

}

void IIOBatchResolver::resolve(IORequestId request) SKR_NOEXCEPT
{
    (void)request;
}

IIOBatchResolverChain::~IIOBatchResolverChain() SKR_NOEXCEPT
{

}

IIORequestProcessor::~IIORequestProcessor() SKR_NOEXCEPT
{
    
}

IIOReader::~IIOReader() SKR_NOEXCEPT
{
    
}

IIODecompressor::~IIODecompressor() SKR_NOEXCEPT
{
    
}

SObjectPtr<IIOBatchResolverChain> IIOBatchResolverChain::Create(IOBatchResolverId resolver) SKR_NOEXCEPT
{
    auto chain = SObjectPtr<IOBatchResolverChain>::Create(resolver);
    return chain;
}

} // namespace io
} // namespace skr