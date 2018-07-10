#include <gtest/gtest.h>
#include <hpnmg/ReadHybridPetrinet.h>
#include <hpnmg/ParseHybridPetrinet.h>
#include "helper/Triangulation.h"

using namespace hpnmg;


TEST(TriangulationTest, Test2D) {
    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("example.xml");
    ParseHybridPetrinet parser;
    shared_ptr<hpnmg::ParametricLocationTree> plt = parser.parseHybridPetrinet(hybridPetrinet, 20);
    plt->updateRegions();

    std::vector<ParametricLocationTree::Node> candidates = plt->getCandidateLocationsForTime(4);

    ASSERT_EQ(candidates.size(), 3);
    vector<Region> triangles = Triangulation::create(candidates[0]);
    ASSERT_EQ(triangles.size(), 2);

    ASSERT_EQ(triangles[0].contains(Point<double>({0,0})), true);
    ASSERT_EQ(triangles[0].contains(Point<double>({20,0})), true);
    ASSERT_EQ(triangles[0].contains(Point<double>({20,5})), true);
    ASSERT_EQ(triangles[0].contains(Point<double>({6,5})), false);
    ASSERT_EQ(triangles[1].contains(Point<double>({6,5})), true);
    ASSERT_EQ(triangles[1].contains(Point<double>({20,5})), true);
    ASSERT_EQ(triangles[1].contains(Point<double>({5,5})), true);
    ASSERT_EQ(triangles[1].contains(Point<double>({6,20})), false);
}

