#include <gtest/gtest.h>

#include <math/math_utils.h>
#include <math/matrix.h>
#include <math/vector.h>

#include <type_traits>

using namespace cw;
using namespace cw::math;

namespace
{
    constexpr float Tolerance = 1e-5f;

    Mat3 MakeArbitrary()
    {
        return mat::Make<float, 3, 3>(
            2.0f, -1.0f, 0.5f,
            0.0f,  3.0f, 1.0f,
            1.0f,  4.0f, -2.0f
        );
    }

    Mat3 MakeOtherArbitrary()
    {
        return mat::Make<float, 3, 3>(
            -1.0f, 2.0f, 3.0f,
             0.5f, 1.0f, 0.0f,
             2.0f, 0.0f, 1.0f
        );
    }
}

TEST(MatrixStorageTest, MakeTakesRowMajorArguments)
{
    const Matrix<float, 2, 3> matrix = mat::Make<float, 2, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    );

    EXPECT_FLOAT_EQ(1.0f, mat::At(matrix, 0, 0));
    EXPECT_FLOAT_EQ(2.0f, mat::At(matrix, 0, 1));
    EXPECT_FLOAT_EQ(3.0f, mat::At(matrix, 0, 2));
    EXPECT_FLOAT_EQ(4.0f, mat::At(matrix, 1, 0));
    EXPECT_FLOAT_EQ(5.0f, mat::At(matrix, 1, 1));
    EXPECT_FLOAT_EQ(6.0f, mat::At(matrix, 1, 2));
}

TEST(MatrixStorageTest, DataIsColumnMajor)
{
    const Matrix<float, 2, 3> matrix = mat::Make<float, 2, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    );

    EXPECT_FLOAT_EQ(1.0f, matrix.Data[0]);
    EXPECT_FLOAT_EQ(4.0f, matrix.Data[1]);
    EXPECT_FLOAT_EQ(2.0f, matrix.Data[2]);
    EXPECT_FLOAT_EQ(5.0f, matrix.Data[3]);
    EXPECT_FLOAT_EQ(3.0f, matrix.Data[4]);
    EXPECT_FLOAT_EQ(6.0f, matrix.Data[5]);
}

TEST(MatrixStorageTest, HasNoPaddingAroundElements)
{
    EXPECT_EQ(sizeof(float) * 4, sizeof(Mat2));
    EXPECT_EQ(sizeof(float) * 9, sizeof(Mat3));
    EXPECT_EQ(sizeof(float) * 16, sizeof(Mat4));
}

TEST(MatrixStorageTest, MutableAtWritesThrough)
{
    Mat3 matrix = mat3::Zero;

    mat::At(matrix, 1, 2) = 7.0f;

    EXPECT_FLOAT_EQ(7.0f, mat::At(matrix, 1, 2));
    EXPECT_FLOAT_EQ(7.0f, matrix.Data[2 * 3 + 1]);
    EXPECT_FLOAT_EQ(0.0f, mat::At(matrix, 2, 1));
}

TEST(MatrixFactoryTest, ZeroHasEveryElementZero)
{
    const Mat4 matrix = mat::Zero<float, 4, 4>;

    for (usize i = 0; i < 16; ++i)
    {
        EXPECT_FLOAT_EQ(0.0f, matrix.Data[i]);
    }
}

TEST(MatrixFactoryTest, IdentityHasOnesOnTheDiagonal)
{
    const Mat4 matrix = mat::Identity<float, 4>;

    for (usize row = 0; row < 4; ++row)
    {
        for (usize column = 0; column < 4; ++column)
        {
            const float expected = row == column ? 1.0f : 0.0f;

            EXPECT_FLOAT_EQ(expected, mat::At(matrix, row, column));
        }
    }
}

TEST(MatrixFactoryTest, FromRowsStacksRows)
{
    const auto matrix = mat::FromRows<float, 3>(
        Vec3{1.0f, 2.0f, 3.0f},
        Vec3{4.0f, 5.0f, 6.0f}
    );

    EXPECT_TRUE((std::is_same_v<decltype(matrix), const Matrix<float, 2, 3>>));

    EXPECT_FLOAT_EQ(1.0f, mat::At(matrix, 0, 0));
    EXPECT_FLOAT_EQ(3.0f, mat::At(matrix, 0, 2));
    EXPECT_FLOAT_EQ(4.0f, mat::At(matrix, 1, 0));
    EXPECT_FLOAT_EQ(6.0f, mat::At(matrix, 1, 2));
}

TEST(MatrixFactoryTest, FromColumnsStacksColumns)
{
    const auto matrix = mat::FromColumns<float, 3>(
        Vec3{1.0f, 2.0f, 3.0f},
        Vec3{4.0f, 5.0f, 6.0f}
    );

    EXPECT_TRUE((std::is_same_v<decltype(matrix), const Matrix<float, 3, 2>>));

    EXPECT_FLOAT_EQ(1.0f, mat::At(matrix, 0, 0));
    EXPECT_FLOAT_EQ(3.0f, mat::At(matrix, 2, 0));
    EXPECT_FLOAT_EQ(4.0f, mat::At(matrix, 0, 1));
    EXPECT_FLOAT_EQ(6.0f, mat::At(matrix, 2, 1));
}

TEST(MatrixFactoryTest, FromRowsAndFromColumnsAreTransposes)
{
    const Vec3 first{1.0f, 2.0f, 3.0f};
    const Vec3 second{4.0f, 5.0f, 6.0f};

    const auto rows = mat::FromRows<float, 3>(first, second);
    const auto columns = mat::FromColumns<float, 3>(first, second);

    EXPECT_TRUE(NearlyEqual(rows, mat::Transpose(columns)));
}

TEST(MatrixShorthandTest, MatchTheirTemplateCounterparts)
{
    EXPECT_TRUE((std::is_same_v<Mat2, Matrix<float, 2, 2>>));
    EXPECT_TRUE((std::is_same_v<Mat3, Matrix<float, 3, 3>>));
    EXPECT_TRUE((std::is_same_v<Mat4, Matrix<float, 4, 4>>));

    EXPECT_TRUE(NearlyEqual(mat2::Identity, (mat::Identity<float, 2>)));
    EXPECT_TRUE(NearlyEqual(mat3::Identity, (mat::Identity<float, 3>)));
    EXPECT_TRUE(NearlyEqual(mat4::Identity, (mat::Identity<float, 4>)));

    EXPECT_TRUE(NearlyEqual(mat2::Zero, (mat::Zero<float, 2, 2>)));
    EXPECT_TRUE(NearlyEqual(mat4::Zero, (mat::Zero<float, 4, 4>)));
}

TEST(MatrixAccessTest, GetRowReadsWholeRow)
{
    const Matrix<float, 2, 3> matrix = mat::Make<float, 2, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    );

    EXPECT_TRUE(NearlyEqual(Vec3{1.0f, 2.0f, 3.0f}, mat::GetRow(matrix, 0)));
    EXPECT_TRUE(NearlyEqual(Vec3{4.0f, 5.0f, 6.0f}, mat::GetRow(matrix, 1)));
}

TEST(MatrixAccessTest, GetColumnReadsWholeColumn)
{
    const Matrix<float, 2, 3> matrix = mat::Make<float, 2, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    );

    EXPECT_TRUE(NearlyEqual(Vec2{1.0f, 4.0f}, mat::GetColumn(matrix, 0)));
    EXPECT_TRUE(NearlyEqual(Vec2{3.0f, 6.0f}, mat::GetColumn(matrix, 2)));
}

TEST(MatrixAccessTest, SetRowWritesEveryColumn)
{
    Matrix<float, 2, 3> matrix = mat::Zero<float, 2, 3>;

    mat::SetRow(matrix, 1, Vec3{7.0f, 8.0f, 9.0f});

    EXPECT_TRUE(NearlyEqual(Vec3{7.0f, 8.0f, 9.0f}, mat::GetRow(matrix, 1)));
    EXPECT_TRUE(NearlyZero(mat::GetRow(matrix, 0)));
}

TEST(MatrixAccessTest, SetColumnWritesEveryRow)
{
    Matrix<float, 2, 3> matrix = mat::Zero<float, 2, 3>;

    mat::SetColumn(matrix, 2, Vec2{7.0f, 8.0f});

    EXPECT_TRUE(NearlyEqual(Vec2{7.0f, 8.0f}, mat::GetColumn(matrix, 2)));
    EXPECT_TRUE(NearlyZero(mat::GetColumn(matrix, 0)));
}

TEST(MatrixAccessTest, SettersRoundTripThroughGetters)
{
    const Mat3 source = MakeArbitrary();

    Mat3 result = mat3::Zero;

    for (usize row = 0; row < 3; ++row)
    {
        mat::SetRow(result, row, mat::GetRow(source, row));
    }

    EXPECT_TRUE(NearlyEqual(source, result));

    result = mat3::Zero;

    for (usize column = 0; column < 3; ++column)
    {
        mat::SetColumn(result, column, mat::GetColumn(source, column));
    }

    EXPECT_TRUE(NearlyEqual(source, result));
}

TEST(MatrixArithmeticTest, AddsAndSubtractsElementWise)
{
    const Mat2 lhs = mat::Make<float, 2, 2>(
        1.0f, 2.0f,
        3.0f, 4.0f
    );

    const Mat2 rhs = mat::Make<float, 2, 2>(
        5.0f, -6.0f,
        7.0f, 8.0f
    );

    const Mat2 expectedSum = mat::Make<float, 2, 2>(
        6.0f, -4.0f,
        10.0f, 12.0f
    );

    const Mat2 expectedDifference = mat::Make<float, 2, 2>(
        -4.0f, 8.0f,
        -4.0f, -4.0f
    );

    EXPECT_TRUE(NearlyEqual(expectedSum, lhs + rhs));
    EXPECT_TRUE(NearlyEqual(expectedDifference, lhs - rhs));
}

TEST(MatrixArithmeticTest, ScalesByScalarFromEitherSide)
{
    const Mat2 matrix = mat::Make<float, 2, 2>(
        1.0f, 2.0f,
        3.0f, 4.0f
    );

    const Mat2 expected = mat::Make<float, 2, 2>(
        2.0f, 4.0f,
        6.0f, 8.0f
    );

    EXPECT_TRUE(NearlyEqual(expected, matrix * 2.0f));
    EXPECT_TRUE(NearlyEqual(expected, 2.0f * matrix));
}

TEST(MatrixArithmeticTest, DividesByScalar)
{
    const Mat2 matrix = mat::Make<float, 2, 2>(
        2.0f, 4.0f,
        6.0f, 8.0f
    );

    const Mat2 expected = mat::Make<float, 2, 2>(
        1.0f, 2.0f,
        3.0f, 4.0f
    );

    EXPECT_TRUE(NearlyEqual(expected, matrix / 2.0f));
}

TEST(MatrixArithmeticTest, AcceptsSmallNonZeroDivisor)
{
    Mat2 matrix = mat::Make<float, 2, 2>(
        1e-4f, 2e-4f,
        3e-4f, 4e-4f
    );

    matrix /= 1e-4f;

    const Mat2 expected = mat::Make<float, 2, 2>(
        1.0f, 2.0f,
        3.0f, 4.0f
    );

    EXPECT_TRUE(NearlyEqual(expected, matrix, Tolerance));
}

TEST(MatrixArithmeticTest, BinaryOperatorsLeaveOperandsUntouched)
{
    const Mat3 lhs = MakeArbitrary();
    const Mat3 rhs = MakeOtherArbitrary();

    const Mat3 lhsCopy = lhs;
    const Mat3 rhsCopy = rhs;

    const Mat3 unused = (lhs + rhs) * 2.0f - lhs;

    EXPECT_TRUE(NearlyEqual(lhsCopy, lhs));
    EXPECT_TRUE(NearlyEqual(rhsCopy, rhs));
    EXPECT_TRUE(NearlyEqual(lhs + rhs * 2.0f, unused - lhs + rhs * 0.0f + lhs));
}

TEST(MatrixArithmeticTest, CompoundAssignmentMutatesInPlace)
{
    Mat2 matrix = mat2::Identity;

    matrix += mat2::Identity;
    EXPECT_FLOAT_EQ(2.0f, mat::At(matrix, 0, 0));

    matrix -= mat2::Identity;
    EXPECT_FLOAT_EQ(1.0f, mat::At(matrix, 0, 0));

    matrix *= 5.0f;
    EXPECT_FLOAT_EQ(5.0f, mat::At(matrix, 1, 1));

    matrix /= 5.0f;
    EXPECT_TRUE(NearlyEqual(mat2::Identity, matrix));
}

TEST(MatrixProductTest, MultipliesNonSquareShapes)
{
    const Matrix<float, 2, 3> lhs = mat::Make<float, 2, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    );

    const Matrix<float, 3, 2> rhs = mat::Make<float, 3, 2>(
        7.0f, 8.0f,
        9.0f, 10.0f,
        11.0f, 12.0f
    );

    const Matrix<float, 2, 2> expected = mat::Make<float, 2, 2>(
        58.0f, 64.0f,
        139.0f, 154.0f
    );

    EXPECT_TRUE(NearlyEqual(expected, lhs * rhs));
}

TEST(MatrixProductTest, MultipliesInTheOtherOrder)
{
    const Matrix<float, 2, 3> lhs = mat::Make<float, 2, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    );

    const Matrix<float, 3, 2> rhs = mat::Make<float, 3, 2>(
        7.0f, 8.0f,
        9.0f, 10.0f,
        11.0f, 12.0f
    );

    const Matrix<float, 3, 3> expected = mat::Make<float, 3, 3>(
        39.0f, 54.0f, 69.0f,
        49.0f, 68.0f, 87.0f,
        59.0f, 82.0f, 105.0f
    );

    EXPECT_TRUE(NearlyEqual(expected, rhs * lhs));
}

TEST(MatrixProductTest, IdentityIsNeutralOnBothSides)
{
    const Mat3 matrix = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(matrix, matrix * mat3::Identity, Tolerance));
    EXPECT_TRUE(NearlyEqual(matrix, mat3::Identity * matrix, Tolerance));
}

TEST(MatrixProductTest, IsAssociative)
{
    const Mat3 a = MakeArbitrary();
    const Mat3 b = MakeOtherArbitrary();
    const Mat3 c = mat::Make<float, 3, 3>(
        1.0f, 0.0f, -2.0f,
        3.0f, 1.0f,  0.5f,
        0.0f, 2.0f,  1.0f
    );

    EXPECT_TRUE(NearlyEqual((a * b) * c, a * (b * c), 1e-4f));
}

TEST(MatrixProductTest, IsNotCommutative)
{
    const Mat3 a = MakeArbitrary();
    const Mat3 b = MakeOtherArbitrary();

    EXPECT_FALSE(NearlyEqual(a * b, b * a, Tolerance));
}

TEST(MatrixProductTest, TransformsVector)
{
    const Matrix<float, 2, 3> matrix = mat::Make<float, 2, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    );

    const Vec3 vector{1.0f, 2.0f, 3.0f};

    EXPECT_TRUE(NearlyEqual(Vec2{14.0f, 32.0f}, matrix * vector));
}

TEST(MatrixProductTest, VectorTransformSelectsColumnsOfIdentity)
{
    const Vec3 vector{1.0f, 2.0f, 3.0f};

    EXPECT_TRUE(NearlyEqual(vector, mat3::Identity * vector));
}

TEST(MatrixProductTest, MatchesProductOfTransformedVectors)
{
    const Mat3 a = MakeArbitrary();
    const Mat3 b = MakeOtherArbitrary();
    const Vec3 vector{1.0f, -2.0f, 0.5f};

    EXPECT_TRUE(NearlyEqual((a * b) * vector, a * (b * vector), 1e-4f));
}

TEST(MatrixTransposeTest, SwapsShapeAndElements)
{
    const Matrix<float, 2, 3> matrix = mat::Make<float, 2, 3>(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    );

    const auto transposed = mat::Transpose(matrix);

    EXPECT_TRUE((std::is_same_v<decltype(transposed), const Matrix<float, 3, 2>>));

    for (usize row = 0; row < 2; ++row)
    {
        for (usize column = 0; column < 3; ++column)
        {
            EXPECT_FLOAT_EQ(mat::At(matrix, row, column), mat::At(transposed, column, row));
        }
    }
}

TEST(MatrixTransposeTest, AppliedTwiceReturnsOriginal)
{
    const Mat3 matrix = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(matrix, mat::Transpose(mat::Transpose(matrix))));
}

TEST(MatrixTransposeTest, LeavesSymmetricMatrixUnchanged)
{
    const Mat3 matrix = mat::Make<float, 3, 3>(
        1.0f, 2.0f, 3.0f,
        2.0f, 4.0f, 5.0f,
        3.0f, 5.0f, 6.0f
    );

    EXPECT_TRUE(NearlyEqual(matrix, mat::Transpose(matrix)));
}

TEST(MatrixTransposeTest, ReversesProductOrder)
{
    const Mat3 a = MakeArbitrary();
    const Mat3 b = MakeOtherArbitrary();

    EXPECT_TRUE(NearlyEqual(mat::Transpose(a * b), mat::Transpose(b) * mat::Transpose(a), 1e-4f));
}

TEST(MatrixDeterminantTest, HandlesSmallFixedSizes)
{
    const Matrix<float, 1, 1> one = mat::Make<float, 1, 1>(3.0f);

    const Mat2 two = mat::Make<float, 2, 2>(
        1.0f, 2.0f,
        3.0f, 4.0f
    );

    const Mat3 three = mat::Make<float, 3, 3>(
        6.0f, 1.0f, 1.0f,
        4.0f, -2.0f, 5.0f,
        2.0f, 8.0f, 7.0f
    );

    EXPECT_FLOAT_EQ(3.0f, mat::Determinant(one));
    EXPECT_FLOAT_EQ(-2.0f, mat::Determinant(two));
    EXPECT_FLOAT_EQ(-306.0f, mat::Determinant(three));
}

TEST(MatrixDeterminantTest, HandlesFourByFour)
{
    const Mat4 matrix = mat::Make<float, 4, 4>(
        1.0f, 0.0f, 2.0f, -1.0f,
        3.0f, 0.0f, 0.0f, 5.0f,
        2.0f, 1.0f, 4.0f, -3.0f,
        1.0f, 0.0f, 5.0f, 0.0f
    );

    EXPECT_FLOAT_EQ(30.0f, mat::Determinant(matrix));
}

TEST(MatrixDeterminantTest, IdentityHasDeterminantOne)
{
    EXPECT_FLOAT_EQ(1.0f, mat::Determinant(mat2::Identity));
    EXPECT_FLOAT_EQ(1.0f, mat::Determinant(mat3::Identity));
    EXPECT_FLOAT_EQ(1.0f, mat::Determinant(mat4::Identity));
    EXPECT_FLOAT_EQ(1.0f, mat::Determinant(mat::Identity<float, 5>));
}

TEST(MatrixDeterminantTest, SingularMatrixHasZeroDeterminant)
{
    const Mat3 matrix = mat::Make<float, 3, 3>(
        1.0f, 2.0f, 3.0f,
        2.0f, 4.0f, 6.0f,
        1.0f, 0.0f, 1.0f
    );

    EXPECT_FLOAT_EQ(0.0f, mat::Determinant(matrix));
}

TEST(MatrixDeterminantTest, KeepsElementTypeForFloating)
{
    EXPECT_TRUE((std::is_same_v<decltype(mat::Determinant(mat3::Identity)), float>));

    const Matrix<double, 2, 2> matrix = mat::Make<double, 2, 2>(
        1.0, 2.0,
        3.0, 4.0
    );

    EXPECT_TRUE((std::is_same_v<decltype(mat::Determinant(matrix)), double>));
    EXPECT_DOUBLE_EQ(-2.0, mat::Determinant(matrix));
}

TEST(MatrixDeterminantTest, WidensIntegerDeterminantToInt64)
{
    const Matrix<int32, 2, 2> matrix = mat::Make<int32, 2, 2>(
        1, 2,
        3, 4
    );

    EXPECT_TRUE((std::is_same_v<decltype(mat::Determinant(matrix)), int64>));
    EXPECT_EQ(-2, mat::Determinant(matrix));
}

TEST(MatrixDeterminantTest, IntegerDeterminantDoesNotOverflowInIntermediateProducts)
{
    const Matrix<int32, 2, 2> matrix = mat::Make<int32, 2, 2>(
        100000, 0,
        0, 100000
    );

    EXPECT_EQ(int64{10000000000}, mat::Determinant(matrix));
}

TEST(MatrixDeterminantTest, GeneralPathHandlesTriangularMatrix)
{
    Matrix<float, 5, 5> matrix = mat::Zero<float, 5, 5>;

    for (usize row = 0; row < 5; ++row)
    {
        for (usize column = row; column < 5; ++column)
        {
            mat::At(matrix, row, column) = row == column ? static_cast<float>(row + 1) : 2.0f;
        }
    }

    EXPECT_NEAR(120.0f, mat::Determinant(matrix), 1e-3f);
}

TEST(MatrixDeterminantTest, GeneralPathNegatesOnRowSwap)
{
    Matrix<float, 5, 5> matrix = mat::Identity<float, 5>;

    mat::SetRow(matrix, 0, mat::GetRow(mat::Identity<float, 5>, 1));
    mat::SetRow(matrix, 1, mat::GetRow(mat::Identity<float, 5>, 0));

    EXPECT_NEAR(-1.0f, mat::Determinant(matrix), Tolerance);
}

TEST(MatrixDeterminantTest, GeneralPathDetectsSingularMatrix)
{
    Matrix<float, 5, 5> matrix = mat::Identity<float, 5>;

    mat::SetRow(matrix, 3, mat::GetRow(matrix, 2));

    EXPECT_FLOAT_EQ(0.0f, mat::Determinant(matrix));
}

TEST(MatrixDeterminantTest, GeneralPathKeepsVerySmallDeterminants)
{
    Matrix<float, 5, 5> matrix = mat::Zero<float, 5, 5>;

    for (usize i = 0; i < 5; ++i)
    {
        mat::At(matrix, i, i) = 1e-3f;
    }

    EXPECT_GT(mat::Determinant(matrix), 0.0f);
    EXPECT_NEAR(1e-15f, mat::Determinant(matrix), 1e-17f);
}

TEST(MatrixDeterminantTest, MatchesProductOfDeterminants)
{
    const Mat3 a = MakeArbitrary();
    const Mat3 b = MakeOtherArbitrary();

    EXPECT_NEAR(
        mat::Determinant(a) * mat::Determinant(b),
        mat::Determinant(a * b),
        1e-3f
    );
}

TEST(MatrixDeterminantTest, ScalingARowScalesTheDeterminant)
{
    const Mat3 matrix = MakeArbitrary();

    Mat3 scaled = matrix;
    mat::SetRow(scaled, 1, mat::GetRow(matrix, 1) * 3.0f);

    EXPECT_NEAR(mat::Determinant(matrix) * 3.0f, mat::Determinant(scaled), 1e-4f);
}

TEST(MatrixCastTest, ConvertsElementType)
{
    const Matrix<int32, 2, 2> source = mat::Make<int32, 2, 2>(
        1, 2,
        3, 4
    );

    const auto result = mat::Cast<float>(source);

    EXPECT_TRUE((std::is_same_v<decltype(result), const Matrix<float, 2, 2>>));

    for (usize i = 0; i < 4; ++i)
    {
        EXPECT_FLOAT_EQ(static_cast<float>(source.Data[i]), result.Data[i]);
    }
}

TEST(MatrixInverseTest, InvertsTwoByTwo)
{
    const Mat2 matrix = mat::Make<float, 2, 2>(
        4.0f, 7.0f,
        2.0f, 6.0f
    );

    const Mat2 expected = mat::Make<float, 2, 2>(
        0.6f, -0.7f,
        -0.2f, 0.4f
    );

    Mat2 result = mat2::Zero;

    ASSERT_TRUE(mat::TryInverse(matrix, result));
    EXPECT_TRUE(NearlyEqual(expected, result, Tolerance));
}

TEST(MatrixInverseTest, InvertsThreeByThree)
{
    const Mat3 matrix = mat::Make<float, 3, 3>(
        2.0f, 0.0f, 0.0f,
        0.0f, 4.0f, 0.0f,
        0.0f, 0.0f, 0.5f
    );

    const Mat3 expected = mat::Make<float, 3, 3>(
        0.5f, 0.0f, 0.0f,
        0.0f, 0.25f, 0.0f,
        0.0f, 0.0f, 2.0f
    );

    Mat3 result = mat3::Zero;

    ASSERT_TRUE(mat::TryInverse(matrix, result));
    EXPECT_TRUE(NearlyEqual(expected, result, Tolerance));
}

TEST(MatrixInverseTest, ProductWithInverseIsIdentity)
{
    const Mat3 matrix = MakeArbitrary();

    Mat3 inverse = mat3::Zero;

    ASSERT_TRUE(mat::TryInverse(matrix, inverse));

    EXPECT_TRUE(NearlyEqual(mat3::Identity, matrix * inverse, 1e-4f));
    EXPECT_TRUE(NearlyEqual(mat3::Identity, inverse * matrix, 1e-4f));
}

TEST(MatrixInverseTest, HandlesFourByFour)
{
    const Mat4 matrix = mat::Make<float, 4, 4>(
        1.0f, 0.0f, 2.0f, -1.0f,
        3.0f, 0.0f, 0.0f, 5.0f,
        2.0f, 1.0f, 4.0f, -3.0f,
        1.0f, 0.0f, 5.0f, 0.0f
    );

    Mat4 inverse = mat4::Zero;

    ASSERT_TRUE(mat::TryInverse(matrix, inverse));
    EXPECT_TRUE(NearlyEqual(mat4::Identity, matrix * inverse, 1e-4f));
}

TEST(MatrixInverseTest, PivotsWhenLeadingElementIsZero)
{
    const Mat3 matrix = mat::Make<float, 3, 3>(
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    );

    Mat3 inverse = mat3::Zero;

    ASSERT_TRUE(mat::TryInverse(matrix, inverse));
    EXPECT_TRUE(NearlyEqual(matrix, inverse, Tolerance));
    EXPECT_TRUE(NearlyEqual(mat3::Identity, matrix * inverse, Tolerance));
}

TEST(MatrixInverseTest, WidensIntegerInputToRequestedFloatingType)
{
    const Matrix<int32, 2, 2> matrix = mat::Make<int32, 2, 2>(
        4, 7,
        2, 6
    );

    Mat2 result = mat2::Zero;

    ASSERT_TRUE(mat::TryInverse(matrix, result));

    const Mat2 expected = mat::Make<float, 2, 2>(
        0.6f, -0.7f,
        -0.2f, 0.4f
    );

    EXPECT_TRUE(NearlyEqual(expected, result, Tolerance));
}

TEST(MatrixInverseTest, InvertsMatricesWithVerySmallElements)
{
    const Mat2 matrix = mat::Make<float, 2, 2>(
        1e-4f, 0.0f,
        0.0f, 1e-4f
    );

    Mat2 inverse = mat2::Zero;

    ASSERT_TRUE(mat::TryInverse(matrix, inverse));
    EXPECT_NEAR(1e4f, mat::At(inverse, 0, 0), 1.0f);
    EXPECT_NEAR(1e4f, mat::At(inverse, 1, 1), 1.0f);
}

TEST(MatrixInverseTest, ReportsFailureForSingularMatrix)
{
    const Mat2 two = mat::Make<float, 2, 2>(
        1.0f, 2.0f,
        2.0f, 4.0f
    );

    const Mat3 three = mat::Make<float, 3, 3>(
        1.0f, 2.0f, 3.0f,
        2.0f, 4.0f, 6.0f,
        1.0f, 0.0f, 1.0f
    );

    Mat2 twoResult = mat2::Zero;
    Mat3 threeResult = mat3::Zero;

    EXPECT_FALSE(mat::TryInverse(two, twoResult));
    EXPECT_FALSE(mat::TryInverse(three, threeResult));
}

TEST(MatrixInverseTest, LeavesOutputUntouchedOnFailure)
{
    const Mat3 singular = mat::Make<float, 3, 3>(
        1.0f, 2.0f, 3.0f,
        2.0f, 4.0f, 6.0f,
        1.0f, 0.0f, 1.0f
    );

    Mat3 result = MakeArbitrary();
    const Mat3 untouched = result;

    EXPECT_FALSE(mat::TryInverse(singular, result));
    EXPECT_TRUE(NearlyEqual(untouched, result));
}

TEST(MatrixInverseTest, LeavesOutputUntouchedWhenSingularityIsFoundAtLastPivot)
{
    Mat3 singular = mat3::Identity;
    mat::At(singular, 2, 2) = 0.0f;

    Mat3 result = MakeArbitrary();
    const Mat3 untouched = result;

    EXPECT_FALSE(mat::TryInverse(singular, result));
    EXPECT_TRUE(NearlyEqual(untouched, result));
}

TEST(MatrixInverseTest, LeavesOutputUntouchedOnSingularTwoByTwo)
{
    const Mat2 singular = mat::Make<float, 2, 2>(
        1.0f, 2.0f,
        2.0f, 4.0f
    );

    Mat2 result = mat::Make<float, 2, 2>(
        9.0f, 9.0f,
        9.0f, 9.0f
    );

    const Mat2 untouched = result;

    EXPECT_FALSE(mat::TryInverse(singular, result));
    EXPECT_TRUE(NearlyEqual(untouched, result));
}

TEST(MatrixInverseTest, IdentityIsItsOwnInverse)
{
    Mat4 result = mat4::Zero;

    ASSERT_TRUE(mat::TryInverse(mat4::Identity, result));
    EXPECT_TRUE(NearlyEqual(mat4::Identity, result));
}

TEST(MatrixInverseTest, ShorthandReturnsSameResultAsTryInverse)
{
    const Mat3 matrix = MakeArbitrary();

    Mat3 expected = mat3::Zero;

    ASSERT_TRUE(mat::TryInverse(matrix, expected));
    EXPECT_TRUE(NearlyEqual(expected, mat::Inverse(matrix)));
}

TEST(MatrixInverseTest, AppliedTwiceReturnsOriginal)
{
    const Mat3 matrix = MakeArbitrary();

    EXPECT_TRUE(NearlyEqual(matrix, mat::Inverse(mat::Inverse(matrix)), 1e-3f));
}

TEST(MatrixConstexprTest, ExpressionsEvaluateAtCompileTime)
{
    constexpr Mat2 sum = mat2::Identity + mat2::Identity;
    static_assert(sum.Data[0] == 2.0f);

    constexpr Mat2 product = mat2::Identity * mat2::Identity;
    static_assert(product.Data[3] == 1.0f);

    constexpr Vec2 transformed = mat2::Identity * Vec2{1.0f, 2.0f};
    static_assert(transformed.Data[1] == 2.0f);

    constexpr float determinant = mat::Determinant(mat2::Identity);
    static_assert(determinant == 1.0f);

    constexpr Mat2 transposed = mat::Transpose(mat2::Identity);
    static_assert(transposed.Data[0] == 1.0f);

    SUCCEED();
}
