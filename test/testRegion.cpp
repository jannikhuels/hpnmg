#include "gtest/gtest.h"

#include "datastructures/Region.h"

#include "representations/GeometricObject.h"

#include <vector>

using namespace hpnmg;

auto createUnitSimplex(int dim) {
    // Allocate a vector with exactly the space we need for all the points. Initialize vertices to (0,...,0)
    auto unitSimplex = std::vector<hypro::Point<double>>(dim + 1, hypro::Point<double>::Zero(dim));

    // Now turn the vertices into (1,0,...,0), (0,1,0,...,0), ..., (0,...,0,1) but leave the first as (0,...,0)
    std::for_each(
            unitSimplex.begin() + 1,
            unitSimplex.end(),
            [i = 0](auto &vertex) mutable {
                vertex.at(i) = 1;
                i++;
            }
    );

    return unitSimplex;
}

TEST(RegionTest, IntegrationTransformation2D)
{
    auto simplex = std::vector<hypro::Point<double>>{
        hypro::Point<double>{1, 0},
        hypro::Point<double>{3, 0},
        hypro::Point<double>{2, 2},
    };

    Region region{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(simplex))};
    const auto &trans = region.getIntegrationTransformationMatrix();

    auto mappedUnitSimplex = std::vector<hypro::Point<double>>();
    for (const auto &vertex : createUnitSimplex(2)) {
        mappedUnitSimplex.emplace_back((trans * Eigen::VectorXd(vertex.rawCoordinates()).homogeneous()).hnormalized());
    }

    std::sort(simplex.begin(), simplex.end());
    std::sort(mappedUnitSimplex.begin(), mappedUnitSimplex.end());

    ASSERT_EQ(simplex, mappedUnitSimplex);
}

TEST(RegionTest, IntegrationTransformation3D)
{
    auto simplex = std::vector<hypro::Point<double>>{
        hypro::Point<double>{1, 0, 0},
        hypro::Point<double>{0, 1, 0},
        hypro::Point<double>{0, 0, 1},
        hypro::Point<double>{1, 1, 1},
    };

    Region region{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(simplex))};
    const auto &trans = region.getIntegrationTransformationMatrix();

    auto mappedUnitSimplex = std::vector<hypro::Point<double>>();
    for (const auto &vertex : createUnitSimplex(3)) {
        mappedUnitSimplex.emplace_back((trans * Eigen::VectorXd(vertex.rawCoordinates()).homogeneous()).hnormalized());
    }

    std::sort(simplex.begin(), simplex.end());
    std::sort(mappedUnitSimplex.begin(), mappedUnitSimplex.end());

    ASSERT_EQ(simplex, mappedUnitSimplex);
}
