#include "../../pch.hpp" // IWYU pragma: keep
#include "SkrRT/async/wait_timeout.hpp"
#include "../common/io_request.hpp"
#include "../dstorage/dstorage_resolvers.hpp"
#include "vram_service.hpp"
#include "vram_resolvers.hpp"

namespace skr {
namespace io {

namespace VRAMUtils
{

}

IOResultId VRAMIOBatch::add_request(IORequestId request, skr_io_future_t* future) SKR_NOEXCEPT
{
    /*
    auto srv = static_cast<VRAMService*>(service);
    auto buffer = srv->vram_buffer_pool->allocate();
    auto rq = skr::static_pointer_cast<VRAMIORequest>(request);
    rq->future = future;
    rq->destination = buffer;
    rq->owner_batch = this;
    SKR_ASSERT(!rq->blocks.empty());
    addRequest(request);
    return buffer;
    */
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}

uint32_t VRAMService::global_idx = 0;
VRAMService::VRAMService(const VRAMServiceDescriptor* desc) SKR_NOEXCEPT
    : name(desc->name ? skr::string(desc->name) : skr::format(u8"VRAMService-{}", global_idx++)), 
      awake_at_request(desc->awake_at_request),
      runner(this, desc->callback_job_queue)
{
    request_pool = SmartPoolPtr<VRAMIORequest, IIORequest>::Create(kIOPoolObjectsMemoryName);
    // vram_buffer_pool = SmartPoolPtr<VRAMIOBuffer, IRAMIOBuffer>::Create(kIOPoolObjectsMemoryName);
    vram_batch_pool = SmartPoolPtr<VRAMIOBatch, IIOBatch>::Create(kIOPoolObjectsMemoryName);

    /*
    if (desc->use_dstorage)
        runner.batch_reader = CreateBatchReader(this, desc);
    if (!runner.batch_reader)
        runner.reader = CreateReader(this, desc);
    */
    runner.set_resolvers();

    if (!desc->awake_at_request)
    {
        if (desc->sleep_time > 2000)
        {
            SKR_ASSERT(desc->sleep_time <= 2000);
            SKR_LOG_FATAL("RAMService: too long sleep_time causes 'deadlock' when awake_at_request is false");
        }
    }
    runner.set_sleep_time(desc->sleep_time);
}

IVRAMService* IVRAMService::create(const VRAMServiceDescriptor* desc) SKR_NOEXCEPT
{
    return SkrNew<VRAMService>(desc);
}

void IVRAMService::destroy(IVRAMService* service) SKR_NOEXCEPT
{
    ZoneScopedN("destroy");

    auto S = static_cast<VRAMService*>(service);
    S->runner.destroy();
    SkrDelete(S);
}

IOBatchId VRAMService::open_batch(uint64_t n) SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&batch_sequence, 1);
    return skr::static_pointer_cast<IIOBatch>(vram_batch_pool->allocate(this, seq, n));
}

IORequestId VRAMService::open_request() SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&request_sequence, 1);
    return skr::static_pointer_cast<IIORequest>(request_pool->allocate(seq));
}

void VRAMService::request(IOBatchId batch) SKR_NOEXCEPT
{
    runner.enqueueBatch(batch);
    if (awake_at_request)
    {
        runner.awake();
    }
}

/*
RAMIOBufferId VRAMService::request(IORequestId request, skr_io_future_t* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto batch = open_batch(1);
    auto result = batch->add_request(request, future);
    auto buffer = skr::static_pointer_cast<RAMIOBuffer>(result);
    batch->set_priority(priority);
    this->request(batch);
    return buffer;
}
*/

void VRAMService::stop(bool wait_drain) SKR_NOEXCEPT
{
    if (wait_drain)
    {
        drain();
    }
    runner.stop();
}

void VRAMService::run() SKR_NOEXCEPT
{
    runner.run();
}

void VRAMService::drain(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    runner.drain(priority);    
    {
        ZoneScopedN("server_drain");
        auto predicate = [this, priority] {
            return !runner.processing_count(priority);
        };
        bool fatal = !::wait_timeout(predicate, 5);
        if (fatal)
        {
            SKR_LOG_FATAL("RAMService: drain timeout, %llu requests are still processing", 
                runner.processing_count(priority));
        }
    }
}

void VRAMService::set_sleep_time(uint32_t ms) SKR_NOEXCEPT
{
    runner.set_sleep_time(ms);
}

SkrAsyncServiceStatus VRAMService::get_service_status() const SKR_NOEXCEPT
{
    return runner.getServiceStatus();
}

void VRAMService::poll_finish_callbacks() SKR_NOEXCEPT
{
    runner.poll_finish_callbacks();
}

VRAMService::Runner::Runner(VRAMService* service, skr::JobQueue* job_queue) SKR_NOEXCEPT
    : RunnerBase({ service->name.u8_str(), SKR_THREAD_ABOVE_NORMAL }, job_queue),
    service(service)
{

}

void VRAMService::Runner::enqueueBatch(const IOBatchId& batch) SKR_NOEXCEPT
{
    const auto priority = batch->get_priority();
    for (auto&& request : batch->get_requests())
    {
        if (auto pStatus = get_component<IOStatusComponent>(request.get()))
        {
            auto status = pStatus->getStatus();
            SKR_ASSERT(status == SKR_IO_STAGE_NONE);
            pStatus->setStatus(SKR_IO_STAGE_ENQUEUED);
        }
    }
    batch_buffer->fetch(priority, batch);
    skr_atomic64_add_relaxed(&processing_request_counts[priority], 1);
}


void VRAMService::Runner::set_resolvers() SKR_NOEXCEPT
{
    IORequestResolverId openfile = nullptr;
    const bool dstorage = batch_reader.get();
    if (dstorage) 
    {
        openfile = SObjectPtr<DStorageFileResolver>::Create();
    }
    else
    {
        openfile = SObjectPtr<VFSFileResolver>::Create();
    }   

    auto alloc_resource = SObjectPtr<AllocateVRAMResourceResolver>::Create();
    auto chain = skr::static_pointer_cast<IORequestResolverChain>(IIORequestResolverChain::Create());
    chain->runner = this;
    chain->then(openfile)
        ->then(alloc_resource);
    batch_buffer = SObjectPtr<IOBatchBuffer>::Create(); // hold batches
    if (dstorage)
    {
        batch_processors = { batch_buffer, chain, batch_reader };
    }
    else
    {
        batch_processors = { batch_buffer, chain };
        request_processors = { reader };
    }
}
} // namespace io
} // namespace skr