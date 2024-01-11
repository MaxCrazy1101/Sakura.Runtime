#include "SkrTestFramework/framework.hpp"
#include "container_test_types.hpp"

template <typename TestArray, typename ModifyCapacity, typename ClampCapacity, typename CheckData, typename CheckNoData, typename CheckDataEQ>
void template_test_array(ModifyCapacity&& capacity_of, ClampCapacity&& clamp_capacity, CheckData&& check_data, CheckNoData&& check_no_data, CheckDataEQ&& check_data_eq)
{
    using namespace skr;

    SUBCASE("ctor")
    {
        TestArray a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);

        TestArray b(20);
        REQUIRE_EQ(b.size(), 20);
        REQUIRE_GE(b.capacity(), capacity_of(20));
        check_data(b);

        TestArray c(20, 114514);
        REQUIRE_EQ(c.size(), 20);
        REQUIRE_GE(c.capacity(), capacity_of(20));
        check_data(c);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(c[i], 114514);
        }

        TestArray d(c.data(), c.size() - 5);
        REQUIRE_EQ(d.size(), 15);
        REQUIRE_GE(d.capacity(), capacity_of(15));
        check_data(d);
        for (uint32_t i = 0; i < 15; ++i)
        {
            REQUIRE_EQ(d[i], 114514);
        }

        TestArray e({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(e.size(), 6);
        REQUIRE_GE(e.capacity(), capacity_of(6));
        check_data(e);
        REQUIRE_EQ(e[0], 1);
        REQUIRE_EQ(e[1], 1);
        REQUIRE_EQ(e[2], 4);
        REQUIRE_EQ(e[3], 5);
        REQUIRE_EQ(e[4], 1);
        REQUIRE_EQ(e[5], 4);
    }

    SUBCASE("copy & move")
    {
        TestArray a(100, 114514);

        TestArray b = a;
        REQUIRE_EQ(b.size(), a.size());
        REQUIRE_GE(b.capacity(), capacity_of(a.capacity()));
        check_data(b);

        auto      old_size     = a.size();
        auto      old_capacity = a.capacity();
        auto      old_data     = a.data();
        TestArray c            = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);
        REQUIRE_EQ(c.size(), old_size);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        check_data_eq(c, old_data);
    }

    SUBCASE("assign & move assign")
    {
        TestArray a(100, 114514), b, c;

        b = a;
        REQUIRE_EQ(b.size(), a.size());
        REQUIRE_GE(b.capacity(), capacity_of(a.size()));
        check_data(b);

        auto old_size     = a.size();
        auto old_capacity = a.capacity();
        auto old_data     = a.data();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);
        REQUIRE_EQ(c.size(), old_size);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        check_data_eq(c, old_data);
    }

    SUBCASE("spacial assign")
    {
        TestArray a(100, 114514);
        TestArray b(200, 114);

        a.assign({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_GE(a.capacity(), capacity_of(100));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        a.assign(b.data(), b.size());
        REQUIRE_EQ(a.size(), 200);
        REQUIRE_GE(a.capacity(), capacity_of(200));
        for (uint32_t i = 0; i < 200; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
    }

    SUBCASE("compare")
    {
        TestArray a({ 1, 1, 4, 5, 1, 4 });
        TestArray b(6);
        TestArray c(10, 114514);
        TestArray d(6, 114514);
        b[0] = 1;
        b[1] = 1;
        b[2] = 4;
        b[3] = 5;
        b[4] = 1;
        b[5] = 4;
        REQUIRE_EQ(a, b);
        REQUIRE_NE(b, c);
        REQUIRE_NE(b, d);
        REQUIRE_NE(a, d);
        REQUIRE_NE(c, d);
    }

    // [needn't test] getter

    SUBCASE("validate")
    {
        TestArray a(10), b;

        REQUIRE_FALSE(a.is_valid_index(-1));
        REQUIRE_FALSE(a.is_valid_index(11));
        REQUIRE(a.is_valid_index(5));
        REQUIRE(a.is_valid_index(0));
        REQUIRE(a.is_valid_index(9));

        REQUIRE_FALSE(b.is_valid_index(-1));
        REQUIRE_FALSE(b.is_valid_index(0));
        REQUIRE_FALSE(b.is_valid_index(1));

        REQUIRE(a.is_valid_pointer(a.begin()));
        REQUIRE(a.is_valid_pointer(a.begin() + 5));
        REQUIRE(a.is_valid_pointer(a.end() - 1));
        REQUIRE_FALSE(a.is_valid_pointer(a.begin() - 1));
        REQUIRE_FALSE(a.is_valid_pointer(a.end()));
    }

    SUBCASE("memory op")
    {
        TestArray a(50, 114514);

        auto old_capacity = a.capacity();
        auto old_data     = a.data();
        a.clear();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
        check_data_eq(a, old_data);

        a = { 1, 1, 4, 5, 1, 4 };
        a.release(20);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(20));

        a.release();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);

        a.reserve(60);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(60));
        check_data(a);

        a.shrink();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);

        a = TestArray(10, 114514);
        a.resize(40, 1145140);
        REQUIRE_EQ(a.size(), 40);
        REQUIRE_GE(a.capacity(), capacity_of(40));
        check_data(a);
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 40; ++i)
        {
            REQUIRE_EQ(a[i], 1145140);
        }

        old_capacity = a.capacity();
        a.clear();
        a.resize_unsafe(36);
        REQUIRE_EQ(a.size(), 36);
        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
        check_data(a);

        a.clear();
        a.resize_default(38);
        REQUIRE_EQ(a.size(), 38);
        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
        check_data(a);

        a.clear();
        a.resize_zeroed(21);
        REQUIRE_EQ(a.size(), 21);
        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
        check_data(a);
        for (uint32_t i = 0; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 0);
        }
    }

    SUBCASE("add")
    {
        TestArray a(10, 114514);
        a.add(1145140, 5);
        a.add(114514, 20);
        REQUIRE_EQ(a.size(), 35);
        REQUIRE_GE(a.capacity(), capacity_of(35));
        check_data(a);
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 15; ++i)
        {
            REQUIRE_EQ(a[i], 1145140);
        }
        for (uint32_t i = 15; i < 35; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        auto old_capacity = a.capacity();
        a.clear();
        a.add_unique(1);
        a.add_unique(1);
        a.add_unique(4);
        a.add_unique(5);
        a.add_unique(1);
        a.add_unique(4);
        REQUIRE_EQ(a.size(), 3);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 4);
        REQUIRE_EQ(a[2], 5);

        a.add_zeroed();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a[3], 0);

        a.add_unsafe();
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a[4], 114514);

        a.add_default();
        REQUIRE_EQ(a.size(), 6);

        a.add_unsafe(10);
        REQUIRE_EQ(a.size(), 16);

        a.add_default(10);
        REQUIRE_EQ(a.size(), 26);

        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
    }

    SUBCASE("add at")
    {
        TestArray a(10, 114514);

        a.add_at(5, 1145140, 20);
        REQUIRE_EQ(a.size(), 30);
        for (uint32_t i = 0; i < 5; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 5; i < 25; ++i)
        {
            REQUIRE_EQ(a[i], 1145140);
        }
        for (uint32_t i = 25; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.clear();
        a.add(10, 50);
        a.add_at_zeroed(25, 100);
        REQUIRE_EQ(a.size(), 150);
        for (uint32_t i = 0; i < 25; ++i)
        {
            REQUIRE_EQ(a[i], 10);
        }
        for (uint32_t i = 25; i < 125; ++i)
        {
            REQUIRE_EQ(a[i], 0);
        }
        for (uint32_t i = 125; i < 150; ++i)
        {
            REQUIRE_EQ(a[i], 10);
        }

        a.clear();
        a.add(114514, 30);
        a.add_at_unsafe(15, 10);
        for (uint32_t i = 0; i < 15; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 25; i < 40; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.clear();
        a.add(114514, 30);
        a.add_at_default(15, 10);
        for (uint32_t i = 0; i < 15; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 25; i < 40; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
    }

    SUBCASE("emplace")
    {
        TestArray a(10, 114514);
        a.emplace(10);
        REQUIRE_EQ(a.size(), 11);
        REQUIRE_GE(a.capacity(), capacity_of(11));
        check_data(a);
        REQUIRE_EQ(a[10], 10);

        a.emplace_at(5, 25);
        REQUIRE_EQ(a.size(), 12);
        REQUIRE_GE(a.capacity(), capacity_of(12));
        check_data(a);
        for (uint32_t i = 0; i < 5; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[5], 25);
        for (uint32_t i = 6; i < 11; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[11], 10);
    }

    SUBCASE("append")
    {
        TestArray a(20, 114514);
        TestArray b(10, 114);

        a.append(b);
        REQUIRE_EQ(a.size(), 30);
        REQUIRE_GE(a.capacity(), capacity_of(30));
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }

        a.append({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 36);
        REQUIRE_GE(a.capacity(), capacity_of(36));
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        REQUIRE_EQ(a[30], 1);
        REQUIRE_EQ(a[31], 1);
        REQUIRE_EQ(a[32], 4);
        REQUIRE_EQ(a[33], 5);
        REQUIRE_EQ(a[34], 1);
        REQUIRE_EQ(a[35], 4);

        a.append(b.data(), 5);
        REQUIRE_EQ(a.size(), 41);
        REQUIRE_GE(a.capacity(), capacity_of(41));
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        REQUIRE_EQ(a[30], 1);
        REQUIRE_EQ(a[31], 1);
        REQUIRE_EQ(a[32], 4);
        REQUIRE_EQ(a[33], 5);
        REQUIRE_EQ(a[34], 1);
        REQUIRE_EQ(a[35], 4);
        for (uint32_t i = 36; i < 41; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
    }

    SUBCASE("append at")
    {
        TestArray a(20, 114514);
        TestArray b(10, 114);

        a.append_at(10, b);
        REQUIRE_EQ(a.size(), 30);
        REQUIRE_GE(a.capacity(), capacity_of(30));
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.append_at(20, { 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 36);
        REQUIRE_GE(a.capacity(), capacity_of(36));
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        REQUIRE_EQ(a[20], 1);
        REQUIRE_EQ(a[21], 1);
        REQUIRE_EQ(a[22], 4);
        REQUIRE_EQ(a[23], 5);
        REQUIRE_EQ(a[24], 1);
        REQUIRE_EQ(a[25], 4);
        for (uint32_t i = 26; i < 36; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.append_at(10, b.data(), 5);
        REQUIRE_EQ(a.size(), 41);
        REQUIRE_GE(a.capacity(), capacity_of(41));
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 25; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        REQUIRE_EQ(a[25], 1);
        REQUIRE_EQ(a[26], 1);
        REQUIRE_EQ(a[27], 4);
        REQUIRE_EQ(a[28], 5);
        REQUIRE_EQ(a[29], 1);
        REQUIRE_EQ(a[30], 4);
        for (uint32_t i = 31; i < 41; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
    }

    SUBCASE("remove")
    {
        TestArray a = { 1, 1, 4, 5, 1, 4 };
        a.add(114514, 20);

        a.remove_at(0, 2);
        REQUIRE_EQ(a.size(), 24);
        REQUIRE_GE(a.capacity(), capacity_of(26));
        REQUIRE_EQ(a[0], 4);
        REQUIRE_EQ(a[1], 5);
        REQUIRE_EQ(a[2], 1);
        REQUIRE_EQ(a[3], 4);
        for (uint32_t i = 4; i < 24; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.remove_at_swap(0, 2);
        REQUIRE_EQ(a.size(), 22);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 1);
        REQUIRE_EQ(a[3], 4);
        for (uint32_t i = 4; i < 22; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove(1);
        REQUIRE_EQ(a.size(), 27);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[21], 1);
        REQUIRE_EQ(a[22], 1);
        REQUIRE_EQ(a[23], 4);
        REQUIRE_EQ(a[24], 5);
        REQUIRE_EQ(a[25], 1);
        REQUIRE_EQ(a[26], 4);

        a.remove_swap(1);
        REQUIRE_EQ(a.size(), 26);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[21], 4);
        REQUIRE_EQ(a[22], 1);
        REQUIRE_EQ(a[23], 4);
        REQUIRE_EQ(a[24], 5);
        REQUIRE_EQ(a[25], 1);

        a.remove_last(1);
        REQUIRE_EQ(a.size(), 25);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[21], 4);
        REQUIRE_EQ(a[22], 1);
        REQUIRE_EQ(a[23], 4);
        REQUIRE_EQ(a[24], 5);

        a.remove_last_swap(1);
        REQUIRE_EQ(a.size(), 24);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[21], 4);
        REQUIRE_EQ(a[22], 5);
        REQUIRE_EQ(a[23], 4);

        a.remove_all_swap(4);
        REQUIRE_EQ(a.size(), 21);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 5);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.append({ 5, 5, 5 });
        a.remove_all(5);
        REQUIRE_EQ(a.size(), 20);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        TestArray aa = { 5, 1, 2, 5, 5, 2, 5 };
        aa.remove_all(5);
        REQUIRE_EQ(aa.size(), 3);
        for (uint32_t i = 0; i < aa.size(); ++i)
        {
            REQUIRE_NE(aa[i], 5);
        }
    }

    // [test in remove] remove if

    SUBCASE("erase")
    {
        uint32_t       raw_data_group[]     = { 1, 1, 4, 5, 1, 4 };
        uint32_t       removed_data_group[] = { 4, 5, 4 };
        const uint64_t kArraySize           = clamp_capacity(114514);

        TestArray a(kArraySize), b(kArraySize);
        for (uint32_t i = 0; i < kArraySize; ++i)
        {
            a[i] = raw_data_group[i % 6];
            b[i] = raw_data_group[i % 6];
        }

        for (auto it = a.begin(); it != a.end();)
        {
            if (*it == 1)
            {
                it = a.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (uint32_t i = 0; i < a.size(); ++i)
        {
            REQUIRE_EQ(a[i], removed_data_group[i % 3]);
        }

        for (auto it = a.begin(); it != a.end();)
        {
            if (*it == 5)
            {
                it = a.erase_swap(it);
            }
            else
            {
                ++it;
            }
        }
        REQUIRE(!a.contains(5));

        const TestArray& cb = b;
        for (auto it = cb.begin(); it != cb.end();)
        {
            if (*it == 1)
            {
                it = b.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (uint32_t i = 0; i < b.size(); ++i)
        {
            REQUIRE_EQ(b[i], removed_data_group[i % 3]);
        }

        for (auto it = cb.begin(); it != cb.end();)
        {
            if (*it == 5)
            {
                it = b.erase_swap(it);
            }
            else
            {
                ++it;
            }
        }
        REQUIRE(!b.contains(5));
    }

    // [needn't test] modify

    SUBCASE("find")
    {
        TestArray a({ 1, 1, 4, 5, 1, 4 });

        REQUIRE_EQ(a.find(1).index(), 0);
        REQUIRE_EQ(a.find(4).index(), 2);
        REQUIRE_EQ(a.find(5).index(), 3);
        REQUIRE_EQ(a.find_last(1).index(), 4);
        REQUIRE_EQ(a.find_last(4).index(), 5);
        REQUIRE_EQ(a.find_last(5).index(), 3);
    }

    // [test in find] find if

    // [test in find] contains

    SUBCASE("sort")
    {
        const uint64_t kArraySize = clamp_capacity(114514);

        TestArray a(kArraySize);
        for (int i = 0; i < kArraySize; ++i)
        {
            a[i] = kArraySize - 1 - i;
        }

        a.sort();
        for (int i = 0; i < kArraySize; ++i)
        {
            REQUIRE_EQ(a[i], i);
        }

        a.sort_stable(Greater<uint32_t>());
        for (int i = 0; i < kArraySize; ++i)
        {
            REQUIRE_EQ(a[i], kArraySize - 1 - i);
        }
    }

    SUBCASE("heap")
    {
        TestArray a({ 1, 1, 4, 5, 1, 4, 1 });

        REQUIRE_FALSE(a.is_heap());

        a.heapify();
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);

        a.heap_pop();
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);
        REQUIRE_EQ(a.size(), 6);

        REQUIRE_EQ(a.heap_pop_get(), 1);
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);
        REQUIRE_EQ(a.size(), 5);

        a.heap_push(100);
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);
        REQUIRE_EQ(a.size(), 6);

        auto ref = a.find(1);
        a.heap_remove_at(ref.index());
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);
        REQUIRE_EQ(a.size(), 5);

        a.heap_sort();
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 4);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 100);
    }

    // [test in above code] stack

    // [needn't test] support foreach

    // test foreach
    SUBCASE("foreach")
    {
        TestArray a;
        for (auto n : a)
        {
            printf("%d\n", n);
        }
    }

    // test cursor & iterator
    SUBCASE("Cursor & iterator")
    {
        TestArray  a;
        const auto kArraySize = clamp_capacity(100);
        a.reserve(kArraySize);
        for (size_t i = 0; i < kArraySize; ++i)
        {
            a.add(i);
        }

        uint64_t count;

        // iter
        count = 0;
        for (auto it = a.iter(); it.has_next(); it.move_next())
        {
            REQUIRE_EQ(it.ref(), count);
            ++count;
        }
        count = 0;
        for (auto it = a.readonly().iter(); it.has_next(); it.move_next())
        {
            REQUIRE_EQ(it.ref(), count);
            ++count;
        }
        count = 0;
        for (auto it = a.iter_inv(); it.has_next(); it.move_next())
        {
            REQUIRE_EQ(it.ref(), kArraySize - 1 - count);
            ++count;
        }
        count = 0;
        for (auto it = a.readonly().iter_inv(); it.has_next(); it.move_next())
        {
            REQUIRE_EQ(it.ref(), kArraySize - 1 - count);
            ++count;
        }

        // iter as range
        count = 0;
        for (auto v : a.iter().as_range())
        {
            REQUIRE_EQ(v, count);
            ++count;
        }
        count = 0;
        for (auto v : a.readonly().iter().as_range())
        {
            REQUIRE_EQ(v, count);
            ++count;
        }
        count = 0;
        for (auto v : a.iter_inv().as_range())
        {
            REQUIRE_EQ(v, kArraySize - 1 - count);
            ++count;
        }
        count = 0;
        for (auto v : a.readonly().iter_inv().as_range())
        {
            REQUIRE_EQ(v, kArraySize - 1 - count);
            ++count;
        }

        // cursor
        count = 0;
        for (auto it = a.cursor_begin(); !it.reach_end(); it.move_next())
        {
            REQUIRE_EQ(it.ref(), count);
            ++count;
        }
        count = 0;
        for (auto it = a.readonly().cursor_begin(); !it.reach_end(); it.move_next())
        {
            REQUIRE_EQ(it.ref(), count);
            ++count;
        }
        count = 0;
        for (auto it = a.cursor_end(); !it.reach_begin(); it.move_prev())
        {
            REQUIRE_EQ(it.ref(), kArraySize - 1 - count);
            ++count;
        }
        count = 0;
        for (auto it = a.readonly().cursor_end(); !it.reach_begin(); it.move_prev())
        {
            REQUIRE_EQ(it.ref(), kArraySize - 1 - count);
            ++count;
        }
    }
}

TEST_CASE("test array")
{
    using namespace skr;
    using TestArray = Array<uint32_t>;

    template_test_array<TestArray>(
    [](auto capacity) { return capacity; },
    [](auto capacity) { return capacity; },
    [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
    [](auto&& vec) { REQUIRE_EQ(vec.data(), nullptr); },
    [](auto&& vec, auto&& v) { REQUIRE_EQ(vec.data(), v); });
}

TEST_CASE("test fixed array")
{
    using namespace skr;
    using namespace skr::container;
    static constexpr uint64_t kFixedCapacity = 200;

    using TestArray = FixedArray<uint32_t, kFixedCapacity>;

    template_test_array<TestArray>(
    [](auto capacity) { return kFixedCapacity; },
    [](auto capacity) { return capacity < kFixedCapacity ? capacity : kFixedCapacity; },
    [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
    [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
    [](auto&& vec, auto&& v) { REQUIRE_NE(vec.data(), nullptr); });
}

TEST_CASE("test inline array")
{
    using namespace skr;
    static constexpr uint64_t kInlineCapacity = 10;

    using TestArray = InlineArray<uint32_t, kInlineCapacity>;

    template_test_array<TestArray>(
    [](auto capacity) { return capacity < kInlineCapacity ? kInlineCapacity : capacity; },
    [](auto capacity) { return capacity; },
    [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
    [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
    [](auto&& vec, auto&& v) { REQUIRE_NE(vec.data(), nullptr); });
}