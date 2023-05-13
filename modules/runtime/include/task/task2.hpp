#pragma once
#include "platform/configure.h"
#include "utils/defer.hpp"
#include "platform/debug.h"
#include "EASTL/functional.h"
#include "platform/thread.h"
#include "containers/sptr.hpp"
#include "containers/vector.hpp"
#include <coroutine>

namespace skr
{
namespace task2
{
    struct RUNTIME_API scheudler_config_t
    {
        scheudler_config_t();
        bool setAffinity = true;
        uint32_t numThreads = 0;
    };
    struct event_t
    {
        struct State
        {
            SMutex mutex;
            SConditionVariable cv;
            skr::vector<std::coroutine_handle<>> waiters;
            skr::vector<int> workerIndices;
            std::atomic<int> numWaiting = {0};
            std::atomic<int> numWaitingOnCondition = {0};
            bool signalled = false;
            State()
            {
                skr_init_mutex(&mutex);
                skr_init_condition_var(&cv);
            }
            ~State()
            {
                skr_destroy_mutex(&mutex);
                skr_destroy_condition_var(&cv);
            }
        };

        event_t()
            : state(SkrNew<State>())
        {
        }
        event_t(std::nullptr_t)
            : state(nullptr)
        {
        }
        ~event_t()
        {
        }
        RUNTIME_API void notify();
        RUNTIME_API void reset();
        size_t hash() const { return (size_t)state.get(); }
        bool done() const 
        { 
            if(!state) 
                return true;
            SMutexLock guard(state->mutex);
            return state->signalled; 
        }
        explicit operator bool() const { return (bool)state; }
        bool operator==(const event_t& other) const { return state == other.state; }

        event_t(SPtr<State> state) : state(std::move(state)) {}
        SPtr<State> state;
    };
    struct weak_event_t
    {
        weak_event_t(event_t& e) : state(e.state) {}
        weak_event_t(const weak_event_t& other) = default;
        weak_event_t(std::nullptr_t) {}
        ~weak_event_t() = default;
        event_t lock() const { return event_t{ state.lock() }; }
        bool expired() const { return state.expired(); }

        SWeakPtr<event_t::State> state;
    };
    struct task_t
    {
        struct promise_type
        {
            RUNTIME_API task_t get_return_object();
            std::suspend_always initial_suspend() { return {}; }
            std::suspend_never final_suspend() noexcept { return {}; }
            void return_void() {}
            RUNTIME_API void unhandled_exception();
            void* operator new(size_t size) { return sakura_malloc(size); }
            void operator delete(void* ptr, size_t size) { sakura_free(ptr); }
        };
        task_t(std::coroutine_handle<promise_type> coroutine) : coroutine(coroutine) {}
        task_t(task_t&& other) : coroutine(other.coroutine) { other.coroutine = nullptr; }
        task_t() = default;
        task_t& operator=(task_t&& other) { coroutine = other.coroutine; other.coroutine = nullptr; return *this; }
        ~task_t() { if(coroutine) coroutine.destroy(); }
        void resume() const { coroutine.resume(); }
        bool done() const { return coroutine.done(); }
        explicit operator bool() const { return (bool)coroutine; }
        std::coroutine_handle<promise_type> coroutine;
    };

    struct RUNTIME_API scheduler_t
    {
        void initialize(const scheudler_config_t&);
        void bind();
        void unbind();
        void shutdown();
        static scheduler_t* instance();
        void schedule(task_t&& task);
        void schedule(eastl::function<void()> function);
        struct RUNTIME_API Awaitable
        {
            Awaitable(scheduler_t& s, event_t event, int workerIdx = -1);
            bool await_ready() const;
            void await_suspend(std::coroutine_handle<>);
            void await_resume() const {}
            scheduler_t& scheduler;
            event_t event;
            int workerIdx = -1;
        };
        Awaitable wait(event_t event, bool pinned = false);
        void sync(event_t event);
        std::array<std::atomic<int>, 8> spinningWorkers;
        std::atomic<unsigned int> nextSpinningWorkerIdx = {0x8000000};
        std::atomic<unsigned int> nextEnqueueIndex = {0};
        std::array<void*, 256> workers;
        void* mainWorker = nullptr;
        scheudler_config_t config;
        bool initialized = false;
        bool binded = false;
    };

    inline void schedule(task_t&& task)
    {
        scheduler_t::instance()->schedule(std::move(task));
    }
    inline void schedule(eastl::function<void()> function)
    {
        scheduler_t::instance()->schedule(std::move(function));
    }
    inline scheduler_t::Awaitable wait(event_t event, bool pinned = false)
    {
        return scheduler_t::instance()->wait(std::move(event), pinned);
    }
    inline void sync(event_t event)
    {
        scheduler_t::instance()->sync(std::move(event));
    }
}
}