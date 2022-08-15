#pragma once
#include "cgpu/io.h"

namespace skr
{
namespace io
{
class RUNTIME_API VRAMService
{
public:
    [[nodiscard]] static VRAMService* create(const skr_vram_io_service_desc_t* desc) SKR_NOEXCEPT;
    static void destroy(VRAMService* service) SKR_NOEXCEPT;

    // we do not lock an ioService to a single vfs, but for better bandwidth use and easier profiling
    // it's recommended to make a unique relevance between ioService & vfs（or vfses share a single I/O hardware)

    virtual void request(const skr_vram_buffer_io_t* buffer_info, skr_async_io_request_t* async_request, skr_vram_buffer_request_t* buffer_request) SKR_NOEXCEPT = 0;
    virtual void request(const skr_vram_texture_io_t* texture_info, skr_async_io_request_t* async_request, skr_vram_texture_request_t* texture_request) SKR_NOEXCEPT = 0;

    // try to cancel an enqueued request at **this** thread
    // not available (returns always false) under lockless mode
    // returns false if the request is under LOADING status
    virtual bool try_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT = 0;

    // emplace a cancel **command** to ioService thread
    // it's recommended to use this under lockless mode
    virtual void defer_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT = 0;

    // stop service and hang up underground thread
    virtual void stop(bool wait_drain = false) SKR_NOEXCEPT = 0;

    // start & run service
    virtual void run() SKR_NOEXCEPT = 0;

    // block & finish up all requests
    virtual void drain() SKR_NOEXCEPT = 0;

    // set sleep time when io queue is detected to be idle
    virtual void set_sleep_time(uint32_t time) SKR_NOEXCEPT = 0;

    // get service status (sleeping or running)
    virtual SkrAsyncIOServiceStatus get_service_status() const SKR_NOEXCEPT = 0;

    virtual ~VRAMService() SKR_NOEXCEPT = default;
    VRAMService() SKR_NOEXCEPT = default;
};
} // namespace io
} // namespace skr