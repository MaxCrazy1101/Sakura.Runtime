// utils
#include "src/filesystem.c"
#include "src/stack_line_reader.c"
#include "src/string_view.c"

#ifdef __unix__
#include "src/hwcaps.c"
#endif

#if defined(__x86_64__) || defined(__i386__)

    #if defined(_WIN32) || defined(_WIN64)
    #include "src/impl_x86_windows.c"
    #elif defined(TARGET_MACOS)
    #define HAVE_SYSCTLBYNAME
    #include "src/impl_x86_macos.c"
    #elif defined(__linux__)
    #include "src/impl_x86_linux_or_android.c"
    #elif defined(__FreeBSD__)
    #include "src/impl_x86_freebsd.c"
    #endif
    
#elif defined(__arm__)

#include "src/impl_arm_linux_or_android.c"

#elif defined(__aarch64__)

#include "src/impl_aarch64_linux_or_android.c"

#elif defined(__mips__) && _MIPS_SIM == _ABI64

#include "src/impl_mips_linux_or_android.c"

#elif defined(__loongarch_lp64)

#elif defined(__powerpc64__)

#include "src/impl_ppc_linux.c"

#endif