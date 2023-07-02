/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#pragma once
#include "platform/configure.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_VERSION "0.1.0"

enum
{
    SKR_LOG_LEVEL_TRACE,
    SKR_LOG_LEVEL_DEBUG,
    SKR_LOG_LEVEL_INFO,
    SKR_LOG_LEVEL_WARN,
    SKR_LOG_LEVEL_ERROR,
    SKR_LOG_LEVEL_FATAL
};

#define __LOG_FUNC__ __FUNCTION__ 

#define SKR_LOG_TRACE(...) log_log(SKR_LOG_LEVEL_TRACE, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_DEBUG(...) log_log(SKR_LOG_LEVEL_DEBUG, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_INFO(...) log_log(SKR_LOG_LEVEL_INFO, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_WARN(...) log_log(SKR_LOG_LEVEL_WARN, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_ERROR(...) log_log(SKR_LOG_LEVEL_ERROR, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_FATAL(...) log_log(SKR_LOG_LEVEL_FATAL, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)

RUNTIME_API void log_initialize_async_worker();

RUNTIME_API void log_set_level(int level);

RUNTIME_API void log_log(int level, const char* file, const char* func, const char* line, const char* fmt, ...);

// flush logs of this thread
RUNTIME_API void log_flush();

RUNTIME_API void log_finalize();

#ifdef __cplusplus
}
#endif

