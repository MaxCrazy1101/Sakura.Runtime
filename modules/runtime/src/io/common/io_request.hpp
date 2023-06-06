#pragma once
#include "pool.hpp"
#include "containers/vector.hpp"
#include <string.h> // ::strlen
#include "tracy/Tracy.hpp"

namespace skr {
namespace io {

typedef enum SkrAsyncIOFinishStep
{
    SKR_ASYNC_IO_FINISH_STEP_NONE = 0,
    SKR_ASYNC_IO_FINISH_STEP_WAIT_CALLBACK_POLLING = 1,
    SKR_ASYNC_IO_FINISH_STEP_DONE = 2
} SkrAsyncIOFinishStep;

constexpr const char* callback_names[] = {
    "IOCallback(None)",
    "IOCallback(Enqueued)",
    "IOCallback(Resolving)",
    "IOCallback(Loading)",
    "IOCallback(Loaded)",
    "IOCallback(Decompressing)",
    "IOCallback(Decompressed)",
    "IOCallback(Completed)",
    "IOCallback(Cancelled)",
};
static_assert(sizeof(callback_names) / sizeof(callback_names[0]) == SKR_IO_STAGE_COUNT, "callback_names size mismatch");

struct IORequestBase : public IIORequest
{
    IO_RC_OBJECT_BODY
public:
    SkrAsyncIOFinishStep getFinishStep() const SKR_NOEXCEPT
    { 
        return (SkrAsyncIOFinishStep)skr_atomicu32_load_acquire(&finish_step); 
    }

    void setFinishStep(SkrAsyncIOFinishStep step) SKR_NOEXCEPT
    { 
        skr_atomicu32_store_release(&finish_step, step); 
    }

    void tryPollFinish() SKR_NOEXCEPT
    {
        if (getStatus() == SKR_IO_STAGE_COMPLETED)
        {
            finish_callbacks[SKR_IO_FINISH_POINT_COMPLETE](
                future, this, finish_callback_datas[SKR_IO_FINISH_POINT_COMPLETE]);
        }
        else
        {
            finish_callbacks[SKR_IO_FINISH_POINT_CANCEL](
                future, this, finish_callback_datas[SKR_IO_FINISH_POINT_CANCEL]);
        }
        skr_atomicu32_store_relaxed(&finish_step, SKR_ASYNC_IO_FINISH_STEP_DONE);
    }

    bool needPollFinish() SKR_NOEXCEPT
    {            
        for (auto f : finish_callbacks)
            return f;
        return false;
    }

    void setStatus(ESkrIOStage status)
    {
        skr_atomicu32_store_release(&future->status, status);
        if (const auto callback = callbacks[status])
        {
            ZoneScoped;
            ZoneName(callback_names[status], ::strlen(callback_names[status]));
            callback(future, nullptr, callback_datas[status]);
        }
    }

    ESkrIOStage getStatus() const
    {
        return static_cast<ESkrIOStage>(skr_atomicu32_load_relaxed(&future->status));
    }
    
    void add_callback(ESkrIOStage stage, skr_io_callback_t callback, void* data) SKR_NOEXCEPT 
    { 
        callbacks[stage] = callback; 
        callback_datas[stage] = data; 
    }

    void add_finish_callback(ESkrIOFinishPoint point, skr_io_callback_t callback, void* data) SKR_NOEXCEPT
    {
        finish_callbacks[point] = callback;
        finish_callback_datas[point] = data;
    }

    void use_async_complete() SKR_NOEXCEPT { async_complete = true; }
    void use_async_cancel() SKR_NOEXCEPT { async_cancel = true; }

    bool async_complete = false;
    bool async_cancel = false;
    skr_io_future_t* future = nullptr;

protected:
    SAtomic32 finish_step = 0;
    skr_io_callback_t callbacks[SKR_IO_STAGE_COUNT];
    void* callback_datas[SKR_IO_STAGE_COUNT];

    skr_io_callback_t finish_callbacks[SKR_IO_FINISH_POINT_COUNT];
    void* finish_callback_datas[SKR_IO_FINISH_POINT_COUNT];

public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<IORequestBase*>(ptr);
            p->pool->deallocate(p); 
        };
    }
    IORequestBase(ISmartPoolPtr<IIORequest> pool) : pool(pool) {}
protected:
    ISmartPoolPtr<IIORequest> pool = nullptr;
};

using RQPtr = skr::SObjectPtr<IORequestBase>;
using IORequestQueue = IOConcurrentQueue<RQPtr>;  
using IORequestArray = skr::vector<RQPtr>;

} // namespace io
} // namespace skr