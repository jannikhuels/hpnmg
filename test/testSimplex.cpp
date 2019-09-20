#include "gtest/gtest.h"

#include "datastructures/Simplex.h"
#include "datastructures/STDPolytope.h"
#include "hpnmg/helper/Triangulation.h"

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

TEST(SimplexTest, IntegrationTransformation2D)
{
    auto vertices = std::vector<hypro::Point<double>>{
        hypro::Point<double>{1, 0},
        hypro::Point<double>{3, 0},
        hypro::Point<double>{2, 2},
    };

    const auto &trans = Simplex{hypro::VPolytope<double>(vertices)}.getIntegrationTransformationMatrix();

    auto mappedUnitSimplex = std::vector<hypro::Point<double>>();
    for (const auto &vertex : createUnitSimplex(2)) {
        mappedUnitSimplex.emplace_back((trans * Eigen::VectorXd(vertex.rawCoordinates()).homogeneous()).hnormalized());
    }

    std::sort(vertices.begin(), vertices.end());
    std::sort(mappedUnitSimplex.begin(), mappedUnitSimplex.end());

    ASSERT_EQ(vertices, mappedUnitSimplex);
}

TEST(SimplexTest, IntegrationTransformation3D)
{
    auto vertices = std::vector<hypro::Point<double>>{
        hypro::Point<double>{1, 0, 0},
        hypro::Point<double>{0, 1, 0},
        hypro::Point<double>{0, 0, 1},
        hypro::Point<double>{1, 1, 1},
    };

    const auto &trans = Simplex{hypro::VPolytope<double>(vertices)}.getIntegrationTransformationMatrix();

    auto mappedUnitSimplex = std::vector<hypro::Point<double>>();
    for (const auto &vertex : createUnitSimplex(3)) {
        mappedUnitSimplex.emplace_back((trans * Eigen::VectorXd(vertex.rawCoordinates()).homogeneous()).hnormalized());
    }

    std::sort(vertices.begin(), vertices.end());
    std::sort(mappedUnitSimplex.begin(), mappedUnitSimplex.end());

    ASSERT_EQ(vertices, mappedUnitSimplex);
}

TEST(SimplexTest, TriangulationTransformation)
{
    auto quadrilateral = std::vector<hypro::Point<double>>{
        hypro::Point<double>{0, 1},
        hypro::Point<double>{2, 0},
        hypro::Point<double>{4, 2},
        hypro::Point<double>{2, 4},
    };

    hypro::HPolytope<double> quadRegion{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(quadrilateral))};

    const auto simplexes = Triangulation::create(quadRegion);

    EXPECT_EQ(simplexes.size(), 2);

    for (auto &simplex : simplexes) {
        const auto &trans = simplex.getIntegrationTransformationMatrix();

        auto mappedUnitSimplex = std::vector<hypro::Point<double>>();
        for (const auto &vertex : createUnitSimplex(2)) {
            mappedUnitSimplex.emplace_back((trans * Eigen::VectorXd(vertex.rawCoordinates()).homogeneous()).hnormalized());
        }

        auto simplexVertices = simplex.vertices();
        std::sort(simplexVertices.begin(), simplexVertices.end());
        std::sort(mappedUnitSimplex.begin(), mappedUnitSimplex.end());

        EXPECT_EQ(simplexVertices, mappedUnitSimplex);
    }
}
