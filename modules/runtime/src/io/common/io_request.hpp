#pragma once
#include "../components/status_component.hpp"
#include "../components/src_components.hpp"
#include "../components/blocks_component.hpp"

#include "pool.hpp"
#include <tuple>
#include "tracy/Tracy.hpp"

namespace skr {
namespace io {

template<typename T>
[[nodiscard]] const T* io_component(const IIORequest* rq) SKR_NOEXCEPT
{
    if (auto c = rq->get_component(IORequestComponentTID<T>::Get()))
        return static_cast<const T*>(c);
    SKR_UNREACHABLE_CODE();
    return nullptr;
}

template<typename T>
[[nodiscard]] T* io_component(IIORequest* rq) SKR_NOEXCEPT
{
    if (auto c = rq->get_component(IORequestComponentTID<T>::Get()))
        return static_cast<T*>(c);
    SKR_UNREACHABLE_CODE();
    return nullptr;
}

template <typename Interface, typename...Components>
struct IORequestMixin : public Interface
{
    IO_RC_OBJECT_BODY
public:
    IORequestMixin(ISmartPoolPtr<Interface> pool) 
        : components(std::make_tuple(Components(this)...)), pool(pool)
    {

    }
    virtual ~IORequestMixin() = default;

    [[nodiscard]] virtual const IORequestComponent* get_component(skr_guid_t tid) const SKR_NOEXCEPT
    {
        ZoneScopedN("IORequestMixin::get_component");
        return std::apply([tid](const auto&... args) {
            const IORequestComponent* cs[] = { &args... };
            const skr_guid_t ids[] = { args.get_tid()... };
            for (uint64_t i = 0; i < sizeof...(Components); ++i)
            {
                if (ids[i] == tid)
                    return cs[i];
            }
            SKR_UNREACHABLE_CODE();
            return cs[0];
        }, components);
    }
    [[nodiscard]] virtual IORequestComponent* get_component(skr_guid_t tid) SKR_NOEXCEPT
    {
        ZoneScopedN("IORequestMixin::get_component");
        return std::apply([tid](auto&... args) {
            IORequestComponent* cs[] = { &args... };
            const skr_guid_t ids[] = { args.get_tid()... };
            for (uint64_t i = 0; i < sizeof...(Components); ++i)
            {
                if (ids[i] == tid)
                    return cs[i];
            }
            SKR_UNREACHABLE_CODE();
            return cs[0];
        }, components);
    }

    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<IORequestMixin*>(ptr);
            p->pool->deallocate(p); 
        };
    }
    
private:
    std::tuple<Components...> components;
    ISmartPoolPtr<Interface> pool = nullptr;

public:
    template <typename C>
    C* safe_comp() SKR_NOEXCEPT
    {
        auto c = io_component<C>(this);
        SKR_ASSERT(c && "failed to get component!");
        return c;
    }

    template <typename C>
    const C* safe_comp() const SKR_NOEXCEPT
    {
        auto c = io_component<C>(this);
        SKR_ASSERT(c && "failed to get component!");
        return c;
    }

    void set_vfs(skr_vfs_t* _vfs) SKR_NOEXCEPT
    {
        safe_comp<PathSrcComponent>()->set_vfs(_vfs);
    }

    void set_path(const char8_t* p) SKR_NOEXCEPT 
    { 
        safe_comp<PathSrcComponent>()->set_path(p); 
    }

    [[nodiscard]] const char8_t* get_path() const SKR_NOEXCEPT 
    { 
        return safe_comp<PathSrcComponent>()->get_path(); 
    }

    void use_async_complete() SKR_NOEXCEPT 
    { 
        safe_comp<IOStatusComponent>()->use_async_complete(); 
    }

    void use_async_cancel() SKR_NOEXCEPT 
    { 
        safe_comp<IOStatusComponent>()->use_async_cancel(); 
    }

    const skr_io_future_t* get_future() const SKR_NOEXCEPT 
    { 
        return safe_comp<IOStatusComponent>()->get_future(); 
    }

    void add_callback(ESkrIOStage stage, IOCallback callback, void* data) SKR_NOEXCEPT
    {
        safe_comp<IOStatusComponent>()->add_callback(stage, callback, data);
    }
    
    void add_finish_callback(ESkrIOFinishPoint point, IOCallback callback, void* data) SKR_NOEXCEPT
    {
        safe_comp<IOStatusComponent>()->add_finish_callback(point, callback, data);
    }

    skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT 
    { 
        return safe_comp<BlocksComponent>()->get_blocks(); 
    }

    void add_block(const skr_io_block_t& block) SKR_NOEXCEPT 
    { 
        safe_comp<BlocksComponent>()->add_block(block); 
    }

    void reset_blocks() SKR_NOEXCEPT 
    { 
        safe_comp<BlocksComponent>()->reset_blocks(); 
    }

    skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT 
    { 
        return safe_comp<CompressedBlocksComponent>()->get_compressed_blocks(); 
    }

    void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT
    {
        safe_comp<CompressedBlocksComponent>()->add_compressed_block(block); 
    }

    void reset_compressed_blocks() SKR_NOEXCEPT
    {
        safe_comp<CompressedBlocksComponent>()->reset_compressed_blocks(); 
    }
};

using IORequestQueue = IOConcurrentQueue<IORequestId>;  
using IORequestArray = skr::vector<IORequestId>;

} // namespace io
} // namespace skr