#pragma once
#include "sparse_hash_set_def.hpp"
#include "SkrBase/containers/sparse_array/sparse_array_iterator.hpp"

// SparseHashSet iterator
namespace skr::container
{
template <typename T, typename TBitBlock, typename TS, typename TH, bool Const>
struct SparseHashSetIt {
    using DataType       = SparseHashSetData<T, TS, TH>;
    using SparseDataType = std::conditional_t<Const, const SparseArrayData<DataType, TS>, SparseArrayData<DataType, TS>>;
    using ValueType      = std::conditional_t<Const, const T, T>;
    using BitItType      = TrueBitIt<TBitBlock, TS, true>;

    SKR_INLINE explicit SparseHashSetIt(SparseDataType* array, TS array_size, const TBitBlock* bit_array, TS start = 0)
        : _array(array)
        , _bit_it(bit_array, array_size, start)
    {
    }

    // impl cpp iterator
    SKR_INLINE SparseHashSetIt& operator++()
    {
        ++_bit_it;
        return *this;
    }
    SKR_INLINE bool       operator==(const SparseHashSetIt& rhs) const { return _bit_it == rhs._bit_it && _array == rhs._array; }
    SKR_INLINE bool       operator!=(const SparseHashSetIt& rhs) const { return !(*this == rhs); }
    SKR_INLINE explicit   operator bool() const { return (bool)_bit_it; }
    SKR_INLINE bool       operator!() const { return !(bool)*this; }
    SKR_INLINE ValueType& operator*() const { return _array[index()]._sparse_array_data._sparse_hash_set_data; }
    SKR_INLINE ValueType* operator->() const { return &_array[index()]._sparse_array_data._sparse_hash_set_data; }

    // other data
    SKR_INLINE TS index() const { return _bit_it.index(); }
    SKR_INLINE TH hash() const { return _array[index()]._sparse_array_data._sparse_hash_set_hash; }

private:
    SparseDataType* _array;
    BitItType       _bit_it;
};
} // namespace skr::container

namespace skr::container
{
template <typename T, typename TBitBlock, typename TS, typename THash, bool kConst>
struct SparseHashSetCursor {
    using SparseArrayCursorType = SparseArrayCursor<SparseHashSetData<T, TS, THash>, TBitBlock, TS, kConst>;
    using DataType              = std::conditional_t<kConst, const T, T>;
    using SizeType              = TS;

    // ctor & copy & move & assign & move assign
    inline SparseHashSetCursor(const SparseArrayCursorType& sparse_array_cursor)
        : _cursor(sparse_array_cursor)
    {
    }
    inline SparseHashSetCursor(const SparseHashSetCursor& rhs)            = default;
    inline SparseHashSetCursor(SparseHashSetCursor&& rhs)                 = default;
    inline SparseHashSetCursor& operator=(const SparseHashSetCursor& rhs) = default;
    inline SparseHashSetCursor& operator=(SparseHashSetCursor&& rhs)      = default;

    // no factory, just wrapper of SparseArrayCursor

    // getter
    inline DataType& ref() const { return _cursor.ref()._sparse_hash_set_data; }
    inline DataType* ptr() const { return &_cursor.ref()._sparse_hash_set_data; }
    inline THash     hash() const { return _cursor.ref()._sparse_hash_set_hash; }
    inline SizeType  index() const { return _cursor.index(); }

    // move & validator
    inline void move_next() { _cursor.move_next(); }
    inline void move_prev() { _cursor.move_prev(); }
    inline void reset_to_begin() { _cursor.reset_to_begin(); }
    inline void reset_to_end() { _cursor.reset_to_end(); }

    // reach & validate
    inline bool reach_begin() const { return _cursor.reach_begin(); }
    inline bool reach_end() const { return _cursor.reach_end(); }
    inline bool is_valid() const { return _cursor.is_valid(); }

    // compare
    inline bool operator==(const SparseHashSetCursor& rhs) const { return _cursor == rhs._cursor; }
    inline bool operator!=(const SparseHashSetCursor& rhs) const { return !(*this == rhs); }

private:
    SparseArrayCursorType _cursor;
};

template <typename T, typename TBitBlock, typename TS, typename THash, bool kConst>
struct SparseHashSetIter {
    using SparseArrayCursorType = SparseArrayCursor<SparseHashSetData<T, TS, THash>, TBitBlock, TS, kConst>;
    using CursorType            = SparseHashSetCursor<T, TBitBlock, TS, THash, kConst>;
    using DataType              = std::conditional_t<kConst, const T, T>;
    using SizeType              = TS;

    // ctor & copy & move & assign & move assign
    inline SparseHashSetIter(SparseArrayCursorType sparse_array_cursor)
        : _cursor(sparse_array_cursor)
    {
    }
    inline SparseHashSetIter(const SparseHashSetIter& rhs)            = default;
    inline SparseHashSetIter(SparseHashSetIter&& rhs)                 = default;
    inline SparseHashSetIter& operator=(const SparseHashSetIter& rhs) = default;
    inline SparseHashSetIter& operator=(SparseHashSetIter&& rhs)      = default;

    // getter
    inline DataType& ref() const { return _cursor.ref(); }
    inline DataType* ptr() const { return _cursor.ptr(); }
    inline THash     hash() const { return _cursor.hash(); }
    inline SizeType  index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_begin(); }
    inline void move_next() { _cursor.move_next(); }
    inline bool has_next() const { return !_cursor.reach_end(); }

private:
    CursorType _cursor;
};

template <typename T, typename TBitBlock, typename TS, typename THash, bool kConst>
struct SparseHashSetIterInv {
    using SparseArrayCursorType = SparseArrayCursor<SparseHashSetData<T, TS, THash>, TBitBlock, TS, kConst>;
    using CursorType            = SparseHashSetCursor<T, TBitBlock, TS, THash, kConst>;
    using DataType              = std::conditional_t<kConst, const T, T>;
    using SizeType              = TS;

    // ctor & copy & move & assign & move assign
    inline SparseHashSetIterInv(SparseArrayCursorType sparse_array_cursor)
        : _cursor(sparse_array_cursor)
    {
    }
    inline SparseHashSetIterInv(const SparseHashSetIterInv& rhs)            = default;
    inline SparseHashSetIterInv(SparseHashSetIterInv&& rhs)                 = default;
    inline SparseHashSetIterInv& operator=(const SparseHashSetIterInv& rhs) = default;
    inline SparseHashSetIterInv& operator=(SparseHashSetIterInv&& rhs)      = default;

    // getter
    inline DataType& ref() const { return _cursor.ref(); }
    inline DataType* ptr() const { return _cursor.ptr(); }
    inline THash     hash() const { return _cursor.hash(); }
    inline SizeType  index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_end(); }
    inline void move_next() { _cursor.move_prev(); }
    inline bool has_next() const { return !_cursor.reach_begin(); }

private:
    CursorType _cursor;
};
} // namespace skr::container
