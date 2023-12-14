#pragma once
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename K, typename V, typename Hasher = Hash<K>>
using UMap = container::SparseHashMap<container::SparseHashMapMemory<
K,                  /*Key Type*/
V,                  /*Value Type*/
uint64_t,           /*BitBlock Type*/
uint64_t,           /*Hash Type*/
Hasher,             /*Hasher Type*/
Equal<K>,           /*Comparer Type*/
false,              /*Allow MultiKey*/
uint64_t,           /*Size Type*/
SkrAllocator_New>>; /*Allocator Type*/
}