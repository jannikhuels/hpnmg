#include "gtest/gtest.h"

#include <vector>

#include <gmpxx.h>
#include <representations/GeometricObject.h>

#include "datastructures/STDPolytope.h"

using namespace hpnmg;

template<typename Numeric>
std::vector<hypro::Point<Numeric>> createUnitSimplex(int dim) {
    // Allocate a vector with exactly the space we need for all the points. Initialize vertices to (0,...,0)
    auto unitSimplex = std::vector<hypro::Point<Numeric>>(dim + 1, hypro::Point<Numeric>::Zero(dim));

    // Now turn the vertices into (1,0,...,0), (0,1,0,...,0), ..., (0,...,0,1) but leave the first as (0,...,0)
    std::for_each(
        unitSimplex.begin() + 1,
        unitSimplex.end(),
        [i = 0](auto &vertex) mutable {
            vertex.at(i) = 1;
            ++i;
        }
    );

    return unitSimplex;
}

template <typename T>
class STDPolytopeTest : public ::testing::Test {};

using STDPolytopeTypes = ::testing::Types<mpq_class, double>;
TYPED_TEST_CASE(STDPolytopeTest, STDPolytopeTypes);

TYPED_TEST(STDPolytopeTest, TimeSlices)
{
    for (int dim = 2; dim < 20; ++dim) {
        auto vertices = createUnitSimplex<TypeParam>(dim);

        const auto stdpoly = STDPolytope<TypeParam>(hypro::HPolytope<TypeParam>(vertices));

        std::vector<hypro::Point<TypeParam>> timeSliceVertices = stdpoly.timeSlice(0).vertices();
        std::vector<hypro::Point<TypeParam>> expectedVertices = createUnitSimplex<TypeParam>(dim - 1);

        std::sort(timeSliceVertices.begin(), timeSliceVertices.end());
        std::sort(expectedVertices.begin(), expectedVertices.end());

        EXPECT_EQ(expectedVertices, timeSliceVertices);
    }
}

TYPED_TEST(STDPolytopeTest, ExtendDownwardsTwoDimensionsNothingToDo)
{
    auto vertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{0, 0},
        hypro::Point<TypeParam>{1, 0},
        hypro::Point<TypeParam>{0, 1},
    };

    const auto stdpoly = STDPolytope<TypeParam>(hypro::HPolytope<TypeParam>(vertices));

    auto extendedVertices = stdpoly.extendDownwards().vertices();

    std::sort(vertices.begin(), vertices.end());
    std::sort(extendedVertices.begin(), extendedVertices.end());

    EXPECT_EQ(vertices, extendedVertices);
}

TYPED_TEST(STDPolytopeTest, ExtendDownwardsTwoDimensionsOneExtension)
{
    auto vertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{1, 0},
        hypro::Point<TypeParam>{1, 1},
        hypro::Point<TypeParam>{0, 1},
    };

    const auto stdpoly = STDPolytope<TypeParam>(hypro::HPolytope<TypeParam>(vertices));
    auto extendedVertices = stdpoly.extendDownwards().vertices();

    auto expectedVertices = vertices;
    expectedVertices.push_back(hypro::Point<TypeParam>{0, 0});
    std::sort(expectedVertices.begin(), expectedVertices.end());
    std::sort(extendedVertices.begin(), extendedVertices.end());

    EXPECT_EQ(expectedVertices, extendedVertices);
}

TYPED_TEST(STDPolytopeTest, ExtendDownwardsTwoDimensionsTwoExtensions)
{
    auto vertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{0, 1},
        hypro::Point<TypeParam>{2, 1},
        hypro::Point<TypeParam>{1, 2},
    };

    const auto stdpoly = STDPolytope<TypeParam>(hypro::HPolytope<TypeParam>(vertices));
    auto extendedVertices = stdpoly.extendDownwards().vertices();

    auto expectedVertices = vertices;
    expectedVertices.push_back(hypro::Point<TypeParam>{0, 0});
    expectedVertices.push_back(hypro::Point<TypeParam>{2, 0});
    std::sort(expectedVertices.begin(), expectedVertices.end());
    std::sort(extendedVertices.begin(), extendedVertices.end());

    EXPECT_EQ(expectedVertices, extendedVertices);
}

TYPED_TEST(STDPolytopeTest, ExtendDownwardsTwoDimensionsComplexShape)
{
    auto vertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{2, 3},
        hypro::Point<TypeParam>{4, 3},
        hypro::Point<TypeParam>{7, 5},
        hypro::Point<TypeParam>{6, 7},
        hypro::Point<TypeParam>{4, 8},
        hypro::Point<TypeParam>{2, 4},
    };

    const auto stdpoly = STDPolytope<TypeParam>(hypro::HPolytope<TypeParam>(vertices));
    auto extendedVertices = stdpoly.extendDownwards().vertices();

    auto expectedVertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{2, 0},
        hypro::Point<TypeParam>{7, 0},
        hypro::Point<TypeParam>{7, 5},
        hypro::Point<TypeParam>{6, 7},
        hypro::Point<TypeParam>{4, 8},
        hypro::Point<TypeParam>{2, 4},
    };
    std::sort(expectedVertices.begin(), expectedVertices.end());
    std::sort(extendedVertices.begin(), extendedVertices.end());

    ASSERT_EQ(expectedVertices.size(), extendedVertices.size());
    for (int i = 0; i < expectedVertices.size(); ++i) {
        ASSERT_EQ(expectedVertices[i].dimension(), extendedVertices[i].dimension());
        for (int j = 0; i < expectedVertices[i].dimension(); ++i)
            EXPECT_EQ(expectedVertices[i][j], extendedVertices[i][j]) << "Vertices at index " << i << " differ in dimension " << j;
    }
}

TYPED_TEST(STDPolytopeTest, ExtendDownwardsThreeDimensionsNothingToDo)
{
    auto vertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{0, 0, 0},
        hypro::Point<TypeParam>{1, 0, 0},
        hypro::Point<TypeParam>{0, 1, 0},
        hypro::Point<TypeParam>{0, 0, 1},
    };

    const auto stdpoly = STDPolytope<TypeParam>(hypro::HPolytope<TypeParam>(vertices));

    auto extendedVertices = stdpoly.extendDownwards().vertices();

    std::sort(vertices.begin(), vertices.end());
    std::sort(extendedVertices.begin(), extendedVertices.end());

    EXPECT_EQ(vertices, extendedVertices);
}

TYPED_TEST(STDPolytopeTest, ExtendDownwardsThreeDimensionsOneExtension)
{
    auto vertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{1, 0, 0},
        hypro::Point<TypeParam>{0, 1, 0},
        hypro::Point<TypeParam>{1, 1, 0},
        hypro::Point<TypeParam>{0, 0, 1},
    };

    const auto stdpoly = STDPolytope<TypeParam>(hypro::HPolytope<TypeParam>(vertices));
    auto extendedVertices = stdpoly.extendDownwards().vertices();

    auto expectedVertices = vertices;
    expectedVertices.push_back(hypro::Point<TypeParam>{0, 0, 0});
    std::sort(expectedVertices.begin(), expectedVertices.end());
    std::sort(extendedVertices.begin(), extendedVertices.end());

    EXPECT_EQ(expectedVertices, extendedVertices);
}

TYPED_TEST(STDPolytopeTest, ExtendDownwardsThreeDimensionsTwoExtensions)
{
    auto vertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{1, 0, 0},
        hypro::Point<TypeParam>{0, 1, 0},
        hypro::Point<TypeParam>{1, 1, 1},
        hypro::Point<TypeParam>{0, 0, 1},
    };

    const auto stdpoly = STDPolytope<TypeParam>(hypro::HPolytope<TypeParam>(vertices));
    auto extendedVertices = stdpoly.extendDownwards().vertices();

    auto expectedVertices = vertices;
    expectedVertices.push_back(hypro::Point<TypeParam>{0, 0, 0});
    expectedVertices.push_back(hypro::Point<TypeParam>{1, 1, 0});
    std::sort(expectedVertices.begin(), expectedVertices.end());
    std::sort(extendedVertices.begin(), extendedVertices.end());

    EXPECT_EQ(expectedVertices, extendedVertices);
}

TYPED_TEST(STDPolytopeTest, ExtendDownwardsThreeDimensionsComplexShape)
{
    auto vertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{2, 3, 5},
        hypro::Point<TypeParam>{4, 3, 5},
        hypro::Point<TypeParam>{7, 5, 5},
        hypro::Point<TypeParam>{6, 7, 5},
        hypro::Point<TypeParam>{4, 8, 5},
        hypro::Point<TypeParam>{2, 4, 5},

        hypro::Point<TypeParam>{3, 5, 3},
        hypro::Point<TypeParam>{5, 7, 3},

        hypro::Point<TypeParam>{5, 5, 7},
    };

    const auto stdpoly = STDPolytope<TypeParam>(hypro::HPolytope<TypeParam>(vertices));
    auto extendedPoly = stdpoly.extendDownwards();
    auto extendedVertices = extendedPoly.vertices();

    auto expectedVertices = std::vector<hypro::Point<TypeParam>>{
        hypro::Point<TypeParam>{2, 3, 5},
        hypro::Point<TypeParam>{4, 3, 5},
        hypro::Point<TypeParam>{7, 5, 5},
        hypro::Point<TypeParam>{6, 7, 5},
        hypro::Point<TypeParam>{4, 8, 5},
        hypro::Point<TypeParam>{2, 4, 5},

        hypro::Point<TypeParam>{2, 3, 0},
        hypro::Point<TypeParam>{4, 3, 0},
        hypro::Point<TypeParam>{7, 5, 0},
        hypro::Point<TypeParam>{6, 7, 0},
        hypro::Point<TypeParam>{4, 8, 0},
        hypro::Point<TypeParam>{2, 4, 0},

        hypro::Point<TypeParam>{5, 5, 7},
    };
    std::sort(expectedVertices.begin(), expectedVertices.end());
    std::sort(extendedVertices.begin(), extendedVertices.end());

    ASSERT_EQ(expectedVertices.size(), extendedVertices.size());
    for (int i = 0; i < expectedVertices.size(); ++i) {
        ASSERT_EQ(expectedVertices[i].dimension(), extendedVertices[i].dimension());
        for (int j = 0; i < expectedVertices[i].dimension(); ++i)
            EXPECT_EQ(expectedVertices[i][j], extendedVertices[i][j]) << "Vertices at index " << i << " differ in dimension " << j;
    }
}

TEST(ExtendDownwards, AssertionFailure)
{
    auto vertices = std::vector<hypro::Point<double>>{
        hypro::Point<double>{2, 3, 5},
        hypro::Point<double>{4, 3, 5},
        hypro::Point<double>{7, 5, 5},
        hypro::Point<double>{6, 7, 5},
        hypro::Point<double>{4, 8, 5},
        hypro::Point<double>{2, 4, 5},

        hypro::Point<double>{3, 5, 3},
        hypro::Point<double>{5, 7, 3},

        hypro::Point<double>{5, 5, 7},
    };

    const auto stdpoly = hypro::HPolytope<double>(vertices);

    //region Extracted from a member function that "extends" a polytope downwards in the last dimension
        auto newVertices = stdpoly.vertices();
        // The vector must not reallocate (and thus invalidate its iterators) while we duplicate it
        newVertices.reserve(newVertices.size() * 2);
        std::transform(newVertices.begin(), newVertices.end(), std::back_inserter(newVertices), [](hypro::Point<double> vertex) {
            vertex[vertex.dimension() - 1] = 0;
            return vertex;
        });

        auto extendedPoly = hypro::HPolytope<double>(newVertices);
    //endregion Extracted from a member function that "extends" a polytope downwards in the last dimension

    auto extendedVertices = extendedPoly.vertices();

    auto expectedVertices = std::vector<hypro::Point<double>>{
        hypro::Point<double>{2, 3, 5},
        hypro::Point<double>{4, 3, 5},
        hypro::Point<double>{7, 5, 5},
        hypro::Point<double>{6, 7, 5},
        hypro::Point<double>{4, 8, 5},
        hypro::Point<double>{2, 4, 5},

        hypro::Point<double>{2, 3, 0},
        hypro::Point<double>{4, 3, 0},
        hypro::Point<double>{7, 5, 0},
        hypro::Point<double>{6, 7, 0},
        hypro::Point<double>{4, 8, 0},
        hypro::Point<double>{2, 4, 0},

        hypro::Point<double>{5, 5, 7},
    };
    std::sort(expectedVertices.begin(), expectedVertices.end());
    std::sort(extendedVertices.begin(), extendedVertices.end());

    ASSERT_EQ(expectedVertices.size(), extendedVertices.size());
    for (int i = 0; i < expectedVertices.size(); ++i) {
        ASSERT_EQ(expectedVertices[i].dimension(), extendedVertices[i].dimension());
        for (int j = 0; i < expectedVertices[i].dimension(); ++i)
            EXPECT_EQ(expectedVertices[i][j], extendedVertices[i][j]) << "Vertices at index " << i << " differ in dimension " << j;
    }
}
