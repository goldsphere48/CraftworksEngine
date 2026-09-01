#include <gtest/gtest.h>

#include <core/darray.h>

#include <limits>

using namespace cw;

namespace
{
    struct NonTrivialElement
    {
        ~NonTrivialElement() {}
    };

    static_assert(CDArrayElement<int>);
    static_assert(!CDArrayElement<NonTrivialElement>);
}

TEST(DArrayTest, GrowsAndPreservesElements)
{
    darray<int> values;

    for (int i = 0; i < 32; ++i)
    {
        ASSERT_TRUE(values.Add(i * 3));
    }

    ASSERT_EQ(32u, values.Count());
    for (usize i = 0; i < values.Count(); ++i)
    {
        EXPECT_EQ(static_cast<int>(i) * 3, values[i]);
    }
}

TEST(DArrayTest, RejectsCapacityOverflowWithoutChangingContents)
{
    darray<uint64> values;

    ASSERT_TRUE(values.Add(42));
    EXPECT_FALSE(values.Reserve(std::numeric_limits<usize>::max()));

    ASSERT_EQ(1u, values.Count());
    EXPECT_EQ(42u, values[0]);
}
