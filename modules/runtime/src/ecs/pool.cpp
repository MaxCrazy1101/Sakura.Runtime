#include "SkrProfile/profile.h"
#include "SkrRT/ecs/dual_config.h"
#include "SkrRT/containers_new/array.hpp"
#include "pool.hpp"
#include <numeric>

const char* kDualMemoryName = "dual";
namespace dual
{
pool_t::pool_t(size_t blockSize, size_t blockCount)
    : blockSize(blockSize)
    , blocks(blockCount)
{
}

pool_t::~pool_t()
{
    void* block;
    while (blocks.try_dequeue(block))
        dual_free(block);
}

void* pool_t::allocate()
{
    void* block;
    if (blocks.try_dequeue(block))
        return block;
    {
        SkrZoneScopedN("DualPoolAllocation");
        return dual_calloc(1, blockSize);
    }
}

void pool_t::free(void* block)
{
    if (blocks.try_enqueue(block))
        return;
    dual_free(block);
}

fixed_pool_t::fixed_pool_t(size_t blockSize, size_t blockCount)
    : blockSize(blockSize)
    , blockCount(blockCount)
    , blocks(blockCount)
{
    buffer = new char[blockSize * blockCount];
    skr::vector<size_t> indicies;
    indicies.resize_default(blockCount);
    std::iota(indicies.begin(), indicies.end(), 0);
    for (size_t i = 0; i < blockCount; ++i)
        blocks.try_enqueue_bulk(indicies.data(), blockCount);
}

fixed_pool_t::~fixed_pool_t()
{
    delete buffer;
}

void* fixed_pool_t::allocate()
{
    size_t block;
    if (blocks.try_dequeue(block))
        return buffer + block * blockSize;
    return nullptr;
}

void fixed_pool_t::free(void* block)
{
    if (block < buffer || block > buffer + blockSize * blockCount)
        return;
    if (blocks.try_enqueue(((char*)block - buffer) / blockSize))
        return;
}

void fixed_pool_t::reset()
{
    skr::ConcurrentQueue<size_t, ECSPoolConcurrentQueueTraits> temp(blockCount);
    blocks.swap(temp);
    skr::vector<size_t> indicies;
    indicies.resize_default(blockCount);
    std::iota(indicies.begin(), indicies.end(), 0);
    for (size_t i = 0; i < blockCount; ++i)
        blocks.try_enqueue_bulk(indicies.data(), blockCount);
}
} // namespace dual