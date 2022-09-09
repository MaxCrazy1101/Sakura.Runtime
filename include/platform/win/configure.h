#if !defined(SKR_HEADER_SCOPE_DEFINING_PLATFORM_CONFIGURE)
    #error "this file can only be included in "platform/configure.h"
#endif
#ifndef SKR_WINDOWS_CONFIGURE_H
    #define SKR_WINDOWS_CONFIGURE_H
#endif
#ifdef SKR_WINDOWS_CONFIGURE_H

    #include <stdio.h>

    #ifdef _DEBUG
        #include <assert.h>
    #else
        #ifndef assert
            #define assert(expr) (void)(expr);
        #endif
    #endif

    #if defined(_WIN32) || defined(XBOX)
        #define CHECK_HRESULT(exp)                                               \
            do                                                                   \
            {                                                                    \
                HRESULT hres = (exp);                                            \
                if (!SUCCEEDED(hres))                                            \
                {                                                                \
                    printf("%s: FAILED with HRESULT: %u", #exp, (uint32_t)hres); \
                    assert(false);                                               \
                }                                                                \
            } while (0)
    #endif

    #define OS_DPI 96

    #if !SKR_SHIPPING
        #include "platform/macros.h"

        #define SKR_TRACE_MSG(msg) \
            skr_debug_output(msg);
        #define SKR_ASSERT(cond)                                                      \
            do                                                                        \
            {                                                                         \
                if (!(cond))                                                          \
                {                                                                     \
                    SKR_TRACE_MSG("Skr Assert fired: " #cond " (" SKR_FILE_LINE ")"); \
                    __debugbreak();                                                   \
                }                                                                     \
            } while (0)
        #define SKR_BREAK() __debugbreak()
        #define SKR_HALT() __debugbreak()

    #endif
#endif

#define SKR_RUNTIME_USE_MIMALLOC