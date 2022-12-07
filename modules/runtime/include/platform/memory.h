#pragma once
#include "platform/configure.h"

RUNTIME_EXTERN_C RUNTIME_API void* _sakura_malloc(size_t size);
RUNTIME_EXTERN_C RUNTIME_API void* _sakura_calloc(size_t count, size_t size);
RUNTIME_EXTERN_C RUNTIME_API void* _sakura_malloc_aligned(size_t size, size_t alignment);
RUNTIME_EXTERN_C RUNTIME_API void* _sakura_calloc_aligned(size_t count, size_t size, size_t alignment);
RUNTIME_EXTERN_C RUNTIME_API void* _sakura_new_n(size_t count, size_t size);
RUNTIME_EXTERN_C RUNTIME_API void* _sakura_new_aligned(size_t size, size_t alignment);
RUNTIME_EXTERN_C RUNTIME_API void _sakura_free(void* p) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void _sakura_free_aligned(void* p, size_t alignment);
RUNTIME_EXTERN_C RUNTIME_API void* _sakura_realloc(void* p, size_t newsize);

#if defined(TRACY_ENABLE) && defined(TRACY_TRACE_ALLOCATION)
#include "string.h"
#include "tracy/TracyC.h"

#define SKR_ALLOC_TRACY_MARKER_COLOR 0xff0000
#define SKR_DEALLOC_TRACY_MARKER_COLOR 0x0000ff
FORCEINLINE void* SkrMallocWithCZone(size_t size, const char* line)
{
    TracyCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    TracyCZoneName(z, line, strlen(line));
    void* ptr = _sakura_malloc(size);
    TracyCZoneEnd(z);
    return ptr;
}

FORCEINLINE void* SkrCallocWithCZone(size_t count, size_t size, const char* line)
{
    TracyCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    TracyCZoneName(z, line, strlen(line));
    void* ptr = _sakura_calloc(count, size);
    TracyCZoneEnd(z);
    return ptr;
}

FORCEINLINE void* SkrMallocAlignedWithCZone(size_t size, size_t alignment, const char* line)
{
    TracyCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    TracyCZoneName(z, line, strlen(line));
    void* ptr = _sakura_malloc_aligned(size, alignment);
    TracyCZoneEnd(z);
    return ptr;
}

FORCEINLINE void* SkrCallocAlignedWithCZone(size_t count, size_t size, size_t alignment, const char* line)
{
    TracyCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    TracyCZoneName(z, line, strlen(line));
    void* ptr = _sakura_calloc_aligned(count, size, alignment);
    TracyCZoneEnd(z);
    return ptr;
}

FORCEINLINE void* SkrNewNWithCZone(size_t count, size_t size, const char* line)
{
    TracyCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    TracyCZoneName(z, line, strlen(line));
    void* ptr = _sakura_new_n(count, size);
    TracyCZoneEnd(z);
    return ptr;
}

FORCEINLINE void* SkrNewAlignedWithCZone(size_t size, size_t alignment, const char* line)
{
    TracyCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    TracyCZoneName(z, line, strlen(line));
    void* ptr = _sakura_new_aligned(size, alignment);
    TracyCZoneEnd(z);
    return ptr;
}

FORCEINLINE void SkrFreeWithCZone(void* p, const char* line)
{
    TracyCZoneC(z, SKR_DEALLOC_TRACY_MARKER_COLOR, 1);
    TracyCZoneName(z, line, strlen(line));
    _sakura_free(p);
    TracyCZoneEnd(z);
}

FORCEINLINE void SkrFreeAlignedWithCZone(void* p, size_t alignment, const char* line)
{
    TracyCZoneC(z, SKR_DEALLOC_TRACY_MARKER_COLOR, 1);
    TracyCZoneName(z, line, strlen(line));
    _sakura_free_aligned(p, alignment);
    TracyCZoneEnd(z);
}

FORCEINLINE void* SkrReallocWithCZone(void* p, size_t newsize, const char* line)
{
    TracyCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    TracyCZoneName(z, line, strlen(line));
    void* ptr = _sakura_realloc(p, newsize);
    TracyCZoneEnd(z);
    return ptr;
}

#define SKR_ALLOC_STRINGFY_IMPL(X) #X
#define SKR_ALLOC_STRINGFY(X) SKR_ALLOC_STRINGFY_IMPL(X)
#define SKR_ALLOC_CAT_IMPL(X,Y) X  Y
#define SKR_ALLOC_CAT(X,Y) SKR_ALLOC_CAT_IMPL(X,Y)

#define sakura_malloc(size) SkrMallocWithCZone((size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )
#define sakura_calloc(count, size) SkrCallocWithCZone((count), (size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )
#define sakura_malloc_aligned(size, alignment) SkrMallocAlignedWithCZone((size), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )
#define sakura_calloc_aligned(count, size, alignment) SkrCallocAlignedWithCZone((count), (size), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )
#define sakura_new_n(count, size) SkrNewNWithCZone((count), (size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )
#define sakura_new_aligned(size, alignment) SkrNewAlignedWithCZone((size), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )
#define sakura_free(p) SkrFreeWithCZone((p), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )
#define sakura_free_aligned(p, alignment) SkrFreeAlignedWithCZone((p), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )
#define sakura_realloc(p, newsize) SkrReallocWithCZone((p), (newsize), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )

#else
#define sakura_malloc(size) _sakura_malloc(size)
#define sakura_calloc(count, size) _sakura_calloc((count), (size))
#define sakura_malloc_aligned(size, alignment) _sakura_malloc_aligned((size), (alignment))
#define sakura_calloc_aligned(count, size, alignment) _sakura_calloc_aligned((count), (size), (alignment))
#define sakura_new_n(count, size) _sakura_new_n((count), (size))
#define sakura_new_aligned(size, alignment) _sakura_new_aligned((size), (alignment))
#define sakura_free(p) _sakura_free(p)
#define sakura_free_aligned(p, alignment) _sakura_free_aligned((p), (alignment))
#define sakura_realloc(p, newsize) _sakura_realloc((p), (newsize))
#endif

#ifdef _CRTDBG_MAP_ALLOC
    #define DEBUG_NEW_SOURCE_LINE (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
    #define DEBUG_NEW_SOURCE_LINE
#endif

#if defined(__cplusplus)
#include "platform/debug.h"
#include <cstddef>     // std::size_t
#include <cstdint>     // PTRDIFF_MAX
#if (__cplusplus >= 201103L) || (_MSC_VER > 1900)  // C++11
#include <type_traits> // std::true_type
#include <utility>     // std::forward
#endif

#include "tracy/Tracy.hpp"

#if defined(TRACY_ENABLE) && defined(TRACY_TRACE_ALLOCATION)
#include <string_view>
#include "utils/demangle.hpp"
template<std::string_view const& sourcelocation>
struct SkrTracedNew
{
    template<class T, class... TArgs>
    [[nodiscard]] FORCEINLINE T* New(TArgs&&... params)
    {
        const std::string_view name = skr::demangle<T>();
        TracyMessage(name.data(), name.size());
        void* pMemory = SkrNewAlignedWithCZone(sizeof(T), alignof(T), sourcelocation.data());
        SKR_ASSERT(pMemory != nullptr);
        return new (pMemory) DEBUG_NEW_SOURCE_LINE T{ std::forward<TArgs>(params)... };
    }

    template<class T>
    [[nodiscard]] FORCEINLINE T* New()
    {
        const std::string_view name = skr::demangle<T>();
        TracyMessage(name.data(), name.size());
        void* pMemory = SkrNewAlignedWithCZone(sizeof(T), alignof(T), sourcelocation.data());
        SKR_ASSERT(pMemory != nullptr);
        return new (pMemory) DEBUG_NEW_SOURCE_LINE T();
    }

    template<class T, class... TArgs>
    [[nodiscard]] FORCEINLINE T* NewSized(size_t size, TArgs&&... params)
    {
        const std::string_view name = skr::demangle<T>();
        TracyMessage(name.data(), name.size());
        SKR_ASSERT(size >= sizeof(T));
        void* pMemory = SkrNewAlignedWithCZone(sizeof(T), alignof(T), sourcelocation.data());
        SKR_ASSERT(pMemory != nullptr);
        return new (pMemory) DEBUG_NEW_SOURCE_LINE T{ std::forward<TArgs>(params)... };
    }

    template<class T>
    [[nodiscard]] FORCEINLINE T* NewSized(size_t size)
    {
        const std::string_view name = skr::demangle<T>();
        TracyMessage(name.data(), name.size());
        SKR_ASSERT(size >= sizeof(T));
        void* pMemory = SkrNewAlignedWithCZone(sizeof(T), alignof(T), sourcelocation.data());
        SKR_ASSERT(pMemory != nullptr);
        return new (pMemory) DEBUG_NEW_SOURCE_LINE T();
    }

    template<class F>
    [[nodiscard]] FORCEINLINE F* NewLambda(F&& lambda)
    {
        const std::string_view name = skr::demangle<T>();
        TracyMessage(name.data(), name.size());
        using ValueType = std::remove_reference_t<F>;
        void* pMemory = SkrNewAlignedWithCZone(sizeof(T), alignof(T), ssourcelocation.data());
        SKR_ASSERT(pMemory != nullptr);
        return new (pMemory) DEBUG_NEW_SOURCE_LINE auto(std::forward<F>(lambda));
    }

    template<class T>
    void Delete(T* pType)
    {
        if (pType != nullptr)
        {
            const std::string_view name = skr::demangle<T>();
            TracyMessage(name.data(), name.size());
            pType->~T();
            SkrFreeAlignedWithCZone((void*)pType, alignof(T), sourcelocation.data());
        }
    }
};
#define SkrNew []{ static constexpr std::string_view sourcelocation = SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)); return SkrTracedNew<sourcelocation>{}; }().template New
#define SkrNewSized []{ static constexpr std::string_view sourcelocation = SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)); return SkrTracedNew<sourcelocation>{}; }().template NewSized
#define SkrNewLambda []{ static constexpr std::string_view sourcelocation = SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)); return SkrTracedNew<sourcelocation>{}; }().template NewLambda
#define SkrDelete []{ static constexpr std::string_view sourcelocation = SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)); return SkrTracedNew<sourcelocation>{}; }().template Delete
#else
template <typename T, typename... TArgs>
[[nodiscard]] FORCEINLINE T* SkrNew(TArgs&&... params)
{
    ZoneScopedN("SkrNew");

    void* pMemory = sakura_new_aligned(sizeof(T), alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T{ std::forward<TArgs>(params)... };
}

template <typename T>
[[nodiscard]] FORCEINLINE T* SkrNew()
{
    ZoneScopedN("SkrNew");
    
    void* pMemory = sakura_new_aligned(sizeof(T), alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T();
}

template <typename T, typename... TArgs>
[[nodiscard]] FORCEINLINE T* SkrNewSized(size_t size, TArgs&&... params)
{
    ZoneScopedN("SkrNewSized");

    SKR_ASSERT(size >= sizeof(T));
    void* pMemory = sakura_new_aligned(size, alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T{ std::forward<TArgs>(params)... };
}

template <typename T>
[[nodiscard]] FORCEINLINE T* SkrNewSized(size_t size)
{
    ZoneScopedN("SkrNewSized");

    SKR_ASSERT(size >= sizeof(T));
    void* pMemory = sakura_new_aligned(size, alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T();
}

template <typename F>
[[nodiscard]] FORCEINLINE F* SkrNewLambda(F&& lambda)
{
    ZoneScopedN("SkrNewLambda");

    using ValueType = std::remove_reference_t<F>;
    void* pMemory = sakura_new_aligned(sizeof(ValueType), alignof(ValueType));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE auto(std::forward<F>(lambda));
}

template <typename T>
void SkrDelete(T* pType)
{
    if (pType != nullptr)
    {
        pType->~T();
        sakura_free_aligned((void*)pType, alignof(T));
    }
}
#endif

template<class T> 
struct skr_stl_allocator 
{
    typedef T                 value_type;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;
    typedef value_type&       reference;
    typedef value_type const& const_reference;
    typedef value_type*       pointer;
    typedef value_type const* const_pointer;
    template <class U> struct rebind { typedef skr_stl_allocator<U> other; };

    skr_stl_allocator()                                              SKR_NOEXCEPT = default;
    skr_stl_allocator(const skr_stl_allocator&)                      SKR_NOEXCEPT = default;
    template<class U> skr_stl_allocator(const skr_stl_allocator<U>&) SKR_NOEXCEPT { }
    skr_stl_allocator  select_on_container_copy_construction() const { return *this; }
    void              deallocate(T* p, size_type) { sakura_free(p); }

#if (__cplusplus >= 201703L)  // C++17
    [[nodiscard]] T* allocate(size_type count) { return static_cast<T*>(sakura_new_n(count, sizeof(T))); }
    [[nodiscard]] T* allocate(size_type count, const void*) { return allocate(count); }
#else
    [[nodiscard]] pointer allocate(size_type count, const void* = 0) { return static_cast<pointer>(sakura_new_n(count, sizeof(value_type))); }
#endif

#if ((__cplusplus >= 201103L) || (_MSC_VER > 1900))  // C++11
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap            = std::true_type;
    using is_always_equal                        = std::true_type;
    template <class U, class ...Args> void construct(U* p, Args&& ...args) { ::new(p) U(std::forward<Args>(args)...); }
    template <class U> void destroy(U* p) SKR_NOEXCEPT { p->~U(); }
#else
    void construct(pointer p, value_type const& val) { ::new(p) value_type(val); }
    void destroy(pointer p) { p->~value_type(); }
#endif

    size_type     max_size() const SKR_NOEXCEPT { return (PTRDIFF_MAX / sizeof(value_type)); }
    pointer       address(reference x) const        { return &x; }
    const_pointer address(const_reference x) const  { return &x; }
};

template<class T1,class T2> bool operator==(const skr_stl_allocator<T1>& , const skr_stl_allocator<T2>& ) SKR_NOEXCEPT { return true; }
template<class T1,class T2> bool operator!=(const skr_stl_allocator<T1>& , const skr_stl_allocator<T2>& ) SKR_NOEXCEPT { return false; }
#endif // __cplusplus