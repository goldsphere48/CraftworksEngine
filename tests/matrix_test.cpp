#include <gtest/gtest.h>

#include <math/matrix.h>

using namespace cw;
using namespace cw::math;
using namespace cw::math::mat;

namespace
{
    template<usize R, usize C>
    void ExpectRowMajor(const Matrix<float, R, C>& matrix, const float (&expected)[R * C])
    {
        for (usize row = 0; row < R; ++row)
        {
            for (usize column = 0; column < C; ++column)
            {
                EXPECT_FLOAT_EQ(expected[row * C + column], At(matrix, row, column))
                    << "row " << row << " column " << column;
            }
        }
    }

    constexpr Matrix<float, 2, 3> MakeA23()
    {
        return Make<float, 2, 3>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
    }

    constexpr Matrix<float, 3, 2> MakeB32()
    {
        return Make<float, 3, 2>(7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f);
    }

    constexpr Matrix<float, 3, 3> MakeTridiagonal()
    {
        return Make<float, 3, 3>(2.0f, -1.0f, 0.0f, -1.0f, 2.0f, -1.0f, 0.0f, -1.0f, 2.0f);
    }
}

TEST(MatrixStorageTest, MakeTakesRowMajorArguments)
{
    auto m = MakeA23();

    EXPECT_FLOAT_EQ(1.0f, At(m, 0, 0));
    EXPECT_FLOAT_EQ(2.0f, At(m, 0, 1));
    EXPECT_FLOAT_EQ(3.0f, At(m, 0, 2));
    EXPECT_FLOAT_EQ(4.0f, At(m, 1, 0));
    EXPECT_FLOAT_EQ(5.0f, At(m, 1, 1));
    EXPECT_FLOAT_EQ(6.0f, At(m, 1, 2));
}

TEST(MatrixStorageTest, DataIsColumnMajor)
{
    auto m = MakeA23();

    EXPECT_FLOAT_EQ(1.0f, m.Data[0]);
    EXPECT_FLOAT_EQ(4.0f, m.Data[1]);
    EXPECT_FLOAT_EQ(2.0f, m.Data[2]);
    EXPECT_FLOAT_EQ(5.0f, m.Data[3]);
    EXPECT_FLOAT_EQ(3.0f, m.Data[4]);
    EXPECT_FLOAT_EQ(6.0f, m.Data[5]);
}

TEST(MatrixStorageTest, MutableAtWritesThrough)
{
    auto m = MakeA23();

    At(m, 1, 2) = 42.0f;

    EXPECT_FLOAT_EQ(42.0f, At(m, 1, 2));
    EXPECT_FLOAT_EQ(42.0f, m.Data[5]);
}

TEST(MatrixFactoryTest, ZeroHasEveryElementZero)
{
    auto m = Zero<float, 2, 3>;

    for (usize i = 0; i < 6; ++i)
    {
        EXPECT_FLOAT_EQ(0.0f, m.Data[i]);
    }
}

TEST(MatrixFactoryTest, IdentityHasOnesOnDiagonal)
{
    auto m = Identity<float, 3>;

    for (usize row = 0; row < 3; ++row)
    {
        for (usize column = 0; column < 3; ++column)
        {
            EXPECT_FLOAT_EQ(row == column ? 1.0f : 0.0f, At(m, row, column));
        }
    }
}

TEST(MatrixFactoryTest, FromRowsStacksRows)
{
    auto m = FromRows<float, 3>(
        vec::Make<float>(1.0f, 2.0f, 3.0f), vec::Make<float>(4.0f, 5.0f, 6.0f)
    );

    EXPECT_TRUE((std::is_same_v<decltype(m), Matrix<float, 2, 3>>));
    EXPECT_TRUE(NearlyEqual(m, MakeA23()));
}

TEST(MatrixFactoryTest, FromColumnsStacksColumns)
{
    auto m = FromColumns<float, 2>(
        vec::Make<float>(1.0f, 4.0f), vec::Make<float>(2.0f, 5.0f), vec::Make<float>(3.0f, 6.0f)
    );

    EXPECT_TRUE((std::is_same_v<decltype(m), Matrix<float, 2, 3>>));
    EXPECT_TRUE(NearlyEqual(m, MakeA23()));
}

TEST(MatrixShorthandTest, MatchTheirTemplateCounterparts)
{
    EXPECT_TRUE(NearlyEqual(mat2::Identity, Identity<float, 2>));
    EXPECT_TRUE(NearlyEqual(mat3::Identity, Identity<float, 3>));
    EXPECT_TRUE(NearlyEqual(mat4::Identity, Identity<float, 4>));

    EXPECT_TRUE(NearlyEqual(mat2::Zero, Zero<float, 2, 2>));
    EXPECT_TRUE(NearlyEqual(mat3::Zero, Zero<float, 3, 3>));
    EXPECT_TRUE(NearlyEqual(mat4::Zero, Zero<float, 4, 4>));
}

TEST(MatrixShorthandTest, HaveExpectedShapeAndContent)
{
    EXPECT_TRUE((std::is_same_v<decltype(mat3::Identity), const Mat3>));
    EXPECT_TRUE((std::is_same_v<decltype(mat4::Zero), const Mat4>));

    EXPECT_FLOAT_EQ(1.0f, At(mat4::Identity, 3, 3));
    EXPECT_FLOAT_EQ(0.0f, At(mat4::Identity, 3, 2));
    EXPECT_FLOAT_EQ(0.0f, At(mat4::Zero, 3, 3));
}

TEST(MatrixAccessTest, GetRowReadsWholeRow)
{
    auto row = GetRow(MakeA23(), 1);

    EXPECT_TRUE((std::is_same_v<decltype(row), Vector<float, 3>>));
    EXPECT_FLOAT_EQ(4.0f, row.Data[0]);
    EXPECT_FLOAT_EQ(5.0f, row.Data[1]);
    EXPECT_FLOAT_EQ(6.0f, row.Data[2]);
}

TEST(MatrixAccessTest, GetColumnReadsWholeColumn)
{
    auto column = GetColumn(MakeA23(), 2);

    EXPECT_TRUE((std::is_same_v<decltype(column), Vector<float, 2>>));
    EXPECT_FLOAT_EQ(3.0f, column.Data[0]);
    EXPECT_FLOAT_EQ(6.0f, column.Data[1]);
}

TEST(MatrixAccessTest, SetRowWritesEveryColumnOfNonSquareMatrix)
{
    Matrix<float, 2, 3> m{};

    SetRow(m, 1, vec::Make<float>(4.0f, 5.0f, 6.0f));

    const float expected[]{0.0f, 0.0f, 0.0f, 4.0f, 5.0f, 6.0f};
    ExpectRowMajor(m, expected);
}

TEST(MatrixAccessTest, SetColumnWritesEveryRowOfNonSquareMatrix)
{
    Matrix<float, 3, 2> m{};

    SetColumn(m, 1, vec::Make<float>(7.0f, 8.0f, 9.0f));

    const float expected[]{0.0f, 7.0f, 0.0f, 8.0f, 0.0f, 9.0f};
    ExpectRowMajor(m, expected);
}

TEST(MatrixAccessTest, SettersRoundTripThroughGetters)
{
    Matrix<float, 2, 3> m{};

    SetRow(m, 0, vec::Make<float>(1.0f, 2.0f, 3.0f));
    SetRow(m, 1, vec::Make<float>(4.0f, 5.0f, 6.0f));

    EXPECT_TRUE(NearlyEqual(GetRow(m, 0), vec::Make<float>(1.0f, 2.0f, 3.0f)));
    EXPECT_TRUE(NearlyEqual(GetColumn(m, 1), vec::Make<float>(2.0f, 5.0f)));
}

TEST(MatrixArithmeticTest, AddsElementWise)
{
    auto m = MakeA23() + MakeA23();

    const float expected[]{2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f};
    ExpectRowMajor(m, expected);
}

TEST(MatrixArithmeticTest, SubtractsElementWise)
{
    auto m = MakeA23() - MakeA23();

    EXPECT_TRUE(NearlyEqual(m, Zero<float, 2, 3>));
}

TEST(MatrixArithmeticTest, ScalesByScalarFromEitherSide)
{
    const float expected[]{2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f};

    ExpectRowMajor(MakeA23() * 2.0f, expected);
    ExpectRowMajor(2.0f * MakeA23(), expected);
}

TEST(MatrixArithmeticTest, DividesByScalar)
{
    auto m = MakeA23() / 2.0f;

    const float expected[]{0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f};
    ExpectRowMajor(m, expected);
}

TEST(MatrixArithmeticTest, AcceptsSmallNonZeroDivisor)
{
    auto m = Make<float, 2, 2>(1.0f, 2.0f, 3.0f, 4.0f);

    m /= 1e-8f;

    EXPECT_FLOAT_EQ(1e8f, At(m, 0, 0));
    EXPECT_FLOAT_EQ(4e8f, At(m, 1, 1));
}

TEST(MatrixArithmeticTest, BinaryOperatorsLeaveOperandsUntouched)
{
    auto a = MakeA23();
    auto b = MakeA23();

    auto c = a + b;

    EXPECT_FLOAT_EQ(1.0f, At(a, 0, 0));
    EXPECT_FLOAT_EQ(1.0f, At(b, 0, 0));
    EXPECT_FLOAT_EQ(2.0f, At(c, 0, 0));
}

TEST(MatrixArithmeticTest, CompoundAssignmentMutatesInPlace)
{
    auto m = MakeA23();

    m += MakeA23();
    EXPECT_FLOAT_EQ(2.0f, At(m, 0, 0));

    m -= MakeA23();
    EXPECT_FLOAT_EQ(1.0f, At(m, 0, 0));

    m *= 4.0f;
    EXPECT_FLOAT_EQ(4.0f, At(m, 0, 0));

    m /= 4.0f;
    EXPECT_FLOAT_EQ(1.0f, At(m, 0, 0));
}

TEST(MatrixProductTest, MultipliesNonSquareShapes)
{
    auto m = MakeA23() * MakeB32();

    EXPECT_TRUE((std::is_same_v<decltype(m), Matrix<float, 2, 2>>));

    const float expected[]{58.0f, 64.0f, 139.0f, 154.0f};
    ExpectRowMajor(m, expected);
}

TEST(MatrixProductTest, MultipliesInTheOtherOrder)
{
    auto m = MakeB32() * MakeA23();

    EXPECT_TRUE((std::is_same_v<decltype(m), Matrix<float, 3, 3>>));

    const float expected[]{39.0f, 54.0f, 69.0f, 49.0f, 68.0f, 87.0f, 59.0f, 82.0f, 105.0f};
    ExpectRowMajor(m, expected);
}

TEST(MatrixProductTest, IdentityIsNeutralOnBothSides)
{
    auto m = MakeTridiagonal();

    EXPECT_TRUE(NearlyEqual(m * Identity<float, 3>, m));
    EXPECT_TRUE(NearlyEqual(Identity<float, 3> * m, m));
}

TEST(MatrixProductTest, IsAssociative)
{
    auto a = MakeA23();
    auto b = MakeB32();

    EXPECT_TRUE(NearlyEqual((a * b) * a, a * (b * a), 1e-4f));
}

TEST(MatrixProductTest, TransformsVector)
{
    auto v = MakeA23() * vec::Make<float>(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE((std::is_same_v<decltype(v), Vector<float, 2>>));
    EXPECT_FLOAT_EQ(14.0f, v.Data[0]);
    EXPECT_FLOAT_EQ(32.0f, v.Data[1]);
}

TEST(MatrixProductTest, VectorTransformSelectsColumnsOfIdentity)
{
    auto v = Identity<float, 3> * vec::Make<float>(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(NearlyEqual(v, vec::Make<float>(1.0f, 2.0f, 3.0f)));
}

TEST(MatrixTransposeTest, SwapsShapeAndElements)
{
    auto m = Transpose(MakeA23());

    EXPECT_TRUE((std::is_same_v<decltype(m), Matrix<float, 3, 2>>));

    const float expected[]{1.0f, 4.0f, 2.0f, 5.0f, 3.0f, 6.0f};
    ExpectRowMajor(m, expected);
}

TEST(MatrixTransposeTest, AppliedTwiceReturnsOriginal)
{
    EXPECT_TRUE(NearlyEqual(Transpose(Transpose(MakeA23())), MakeA23()));
}

TEST(MatrixTransposeTest, LeavesSymmetricMatrixUnchanged)
{
    EXPECT_TRUE(NearlyEqual(Transpose(MakeTridiagonal()), MakeTridiagonal()));
}

TEST(MatrixTransposeTest, ReversesProductOrder)
{
    auto a = MakeA23();
    auto b = MakeB32();

    EXPECT_TRUE(NearlyEqual(Transpose(a * b), Transpose(b) * Transpose(a), 1e-4f));
}

TEST(MatrixDeterminantTest, HandlesSmallFixedSizes)
{
    EXPECT_FLOAT_EQ(3.0f, Determinant(Make<float, 1, 1>(3.0f)));
    EXPECT_FLOAT_EQ(-2.0f, Determinant(Make<float, 2, 2>(1.0f, 2.0f, 3.0f, 4.0f)));
    EXPECT_FLOAT_EQ(4.0f, Determinant(MakeTridiagonal()));
}

TEST(MatrixDeterminantTest, HandlesFourByFour)
{
    auto m = Make<float, 4, 4>(
        1.0f, 0.0f, 2.0f, -1.0f,
        3.0f, 0.0f, 0.0f, 5.0f,
        2.0f, 1.0f, 4.0f, -3.0f,
        1.0f, 0.0f, 5.0f, 0.0f
    );

    EXPECT_FLOAT_EQ(30.0f, Determinant(m));
}

TEST(MatrixDeterminantTest, FloatingDeterminantKeepsElementType)
{
    EXPECT_TRUE((std::is_same_v<decltype(Determinant(Identity<float, 3>)), float>));
    EXPECT_TRUE((std::is_same_v<decltype(Determinant(Identity<double, 3>)), double>));
    EXPECT_TRUE((std::is_same_v<decltype(Determinant(Identity<float, 5>)), float>));
}

TEST(MatrixDeterminantTest, IntegerDeterminantWidensToInt64)
{
    auto m = Make<int32, 2, 2>(1, 2, 3, 4);

    EXPECT_TRUE((std::is_same_v<decltype(Determinant(m)), int64>));
    EXPECT_EQ(-2, Determinant(m));
}

TEST(MatrixDeterminantTest, UnsignedDeterminantDoesNotWrapAround)
{
    EXPECT_TRUE((std::is_same_v<decltype(Determinant(Make<uint32, 2, 2>(1u, 2u, 3u, 4u))), int64>));

    EXPECT_EQ(-2, Determinant(Make<uint32, 2, 2>(1u, 2u, 3u, 4u)));
    EXPECT_EQ(-3, Determinant(Make<uint32, 3, 3>(1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 10u)));
    EXPECT_EQ(-2, Determinant(Make<uint32, 4, 4>(
        1u, 0u, 0u, 0u,
        0u, 1u, 0u, 0u,
        0u, 0u, 0u, 1u,
        0u, 0u, 2u, 0u
    )));
}

TEST(MatrixDeterminantTest, IntegerDeterminantDoesNotOverflowInIntermediateProducts)
{
    auto m = Make<int32, 2, 2>(100000, 0, 0, 100000);

    EXPECT_EQ(10000000000LL, Determinant(m));
}

TEST(MatrixDeterminantTest, IdentityHasDeterminantOne)
{
    EXPECT_FLOAT_EQ(1.0f, Determinant(Identity<float, 2>));
    EXPECT_FLOAT_EQ(1.0f, Determinant(Identity<float, 3>));
    EXPECT_FLOAT_EQ(1.0f, Determinant(Identity<float, 4>));
    EXPECT_FLOAT_EQ(1.0f, Determinant(Identity<float, 5>));
}

TEST(MatrixDeterminantTest, SingularMatrixHasZeroDeterminant)
{
    EXPECT_FLOAT_EQ(0.0f, Determinant(Make<float, 2, 2>(1.0f, 2.0f, 2.0f, 4.0f)));
    EXPECT_FLOAT_EQ(0.0f, Determinant(Zero<float, 3, 3>));
}

TEST(MatrixDeterminantTest, GeneralPathHandlesTriangularMatrix)
{
    auto m = Make<float, 5, 5>(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
        0.0f, 2.0f, 3.0f, 4.0f, 5.0f,
        0.0f, 0.0f, 3.0f, 4.0f, 5.0f,
        0.0f, 0.0f, 0.0f, 4.0f, 5.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 5.0f
    );

    EXPECT_FLOAT_EQ(120.0f, Determinant(m));
}

TEST(MatrixDeterminantTest, GeneralPathNegatesOnRowSwap)
{
    auto m = Make<float, 5, 5>(
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    );

    EXPECT_FLOAT_EQ(-1.0f, Determinant(m));
}

TEST(MatrixDeterminantTest, GeneralPathKeepsVerySmallDeterminants)
{
    auto m = Make<float, 5, 5>(
        1e-4f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1e-4f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1e-4f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1e-4f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1e-4f
    );

    EXPECT_TRUE(NearlyEqual(Determinant(m), 1e-20f, 1e-5f));
}

TEST(MatrixDeterminantTest, GeneralPathDetectsSingularMatrix)
{
    Matrix<float, 5, 5> m = Identity<float, 5>;

    SetRow(m, 3, vec::Fill<float, 5>(0.0f));

    EXPECT_FLOAT_EQ(0.0f, Determinant(m));
}

TEST(MatrixDeterminantTest, MatchesProductOfDeterminants)
{
    auto a = MakeTridiagonal();
    auto b = Make<float, 3, 3>(1.0f, 2.0f, 0.0f, 0.0f, 1.0f, 3.0f, 4.0f, 0.0f, 1.0f);

    EXPECT_NEAR(Determinant(a) * Determinant(b), Determinant(a * b), 1e-3f);
}

TEST(MatrixCastTest, ConvertsElementType)
{
    auto m = Cast<double>(Make<int, 2, 2>(1, 2, 3, 4));

    EXPECT_TRUE((std::is_same_v<decltype(m), Matrix<double, 2, 2>>));
    EXPECT_DOUBLE_EQ(1.0, At(m, 0, 0));
    EXPECT_DOUBLE_EQ(4.0, At(m, 1, 1));
}

TEST(MatrixInverseTest, InvertsTwoByTwo)
{
    Matrix<float, 2, 2> inverse{};

    EXPECT_TRUE(TryInverse(Make<float, 2, 2>(4.0f, 7.0f, 2.0f, 6.0f), inverse));

    const float expected[]{0.6f, -0.7f, -0.2f, 0.4f};
    ExpectRowMajor(inverse, expected);
}

TEST(MatrixInverseTest, InvertsThreeByThree)
{
    Matrix<float, 3, 3> inverse{};

    EXPECT_TRUE(TryInverse(MakeTridiagonal(), inverse));

    auto expected = Make<float, 3, 3>(0.75f, 0.5f, 0.25f, 0.5f, 1.0f, 0.5f, 0.25f, 0.5f, 0.75f);

    EXPECT_TRUE(NearlyEqual(inverse, expected, 1e-6f));
}

TEST(MatrixInverseTest, ProductWithInverseIsIdentity)
{
    auto m = Make<float, 4, 4>(
        1.0f, 0.0f, 2.0f, -1.0f,
        3.0f, 0.0f, 0.0f, 5.0f,
        2.0f, 1.0f, 4.0f, -3.0f,
        1.0f, 0.0f, 5.0f, 0.0f
    );

    Matrix<float, 4, 4> inverse{};

    ASSERT_TRUE(TryInverse(m, inverse));

    EXPECT_TRUE(NearlyEqual(m * inverse, Identity<float, 4>, 1e-5f));
    EXPECT_TRUE(NearlyEqual(inverse * m, Identity<float, 4>, 1e-5f));
}

TEST(MatrixInverseTest, PivotsWhenLeadingElementIsZero)
{
    auto m = Make<float, 3, 3>(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f);

    Matrix<float, 3, 3> inverse{};

    ASSERT_TRUE(TryInverse(m, inverse));

    const float expected[]{0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f};
    ExpectRowMajor(inverse, expected);
}

TEST(MatrixInverseTest, WidensIntegerInputToRequestedFloatingType)
{
    Matrix<double, 3, 3> inverse{};

    ASSERT_TRUE(TryInverse(Make<int, 3, 3>(2, -1, 0, -1, 2, -1, 0, -1, 2), inverse));

    EXPECT_NEAR(0.75, At(inverse, 0, 0), 1e-12);
    EXPECT_NEAR(0.5, At(inverse, 0, 1), 1e-12);
    EXPECT_NEAR(0.25, At(inverse, 0, 2), 1e-12);
}

TEST(MatrixInverseTest, IntegerTwoByTwoDoesNotTruncate)
{
    Matrix<double, 2, 2> inverse{};

    ASSERT_TRUE(TryInverse(Make<int, 2, 2>(4, 7, 2, 6), inverse));

    EXPECT_DOUBLE_EQ(0.6, At(inverse, 0, 0));
    EXPECT_DOUBLE_EQ(-0.7, At(inverse, 0, 1));
    EXPECT_DOUBLE_EQ(-0.2, At(inverse, 1, 0));
    EXPECT_DOUBLE_EQ(0.4, At(inverse, 1, 1));
}

TEST(MatrixInverseTest, InvertsTwoByTwoWithVerySmallElements)
{
    Matrix<float, 2, 2> inverse{};

    ASSERT_TRUE(TryInverse(Make<float, 2, 2>(1e-4f, 0.0f, 0.0f, 1e-4f), inverse));

    EXPECT_TRUE(NearlyEqual(inverse, Make<float, 2, 2>(1e4f, 0.0f, 0.0f, 1e4f), 1e-5f));
}

TEST(MatrixInverseTest, InvertsThreeByThreeWithVerySmallElements)
{
    auto m = Make<float, 3, 3>(1e-8f, 0.0f, 0.0f, 0.0f, 1e-8f, 0.0f, 0.0f, 0.0f, 1e-8f);

    Matrix<float, 3, 3> inverse{};

    ASSERT_TRUE(TryInverse(m, inverse));

    EXPECT_TRUE(NearlyEqual(inverse, Identity<float, 3> * 1e8f, 1e-5f));
    EXPECT_TRUE(NearlyEqual(m * inverse, Identity<float, 3>, 1e-5f));
}

TEST(MatrixInverseTest, ScalingAMatrixDoesNotChangeInvertibility)
{
    Matrix<float, 3, 3> inverse{};

    for (float scale = 1.0f; scale > 1e-9f; scale *= 0.1f)
    {
        EXPECT_TRUE(TryInverse(MakeTridiagonal() * scale, inverse)) << "scale " << scale;
    }
}

TEST(MatrixInverseTest, ReportsFailureForSingularMatrix)
{
    Matrix<float, 2, 2> inverse2{};
    Matrix<float, 3, 3> inverse3{};

    EXPECT_FALSE(TryInverse(Make<float, 2, 2>(1.0f, 2.0f, 2.0f, 4.0f), inverse2));
    EXPECT_FALSE(TryInverse(Zero<float, 3, 3>, inverse3));
}

TEST(MatrixInverseTest, LeavesOutputUntouchedOnFailure)
{
    auto output = Make<float, 3, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );

    const auto original = output;

    EXPECT_FALSE(TryInverse(Zero<float, 3, 3>, output));
    EXPECT_TRUE(NearlyEqual(output, original));
}

TEST(MatrixInverseTest, LeavesOutputUntouchedWhenSingularityIsFoundAtLastPivot)
{
    auto output = Make<float, 3, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );

    const auto original = output;
    const auto singular = Make<float, 3, 3>(2.0f, 1.0f, 0.0f, 1.0f, 2.0f, 0.0f, 3.0f, 3.0f, 0.0f);

    EXPECT_FALSE(TryInverse(singular, output));
    EXPECT_TRUE(NearlyEqual(output, original));
}

TEST(MatrixInverseTest, LeavesOutputUntouchedOnSingularTwoByTwo)
{
    auto output = Make<float, 2, 2>(1.0f, 2.0f, 3.0f, 4.0f);

    const auto original = output;

    EXPECT_FALSE(TryInverse(Make<float, 2, 2>(1.0f, 2.0f, 2.0f, 4.0f), output));
    EXPECT_TRUE(NearlyEqual(output, original));
}

TEST(MatrixInverseTest, IdentityIsItsOwnInverse)
{
    EXPECT_TRUE(NearlyEqual(Inverse(Identity<float, 4>), Identity<float, 4>));
}

TEST(MatrixInverseTest, ShorthandReturnsSameResultAsTryInverse)
{
    Matrix<float, 3, 3> expected{};

    ASSERT_TRUE(TryInverse(MakeTridiagonal(), expected));

    EXPECT_TRUE(NearlyEqual(Inverse(MakeTridiagonal()), expected));
}

TEST(MatrixNearlyEqualTest, AcceptsIdenticalMatrices)
{
    EXPECT_TRUE(NearlyEqual(MakeA23(), MakeA23()));
}

TEST(MatrixNearlyEqualTest, RejectsDifferenceBeyondTolerance)
{
    auto m = MakeA23();

    At(m, 1, 2) += 0.5f;

    EXPECT_FALSE(NearlyEqual(m, MakeA23()));
    EXPECT_FALSE(NearlyEqual(m, MakeA23(), 0.01f));
}

TEST(MatrixNearlyEqualTest, ToleranceIsRelativeToElementMagnitude)
{
    auto small = Make<float, 1, 1>(1.0f);
    auto large = Make<float, 1, 1>(100.0f);

    EXPECT_FALSE(NearlyEqual(small, Make<float, 1, 1>(1.5f), 0.1f));
    EXPECT_TRUE(NearlyEqual(large, Make<float, 1, 1>(100.5f), 0.1f));
}

TEST(MatrixNearlyEqualTest, AcceptsDifferenceWithinTolerance)
{
    auto m = MakeA23();

    At(m, 1, 2) += 0.05f;

    EXPECT_TRUE(NearlyEqual(m, MakeA23(), 0.1f));
}

TEST(MatrixNearlyEqualTest, ScalesToleranceWithMagnitude)
{
    auto a = Make<float, 1, 1>(1000000.0f);
    auto b = Make<float, 1, 1>(1000000.06f);

    EXPECT_TRUE(NearlyEqual(a, b));
    EXPECT_FALSE(NearlyEqual(Make<float, 1, 1>(0.0f), Make<float, 1, 1>(0.001f)));
}

TEST(MatrixNearlyEqualTest, ComparesIntegerMatricesExactly)
{
    EXPECT_TRUE(NearlyEqual(Make<int, 2, 2>(1, 2, 3, 4), Make<int, 2, 2>(1, 2, 3, 4)));
    EXPECT_FALSE(NearlyEqual(Make<int, 2, 2>(1, 2, 3, 4), Make<int, 2, 2>(1, 2, 3, 5)));
}

TEST(MathUtilsTest, NearlyEqualUsesAbsoluteAndRelativeTolerance)
{
    EXPECT_TRUE(NearlyEqual(1.0f, 1.0f));
    EXPECT_TRUE(NearlyEqual(1.0f, 1.05f, 0.1f));
    EXPECT_FALSE(NearlyEqual(1.0f, 1.5f, 0.1f));
    EXPECT_TRUE(NearlyEqual(1000000.0f, 1000000.06f));
    EXPECT_FALSE(NearlyEqual(0.0f, 0.001f));
}

TEST(MathUtilsTest, NearlyZeroMatchesOnlySmallValues)
{
    EXPECT_TRUE(NearlyZero(0.0));
    EXPECT_TRUE(NearlyZero(0.001, 0.01));
    EXPECT_FALSE(NearlyZero(0.001));
    EXPECT_TRUE(NearlyZero(0));
    EXPECT_FALSE(NearlyZero(1));
}

TEST(MathUtilsTest, AbsRemovesSign)
{
    EXPECT_FLOAT_EQ(2.5f, Abs(-2.5f));
    EXPECT_FLOAT_EQ(2.5f, Abs(2.5f));
    EXPECT_EQ(7, Abs(-7));
}

namespace
{
    constexpr auto g_A       = Make<float, 2, 3>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
    constexpr auto g_B       = Make<float, 3, 2>(7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f);
    constexpr auto g_Product = g_A * g_B;
    constexpr auto g_Vector  = g_A * vec::Make<float>(1.0f, 2.0f, 3.0f);

    static_assert(At(g_A, 1, 2) == 6.0f);
    static_assert(g_A.Data[1] == 4.0f);

    static_assert(NearlyEqual(g_Product, Make<float, 2, 2>(58.0f, 64.0f, 139.0f, 154.0f)));
    static_assert(NearlyEqual(g_Vector, vec::Make<float>(14.0f, 32.0f)));

    static_assert(NearlyEqual(Transpose(g_A), Make<float, 3, 2>(1.0f, 4.0f, 2.0f, 5.0f, 3.0f, 6.0f)));
    static_assert(NearlyEqual(Transpose(Transpose(g_A)), g_A));

    static_assert(NearlyEqual(g_A + g_A, g_A * 2.0f));
    static_assert(NearlyEqual(g_A - g_A, Zero<float, 2, 3>));

    static_assert(At(Identity<float, 3>, 1, 1) == 1.0f);
    static_assert(At(Identity<float, 3>, 1, 2) == 0.0f);

    static_assert(Determinant(Make<float, 2, 2>(1.0f, 2.0f, 3.0f, 4.0f)) == -2.0f);
    static_assert(Determinant(Make<int32, 2, 2>(1, 2, 3, 4)) == -2);
    static_assert(Determinant(Make<uint32, 2, 2>(1u, 2u, 3u, 4u)) == -2);
    static_assert(std::is_same_v<DeterminantResultT<float>, float>);
    static_assert(std::is_same_v<DeterminantResultT<double>, double>);
    static_assert(std::is_same_v<DeterminantResultT<int32>, int64>);
    static_assert(std::is_same_v<DeterminantResultT<uint32>, int64>);
    static_assert(Determinant(Make<float, 3, 3>(2.0f, -1.0f, 0.0f, -1.0f, 2.0f, -1.0f, 0.0f, -1.0f, 2.0f)) == 4.0f);

    static_assert(NearlyEqual(
        Inverse(Make<float, 2, 2>(4.0f, 7.0f, 2.0f, 6.0f)),
        Make<float, 2, 2>(0.6f, -0.7f, -0.2f, 0.4f),
        1e-6f
    ));

    static_assert(NearlyEqual(
        Make<float, 2, 2>(4.0f, 7.0f, 2.0f, 6.0f) * Inverse(Make<float, 2, 2>(4.0f, 7.0f, 2.0f, 6.0f)),
        Identity<float, 2>,
        1e-6f
    ));

    static_assert(NearlyEqual(1.0f, 1.0f));
    static_assert(!NearlyEqual(0.0f, 0.001f));
    static_assert(NearlyZero(0.0f));
    static_assert(Abs(-3) == 3);
}

TEST(MatrixConstexprTest, ExpressionsEvaluateAtCompileTime)
{
    SUCCEED();
}
