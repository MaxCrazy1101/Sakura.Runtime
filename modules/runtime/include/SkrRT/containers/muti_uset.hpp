#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename T, typename Hasher = Hash<typename container::KeyTraits<T>::KeyType>, typename Allocator = SkrAllocator_New>
using MultiUSet = container::SparseHashSet<container::SparseHashSetMemory<
T,                                                /*element Type*/
uint64_t,                                         /*BitBlock Type*/
uint64_t,                                         /*Hash Type*/
Hasher,                                           /*Hasher Type*/
Equal<typename container::KeyTraits<T>::KeyType>, /*Comparer Type*/
true,                                             /*Allow MultiKey*/
uint64_t,                                         /*Size Type*/
Allocator>>;                                      /*Allocator Type*/

template <typename T, uint64_t kCount, typename Hasher = Hash<typename container::KeyTraits<T>::KeyType>>
using FixedMultiUSet = container::SparseHashSet<container::FixedSparseHashSetMemory<
T,                                                /*element Type*/
uint64_t,                                         /*BitBlock Type*/
uint64_t,                                         /*Hash Type*/
Hasher,                                           /*Hasher Type*/
Equal<typename container::KeyTraits<T>::KeyType>, /*Comparer Type*/
true,                                             /*Allow MultiKey*/
uint64_t,                                         /*Size Type*/
kCount>>;                                         /*Allocator Type*/
} // namespace skr