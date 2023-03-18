#include "common_layer.hpp"
#include "reading_pool.hpp"
#include "platform/memory.h"
#include "platform/atomic.h"
#include "platform/debug.h"
#include "utils/log.h"
#include "containers/vector.hpp"
#include <EASTL/fixed_vector.h>
#include "platform/shared_library.hpp"
#include "SDL2/SDL_timer.h"

namespace skr {
namespace input {

extern CommonInputDevice* CreateInputDevice_SDL2Keyboard(CommonInputLayer* pLayer) SKR_NOEXCEPT;

CommonInputReading::CommonInputReading(CommonInputReadingProxy* pPool, struct CommonInputDevice* pDevice) SKR_NOEXCEPT
    : ref_count(0), pool(pPool), device(pDevice)
{

}

CommonInputReading::~CommonInputReading() SKR_NOEXCEPT
{

}

CommonInputReadingPoolBase::~CommonInputReadingPoolBase() SKR_NOEXCEPT
{
    uint64_t count = 0;
    CommonInputReading* ptr = nullptr;
    while (m_pool.try_dequeue(ptr))
    {
        SkrDelete(ptr);
        ++count;
    }
    SKR_LOG_INFO("CommonInputReadingPoolBase::~CommonInputReadingPoolBase() - %llu objects deleted", count);
    ReportLeaking();
}

CommonInputDevice::CommonInputDevice(struct CommonInputLayer* pLayer) SKR_NOEXCEPT
    : layer(pLayer)
{

}

CommonInputDevice::~CommonInputDevice() SKR_NOEXCEPT
{

}

CommonInputLayer::~CommonInputLayer() SKR_NOEXCEPT
{

}

// GameInput implementation
struct Input_Common : public CommonInputLayer
{
    Input_Common() SKR_NOEXCEPT
        : CommonInputLayer()
    {

    }

    ~Input_Common() SKR_NOEXCEPT
    {

    }

    void GetLayerId(LayerId* out_id) const SKR_NOEXCEPT final
    {
        if (out_id) *out_id = kCommonInputLayerId;
    }

    bool Initialize() SKR_NOEXCEPT final
    {
        // Add SDL2 keyboard device
        devices.emplace_back(CreateInputDevice_SDL2Keyboard(this));

        return true;
    }
    
    bool Finalize() SKR_NOEXCEPT final
    {
        for (auto& device : devices)
        {
            SkrDelete(device);
        }
        return true;
    }

    bool SetEnabled(bool _enabled) SKR_NOEXCEPT final
    {
        skr_atomic32_store_release(&enabled, _enabled ? 1 : 0);
        return true;
    }
    
    bool IsEnabled() const SKR_NOEXCEPT final
    {
        auto enabled_val = skr_atomic32_load_acquire(&enabled);
        return enabled_val;
    }

    uint64_t GetCurrentTimestampUSec() SKR_NOEXCEPT final
    {
        uint64_t now = SDL_GetPerformanceCounter();
        uint64_t freq = SDL_GetPerformanceFrequency();
        uint64_t value = now / freq * 1000 * 1000;
        return value;
    }

    EInputResult GetCurrentReading(EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        for (auto device : devices)
        {
            if (device->SupportKind(kind))
            {
                if (device->GetCurrentReading(kind, out_reading) == INPUT_RESULT_OK)
                {
                    return INPUT_RESULT_OK;
                }
            }
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    EInputResult GetNextReading(InputReading* in_reference, EInputKind kind, InputDevice* in_device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        CommonInputDevice* device = (CommonInputDevice*)in_device;
        if (device)
        {
            return device->GetNextReading(in_reference, kind, out_reading);
        }
        else
        {
            CommonInputReading* reference = (CommonInputReading*)in_reference;
            uint64_t InTimestamp = reference->GetTimestamp();
            CommonInputReading* ptr = nullptr;
            const auto total = skr_atomic32_load_acquire(&GlobalReadingQueueCount);
            size_t cycles = 0u;
            while (GlobalReadingQueue.try_dequeue(ptr) && cycles++ < total)
            {
                if (InTimestamp < ptr->GetTimestamp() && ptr->GetInputKind() == kind)
                {
                    skr_atomic32_add_relaxed(&GlobalReadingQueueCount, -1);
                    ptr->Fill(out_reading);
                    return INPUT_RESULT_OK;
                }
                else
                {
                    GlobalReadingQueue.enqueue(ptr);
                }
            }
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    EInputResult GetPreviousReading(InputReading* reference, EInputKind kind, InputDevice* device, InputReading** out_reading) SKR_NOEXCEPT final
    {
        CommonInputDevice* input_device = (CommonInputDevice*)device;
        if (input_device)
        {
            return input_device->GetPreviousReading(reference, kind, out_reading);
        }
        else
        {
            CommonInputReading* ref = (CommonInputReading*)reference;
            uint64_t InTimestamp = ref->GetTimestamp();
            CommonInputReading* ptr = nullptr;
            const auto total = skr_atomic32_load_acquire(&GlobalReadingQueueCount);
            size_t cycles = 0u;
            while (GlobalReadingQueue.try_dequeue(ptr) && cycles++ < total)
            {
                if (InTimestamp > ptr->GetTimestamp() && ptr->GetInputKind() == kind)
                {
                    skr_atomic32_add_relaxed(&GlobalReadingQueueCount, -1);
                    ptr->Fill(out_reading);
                    return INPUT_RESULT_OK;
                }
                else
                {
                    GlobalReadingQueue.enqueue(ptr);
                }
            }
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    void GetDevice(InputReading* in_reading, InputDevice** out_device) SKR_NOEXCEPT final
    {
        CommonInputReading* reading = (CommonInputReading*)in_reading;
        if (out_device) *out_device = (InputDevice*)reading->device;      
    }

    uint32_t GetKeyState(InputReading* in_reading, uint32_t stateArrayCount, InputKeyState* stateArray) SKR_NOEXCEPT final
    {
        CommonInputReading* reading = (CommonInputReading*)in_reading;
        return reading->GetKeyState(stateArrayCount, stateArray);
    }

    uint32_t GetMouseState(InputReading* in_reading, InputMouseState* state) SKR_NOEXCEPT final
    {
        return 0;
    }

    virtual uint64_t GetTimestampUSec(InputReading* in_reading) SKR_NOEXCEPT final
    {
        CommonInputReading* reading = (CommonInputReading*)in_reading;
        return reading->GetTimestamp();
    }

    void Release(InputReading* in_reading) SKR_NOEXCEPT final
    {
        CommonInputReading* reading = (CommonInputReading*)in_reading;
        reading->release();
    }

    void Release(InputDevice* in_device) SKR_NOEXCEPT final
    {
        
    }

    void Tick() SKR_NOEXCEPT final
    {
        auto TimeStamp = GetCurrentTimestampUSec();
        for (auto device : devices)
        {
            device->Tick();
        }
        // Poll newest readings
        for (auto device : devices)
        {
            InputReading* out_reading = nullptr;
            for (auto kind : device->ReportKinds())
            {
                device->GetCurrentReading(kind, &out_reading);
                if (CommonInputReading* reading = (CommonInputReading*)out_reading)
                {
                    if (reading->GetTimestamp() >= TimeStamp) // Generated at this Tick
                    {
                        GlobalReadingQueue.enqueue((CommonInputReading*)reading);
                        skr_atomic32_add_relaxed(&GlobalReadingQueueCount, 1);
                    }
                    else
                    {
                        reading->release();
                    }
                }
            }
        }
        // Clear old readings
        {
            CommonInputReading* ptr = nullptr;
            const auto total = GlobalReadingQueue.size_approx();
            size_t cycles = 0u;
            while (GlobalReadingQueue.try_dequeue(ptr) && cycles++ < total)
            {
                if (ptr->GetTimestamp() < TimeStamp - GetReadingHistoryLifetimeUSec())
                {
                    ptr->release();
                    skr_atomic32_add_relaxed(&GlobalReadingQueueCount, -1);
                }
                else
                {
                    GlobalReadingQueue.enqueue(ptr);
                }
            }
        }
    }

    uint64_t GetReadingHistoryLifetimeUSec() const SKR_NOEXCEPT final
    {
        return 500 * 1000;
    }

    SAtomic32 GlobalReadingQueueCount = 0;
    moodycamel::ConcurrentQueue<CommonInputReading*> GlobalReadingQueue;
    skr::vector<CommonInputDevice*> devices;
    SAtomic32 enabled = true;
};

InputLayer* Input_Common_Create() SKR_NOEXCEPT
{
    return SkrNew<Input_Common>();
}

} }