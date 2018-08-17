#include <gtest/gtest.h>
#include <hpnmg/ReadHybridPetrinet.h>
#include <hpnmg/ParseHybridPetrinet.h>
#include "helper/Triangulation.h"
#include "helper/Computation.h"

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

TEST(ComputationTest, TestUnequationCut) {
    std::vector<double> res = Computation::computeUnequationCut({-5,0,0}, {0, 2,-2});

    ASSERT_EQ(res.size(), 3);
    ASSERT_EQ(res[0], 2.5);
    ASSERT_EQ(res[1], 1);
    ASSERT_EQ(res[2], 0);

    //TODO Add tests here
    res = Computation::computeUnequationCut({2,1,2},{0,-1,1},1);

    ASSERT_EQ(res.size(), 3);
    ASSERT_EQ(res[0], -1);
    ASSERT_EQ(res[1], 0);
    ASSERT_EQ(res[2], -0.5);
}

TEST(ComputationTest, TestSolveEquations) {
    std::vector<std::pair<std::vector<double>, std::vector<double>>> noChange = Computation::solveEquations({{6,0}}, 4);

    ASSERT_EQ(noChange.size(), 1);
    ASSERT_EQ(noChange[0].first.size(), 0);
    ASSERT_EQ(noChange[0].second.size(), 0);

    std::vector<std::pair<std::vector<double>, std::vector<double>>> res = Computation::solveEquations({{0,1},{10,-1}}, 4);

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0].first.size(), 2);
    ASSERT_EQ(res[0].first[0], 4);
    ASSERT_EQ(res[0].first[1], 0);
    ASSERT_EQ(res[0].second.size(), 2);
    ASSERT_EQ(res[0].second[0], 6);
    ASSERT_EQ(res[0].second[1], 0);

    res = Computation::solveEquations({{0,-2,2},{20,0,-2}}, 4);
    ASSERT_EQ(res.size(), 2);
    ASSERT_EQ(res[0].first.size(), 0);
    ASSERT_EQ(res[0].second.size(), 3);
    ASSERT_EQ(res[0].second[0], -2);
    ASSERT_EQ(res[0].second[1], 0);
    ASSERT_EQ(res[0].second[2], 1);
    ASSERT_EQ(res[1].first.size(), 3);
    ASSERT_EQ(res[1].first[0], 2);
    ASSERT_EQ(res[1].first[1], 1);
    ASSERT_EQ(res[1].first[2], 0);
    ASSERT_EQ(res[1].second.size(), 3);
    ASSERT_EQ(res[1].second[0], 8);
    ASSERT_EQ(res[1].second[1], 0);
    ASSERT_EQ(res[1].second[2], 0);

    res = Computation::solveEquations({{0,1,0}, {0,0,1}, {5,0,0}}, 4);
    ASSERT_EQ(res.size(), 2);
    ASSERT_EQ(res[0].first.size(), 3);
    ASSERT_EQ(res[0].first[0], 4);
    ASSERT_EQ(res[0].first[1], 0);
    ASSERT_EQ(res[0].first[2], 0);
    ASSERT_EQ(res[0].second.size(), 0);
    ASSERT_EQ(res[1].first.size(), 3);
    ASSERT_EQ(res[1].first[0], 4);
    ASSERT_EQ(res[1].first[1], 0);
    ASSERT_EQ(res[1].first[2], 0);
    ASSERT_EQ(res[1].second.size(), 0);
}

TEST(ComputationTest, SingeReplaceTest) {
    std::vector<double> newFunc = Computation::replace({-2, 0, 1}, {8, 0, 0}, 2);
    ASSERT_EQ(newFunc.size(), 3);
    ASSERT_EQ(newFunc[0], 6);
    ASSERT_EQ(newFunc[1], 0);
    ASSERT_EQ(newFunc[2], 0);

    newFunc = Computation::replace({6, 2, 2}, {1,1,0}, 2);
    ASSERT_EQ(newFunc.size(), 3);
    ASSERT_EQ(newFunc[0], 8);
    ASSERT_EQ(newFunc[1], 4);
    ASSERT_EQ(newFunc[2], 0);
}

TEST(ComputationTest, ReplaceValuesTest) {
    std::vector<std::pair<std::vector<double>, std::vector<double>>> replaced = Computation::replaceValues({std::pair<std::vector<double>,std::vector<double>>({},{-2,0,1}), std::pair<std::vector<double>,std::vector<double>>({2,1,0},{8,0,0})});
    ASSERT_EQ(replaced.size(),2);
    ASSERT_EQ(replaced[0].first.size(), 0);
    ASSERT_EQ(replaced[0].second.size(), 3);
    ASSERT_EQ(replaced[0].second[0], 6);
    ASSERT_EQ(replaced[0].second[1], 0);
    ASSERT_EQ(replaced[0].second[2], 0);
    ASSERT_EQ(replaced[1].first.size(), 3);
    ASSERT_EQ(replaced[1].first[0], 2);
    ASSERT_EQ(replaced[1].first[1], 1);
    ASSERT_EQ(replaced[1].first[2], 0);
    ASSERT_EQ(replaced[1].second.size(), 3);
    ASSERT_EQ(replaced[1].second[0], 8);
    ASSERT_EQ(replaced[1].second[1], 0);
    ASSERT_EQ(replaced[1].second[2], 0);
}
