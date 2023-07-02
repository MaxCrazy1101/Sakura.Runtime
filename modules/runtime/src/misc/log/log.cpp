#include "../../pch.hpp"
#include "platform/thread.h"
#include "misc/log.h"
#include "misc/log/logger.hpp"
#include "misc/log/log_manager.hpp"

#include <thread>
#include <csignal>
#include <stdarg.h> // va_start
#include <stdio.h> // ::atexit
#include <EASTL/fixed_hash_set.h>

#include "tracy/Tracy.hpp"

namespace skr {
namespace log {

const char* kLogMemoryName = "sakura::log";

LogEvent::LogEvent(Logger* logger, LogLevel level, const LogSourceData& src_data) SKR_NOEXCEPT
    : level(level), timestamp(LogManager::tscns_.rdtsc()), 
      thread_id(skr_current_thread_id()), thread_name(skr_current_thread_get_name()),
      logger(logger), src_data(src_data)
{

}

LogFormatter::~LogFormatter() SKR_NOEXCEPT
{

}

skr::string const& LogFormatter::format(const skr::string& format, const ArgsList& args_list)
{
    args_list.format_(format, *this);
    return formatted_string;
}

Logger::Logger(const char8_t* name) SKR_NOEXCEPT
    : name(name)
{
    if (auto worker = LogManager::TryGetWorker())
    {
        worker->add_logger(this);
    }
}

Logger::~Logger() SKR_NOEXCEPT
{
    if (auto worker = LogManager::TryGetWorker())
    {
        worker->remove_logger(this);
    }
}

Logger* Logger::GetDefault() SKR_NOEXCEPT
{
    return LogManager::GetDefaultLogger();
}

void Logger::sinkDefaultImmediate(const LogEvent& e, skr::string_view what) const SKR_NOEXCEPT
{
    skr::log::LogManager::PatternAndSink(e, what);
}

bool Logger::canPushToQueue() const SKR_NOEXCEPT
{
    auto worker = LogManager::TryGetWorker();
    return worker;
}

bool Logger::tryPushToQueue(LogEvent ev, skr::string_view format, ArgsList&& args_list) SKR_NOEXCEPT
{
    auto worker = LogManager::TryGetWorker();
    if (worker)
    {
        auto queue_ = worker->queue_;
        queue_->push(ev, format, skr::move(args_list));
        notifyWorker();
        return true;
    }
    return false;
}

bool Logger::tryPushToQueue(LogEvent ev, skr::string&& what) SKR_NOEXCEPT
{
    auto worker = LogManager::TryGetWorker();
    if (worker)
    {
        auto queue_ = worker->queue_;
        queue_->push(ev, skr::move(what));
        notifyWorker();
        return true;
    }
    return false;
}

void Logger::notifyWorker() SKR_NOEXCEPT
{
    if (auto worker = LogManager::TryGetWorker())
    {
        worker->awake();
    }
}

LogElement::LogElement(LogEvent ev) SKR_NOEXCEPT
    : event(ev)
{

}

LogElement::LogElement() SKR_NOEXCEPT
    : event(nullptr, LogLevel::kTrace, {})
{
    
}

skr::log::LogLevel LogConstants::gLogLevel = skr::log::LogLevel::kTrace;
using namespace skr::guid::literals;
const skr_guid_t LogConstants::kDefaultPatternId = u8"c236a30a-c91e-4b26-be7c-c7337adae428"_guid;
const skr_guid_t LogConstants::kDefaultConsolePatternId = u8"e3b22b5d-95ea-462d-93bf-b8b91e7b991b"_guid;
const skr_guid_t LogConstants::kDefaultConsoleSinkId = u8"11b910c7-de4b-4bba-9dee-5853f35b0c10"_guid;
const skr_guid_t LogConstants::kDefaultFilePatternId = u8"75871d37-ba78-4a75-bb7a-455f08bc8a2e"_guid;
const skr_guid_t LogConstants::kDefaultFileSinkId = u8"289d0408-dec8-4ceb-ae32-55d4793df983"_guid;

SAtomic64 LogManager::available_ = 0;
eastl::unique_ptr<LogWorker> LogManager::worker_ = nullptr;
LogPatternMap LogManager::patterns_ = {};
LogSinkMap LogManager::sinks_ = {};
std::once_flag default_logger_once_;
std::once_flag default_pattern_once_;
eastl::unique_ptr<skr::log::Logger> LogManager::logger_ = nullptr;
TSCNS LogManager::tscns_ = {};
LogManager::DateTime LogManager::datetime_ = {};

void LogManager::Initialize() SKR_NOEXCEPT
{
    if (skr_atomic64_load_acquire(&available_) != 0)
        return;

    // start worker
    if (!worker_)
    {
        worker_ = eastl::make_unique<LogWorker>(kLoggerWorkerThreadDesc);
        worker_->run();
    }
    skr_atomic64_cas_relaxed(&available_, 0, 1);
}

void LogManager::Finalize() SKR_NOEXCEPT
{
    // skr::log::LogManager::logger_.reset();
    if (skr_atomic64_load_acquire(&available_) != 0)
    {
        worker_.reset();
        skr_atomic64_cas_relaxed(&available_, 1, 0);
    }
}

LogWorker* LogManager::TryGetWorker() SKR_NOEXCEPT
{
    if (skr_atomic64_load_acquire(&available_) == 0)
        return nullptr;
    return worker_.get();
}

Logger* LogManager::GetDefaultLogger() SKR_NOEXCEPT
{
    std::call_once(
        skr::log::default_logger_once_,
        [] {
            skr::log::LogManager::tscns_.init();
            skr::log::LogManager::datetime_.reset_date();
            skr::log::LogManager::logger_ = eastl::make_unique<skr::log::Logger>(u8"Log");

            // register default pattern
            auto ret = LogManager::RegisterPattern(LogConstants::kDefaultPatternId, 
                eastl::make_unique<LogPattern>(
                    u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message)"
                ));
            SKR_ASSERT(ret && "Default log pattern register failed!");
            
            // register default console pattern & sink
            ret = LogManager::RegisterPattern(LogConstants::kDefaultConsolePatternId, 
                eastl::make_unique<LogPattern>(
                    u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message) "
                    u8"\n    \x1b[90mIn %(function_name) At %(file_name):%(file_line)\x1b[0m"
                ));
            SKR_ASSERT(ret && "Default log console pattern register failed!");
            ret = LogManager::RegisterSink(LogConstants::kDefaultConsoleSinkId, eastl::make_unique<LogConsoleSink>());
            SKR_ASSERT(ret && "Default log console sink register failed!");
            
            // register default file pattern & sink
            ret = LogManager::RegisterPattern(LogConstants::kDefaultFilePatternId, 
                eastl::make_unique<LogPattern>(
                    u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message) "
                    u8"\n    In %(function_name) At %(file_name):%(file_line)"
                ));
            SKR_ASSERT(ret && "Default log file pattern register failed!");
            ret = LogManager::RegisterSink(LogConstants::kDefaultFileSinkId, eastl::make_unique<LogFileSink>());
            SKR_ASSERT(ret && "Default log file sink register failed!");
        }
    );
    return logger_.get();
}

skr_guid_t LogManager::RegisterPattern(eastl::unique_ptr<LogPattern> pattern)
{
    auto guid = skr_guid_t();
    skr_make_guid(&guid);
    patterns_.emplace(guid, skr::move(pattern));
    return guid;
}

bool LogManager::RegisterPattern(skr_guid_t guid, eastl::unique_ptr<LogPattern> pattern)
{
    if (patterns_.find(guid) != patterns_.end())
        return false;
    patterns_.emplace(guid, skr::move(pattern));
    return true;
}

LogPattern* LogManager::QueryPattern(skr_guid_t guid)
{
    auto it = patterns_.find(guid);
    if (it != patterns_.end())
        return it->second.get();
    return nullptr;
}

skr_guid_t LogManager::RegisterSink(eastl::unique_ptr<LogSink> sink)
{
    auto guid = skr_guid_t();
    skr_make_guid(&guid);
    sinks_.emplace(guid, skr::move(sink));
    return guid;
}

bool LogManager::RegisterSink(skr_guid_t guid, eastl::unique_ptr<LogSink> sink)
{
    if (sinks_.find(guid) != sinks_.end())
        return false;
    sinks_.emplace(guid, skr::move(sink));
    return true;
}

LogSink* LogManager::QuerySink(skr_guid_t guid)
{
    auto it = sinks_.find(guid);
    if (it != sinks_.end())
        return it->second.get();
    return nullptr;
}

void LogManager::PatternAndSink(const LogEvent& event, skr::string_view formatted_message) SKR_NOEXCEPT
{
    eastl::fixed_hash_set<skr_guid_t, 4, 5, true, skr::guid::hash> patterns_;
    for (auto&& [id, sink] : sinks_)
    {
        auto pattern_id = sink->get_pattern();
        auto&& iter = patterns_.find(pattern_id);
        if (iter != patterns_.end())
            continue;
        
        if (auto p = LogManager::QueryPattern(pattern_id))
        {
            [[maybe_unused]] 
            auto& _ = p->pattern(event, formatted_message);
            patterns_.insert(pattern_id);
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
    }
    
    for (auto&& [id, sink] : sinks_)
    {
        auto pattern_id = sink->get_pattern();

        if (auto p = LogManager::QueryPattern(pattern_id))
        {
            sink->sink(event, p->last_result().view());
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
    }
}

void LogManager::DateTime::reset_date() SKR_NOEXCEPT
{
    time_t rawtime = LogManager::tscns_.rdns() / 1000000000;
    struct tm* timeinfo = ::localtime(&rawtime);
    timeinfo->tm_sec = timeinfo->tm_min = timeinfo->tm_hour = 0;
    midnightNs = ::mktime(timeinfo) * 1000000000;
    year = 1900 + timeinfo->tm_year;
    month = 1 + timeinfo->tm_mon;
    day = timeinfo->tm_mday;
}

LogWorker::LogWorker(const ServiceThreadDesc& desc) SKR_NOEXCEPT
    : AsyncService(desc), queue_(SPtr<LogQueue>::Create())
{

}

LogWorker::~LogWorker() SKR_NOEXCEPT
{
    drain();
    if (get_status() == skr::ServiceThread::Status::kStatusRunning)
    {
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_QUITING);
        stop();
    }
    wait_stop();
    exit();
}

void LogWorker::add_logger(Logger* logger) SKR_NOEXCEPT
{
    drain();
    loggers_.emplace_back(logger);
}

void LogWorker::remove_logger(Logger* logger) SKR_NOEXCEPT
{
    drain();
    loggers_.erase(std::remove(loggers_.begin(), loggers_.end(), logger), loggers_.end());
}

bool LogWorker::predicate() SKR_NOEXCEPT
{
    return queue_->query_cnt();
}

void LogWorker::process_logs() SKR_NOEXCEPT
{
    LogManager::tscns_.calibrate();
    
    LogElement e;
    while (queue_->try_dequeue(e))
    {
        ZoneScopedNC("LogSingle", tracy::Color::Orchid1);
        const auto& what = e.need_format ? 
            formatter_.format(e.format, e.args) :
            e.format;
        skr::log::LogManager::PatternAndSink(e.event, what.view());
    }
}

skr::AsyncResult LogWorker::serve() SKR_NOEXCEPT
{
    if (!predicate())
    {
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_SLEEPING);
        sleep();
        return ASYNC_RESULT_OK;
    }
    
    {
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_RUNNING);
        ZoneScopedNC("Dispatch", tracy::Color::Orchid3);
        process_logs();
    }
    return ASYNC_RESULT_OK;
}

} } // namespace skr::log

RUNTIME_EXTERN_C
void log_initialize_async_worker()
{
    skr::log::LogManager::Initialize();

    auto worker = skr::log::LogManager::TryGetWorker();
    SKR_ASSERT(worker && "worker must not be null & something is wrong with initialization!");

    ::atexit(+[]() {
        auto worker = skr::log::LogManager::TryGetWorker();
        SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
    });
}

RUNTIME_EXTERN_C
void log_set_level(int level)
{
    const auto kLogLevel = skr::log::LogConstants::kLogLevelsLUT[level];\
    skr::log::LogConstants::gLogLevel = kLogLevel;
}

RUNTIME_EXTERN_C 
void log_log(int level, const char* file, const char* func, const char* line, const char* fmt, ...)
{
    ZoneScopedN("Log");
    
    const auto kLogLevel = skr::log::LogConstants::kLogLevelsLUT[level];
    if (kLogLevel < skr::log::LogConstants::gLogLevel) return;

    auto logger = skr::log::LogManager::GetDefaultLogger();
    const skr::log::LogSourceData Src = { file, func, line  };
    const auto Event = skr::log::LogEvent(logger, kLogLevel, Src);

    va_list va_args;
    va_start(va_args, fmt);
    logger->log(Event, (const char8_t*)fmt, va_args);
    va_end(va_args);
}

RUNTIME_EXTERN_C 
void log_finalize()
{
    {
        if (auto worker = skr::log::LogManager::TryGetWorker())
            worker->drain();
    }
    skr::log::LogManager::Finalize();

    auto worker = skr::log::LogManager::TryGetWorker();
    SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
}