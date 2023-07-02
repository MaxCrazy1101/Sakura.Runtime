#pragma once
#include "containers/hashmap.hpp"
#include "platform/atomic.h"
#include "platform/thread.h"
#include "misc/log/log_base.hpp"
#include "misc/log/log_formatter.hpp"

#include "containers/concurrent_queue.h"
#include <EASTL/unique_ptr.h>

namespace skr {
namespace log {

struct LogQueue;

enum EFlushStatus
{
    kNoFlush = 0,
    kFlushing = 1,
    kFlushed = 2
};

struct ThreadToken
{
    ThreadToken(LogQueue& q) SKR_NOEXCEPT;

    SAtomic64 tls_cnt_ = 0;
    skr::ProducerToken ptok_;
    SAtomic32 flush_status_ = kNoFlush;
};

struct LogElement
{
public:
    void dec() SKR_NOEXCEPT;

private:
    LogElement(LogEvent ev, ThreadToken* ptok) SKR_NOEXCEPT;
    LogElement() SKR_NOEXCEPT;
    
    friend struct LogQueue;
    friend struct LogWorker;

    LogEvent event;
    ThreadToken* tok;
    skr::string format;
    ArgsList args;
    bool need_format = true;
};

struct LogQueue
{
public:
    LogQueue() SKR_NOEXCEPT;

    void push(LogEvent ev, const skr::string&& what) SKR_NOEXCEPT;
    void push(LogEvent ev, const skr::string_view format, ArgsList&& args) SKR_NOEXCEPT;
    void mark_flushing(SThreadID tid) SKR_NOEXCEPT;
    
    bool try_dequeue(LogElement& element) SKR_NOEXCEPT;
    bool try_dequeue_from(ThreadToken* tok, LogElement& element) SKR_NOEXCEPT;

    ThreadToken* query_token(SThreadID tid) const SKR_NOEXCEPT;
    ThreadToken* query_flusing() const SKR_NOEXCEPT;
    int64_t query_cnt(SThreadID tid) const SKR_NOEXCEPT;
    int64_t query_cnt() const SKR_NOEXCEPT;

private:
    [[nodiscard]] ThreadToken* on_push(const LogEvent& ev) SKR_NOEXCEPT;

    // formatter & args
    struct LogQueueTraits : public ConcurrentQueueDefaultTraits
    {
        static const bool RECYCLE_ALLOCATED_BLOCKS = true;
        static const int BLOCK_SIZE = 256;

        static inline void* malloc(size_t size) SKR_NOEXCEPT { return sakura_mallocN(size, kLogMemoryName); }
        static inline void free(void* ptr) SKR_NOEXCEPT { return sakura_freeN(ptr, kLogMemoryName); }
    };

    friend struct ThreadToken;
    SAtomic64 total_cnt_ = 0;
    // MPSC
    skr::ConsumerToken ctok_;
    skr::parallel_flat_hash_map<uint64_t, eastl::unique_ptr<ThreadToken>> thread_id_map_;
    skr::ConcurrentQueue<LogElement, LogQueueTraits> queue_;
};

} } // namespace skr::log