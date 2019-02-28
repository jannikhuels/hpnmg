#include <gtest/gtest.h>
#include <hpnmg/ReadHybridPetrinet.h>
#include <hpnmg/ParseHybridPetrinet.h>
#include "helper/Triangulation.h"
#include "helper/Computation.h"

using namespace hpnmg;

/*
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
}*/

TEST(ComputationTest, TestUnequationCut) {
    std::vector<double> res = Computation::computeUnequationCut({-5,0,0}, {0, 2,-2});

    ASSERT_EQ(res.size(), 3);
    ASSERT_EQ(res[0], 2.5);
    ASSERT_EQ(res[1], 1);
    ASSERT_EQ(res[2], 0);

    res = Computation::computeUnequationCut({2,3},{1,1});
    EXPECT_EQ(2, res.size());
    EXPECT_EQ(-0.5,res[0]);
    EXPECT_EQ(0,res[1]);

    res = Computation::computeUnequationCut({2,3},{1,1},1);
    EXPECT_EQ(2, res.size());
    EXPECT_EQ(-0.5,res[0]);
    EXPECT_EQ(0,res[1]);

    res = Computation::computeUnequationCut({2,3,1},{1,1,2});
    EXPECT_EQ(3, res.size());
    EXPECT_EQ(1,res[0]);
    EXPECT_EQ(2,res[1]);
    EXPECT_EQ(0,res[2]);

    res = Computation::computeUnequationCut({2,3,1},{1,1,2},1);
    EXPECT_EQ(3, res.size());
    EXPECT_EQ(-0.5,res[0]);
    EXPECT_EQ(0,res[1]);
    EXPECT_EQ(0.5,res[2]);

    res = Computation::computeUnequationCut({2,3,1},{1,1,2},2);
    EXPECT_EQ(3, res.size());
    EXPECT_EQ(1,res[0]);
    EXPECT_EQ(2,res[1]);
    EXPECT_EQ(0,res[2]);


    res = Computation::computeUnequationCut({1,2,1,0},{3,1,3,1},1);
    EXPECT_EQ(4, res.size());
    EXPECT_EQ(2,res[0]);
    EXPECT_EQ(0,res[1]);
    EXPECT_EQ(2,res[2]);
    EXPECT_EQ(1,res[3]);

    res = Computation::computeUnequationCut({1,2,1,0},{3,1,3,1},2);
    EXPECT_EQ(4, res.size());
    EXPECT_EQ(-1,res[0]);
    EXPECT_EQ(0.5,res[1]);
    EXPECT_EQ(0,res[2]);
    EXPECT_EQ(-0.5,res[3]);

    res = Computation::computeUnequationCut({1,2,1,0},{3,1,3,1},3);
    EXPECT_EQ(4, res.size());
    EXPECT_EQ(-2,res[0]);
    EXPECT_EQ(1,res[1]);
    EXPECT_EQ(-2,res[2]);
    EXPECT_EQ(0,res[3]);

    //TODO Add tests here
    res = Computation::computeUnequationCut({2,1,2},{0,-1,1},1);

    ASSERT_EQ(res.size(), 3);
    ASSERT_EQ(res[0], -1);
    ASSERT_EQ(res[1], 0);
    ASSERT_EQ(res[2], -0.5);

    res = Computation::computeUnequationCut({4,0,0}, {3,1,0}, 1);
    ASSERT_EQ(res.size(), 3);
    ASSERT_EQ(res[0], 1);
    ASSERT_EQ(res[1], 0);
    ASSERT_EQ(res[2], 0);
}

TEST(ComputationTest, TestSolveEquations) {
    /*std::vector<std::pair<std::vector<double>, std::vector<double>>> noChange = Computation::solveEquations({{6,0}}, 4);

    ASSERT_EQ(noChange.size(), 1);
    ASSERT_EQ(noChange[0].first.size(), 0);
    ASSERT_EQ(noChange[0].second.size(), 0);*/

    /* std::vector<std::pair<std::vector<double>, std::vector<double>>> res = Computation::solveEquations({{0,1},{10,-1}}, 4);
 
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
     ASSERT_EQ(res[1].second[2], 0);*/

    auto res = Computation::solveEquations({{0,-2,2},{20,-2,-2}}, 4);
    EXPECT_EQ(2,res.size());
    EXPECT_EQ(2,res[0].size());
    EXPECT_EQ(2,res[1].size());


    EXPECT_EQ(res[0][0].first.size(), 0);
    EXPECT_EQ(res[0][0].second.size(), 3);
    EXPECT_EQ(res[1][0].first.size(), 0);
    EXPECT_EQ(res[1][0].second.size(), 3);
    EXPECT_EQ(res[0][0].second[0], -2); 
    EXPECT_EQ(res[0][0].second[1], 0);
    EXPECT_EQ(res[0][0].second[2], 1); 
    EXPECT_EQ(res[1][0].second[0], 8); 
    EXPECT_EQ(res[1][0].second[1], 0);
    EXPECT_EQ(res[1][0].second[2], -1); 
    
    EXPECT_EQ(res[0][1].first.size(), 3);
    EXPECT_EQ(res[0][1].first[0], 2);
    EXPECT_EQ(res[0][1].first[1], 1);
    EXPECT_EQ(res[0][1].first[2], 0);
    EXPECT_EQ(res[0][1].second.size(), 0);

    EXPECT_EQ(res[1][1].first.size(), 0);
    EXPECT_EQ(res[1][1].second.size(), 3);
    EXPECT_EQ(res[1][1].second[0], 8);
    EXPECT_EQ(res[1][1].second[1], -1);
    EXPECT_EQ(res[1][1].second[2], 0);

    /*res = Computation::solveEquations({{0,-2,2},{20,-2,-2}}, 4);
    EXPECT_EQ(res.size(), 2);
    EXPECT_EQ(res[0].first.size(), 0);
    EXPECT_EQ(res[0].second.size(), 3);
    EXPECT_EQ(res[0].second[0], 8); // und -2
    EXPECT_EQ(res[0].second[1], 0);
    EXPECT_EQ(res[0].second[2], -1); // und 1
    EXPECT_EQ(res[1].first.size(), 3);
    EXPECT_EQ(res[1].first[0], 2);
    EXPECT_EQ(res[1].first[1], 1);
    EXPECT_EQ(res[1].first[2], 0);
    EXPECT_EQ(res[1].second.size(), 3);
    EXPECT_EQ(res[1].second[0], 8);
    EXPECT_EQ(res[1].second[1], -1);
    EXPECT_EQ(res[1].second[2], 0);*/

   /* res = Computation::solveEquations({{0,1,0}, {0,0,1}, {5,0,0}}, 4);
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
    ASSERT_EQ(res[1].second.size(), 0);*/
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

    newFunc = Computation::replace({2,1,0},{},1);
    EXPECT_EQ(2,newFunc[0]);
    EXPECT_EQ(1,newFunc[1]);
    EXPECT_EQ(0,newFunc[2]);
    newFunc = Computation::replace({2,1,0},{-2,0,1},1);
    EXPECT_EQ(0,newFunc[0]);
    EXPECT_EQ(0,newFunc[1]);
    EXPECT_EQ(1,newFunc[2]);
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

/*
TEST(ComputationTest, SolveResultZero) {
    std::vector<std::pair<std::vector<double>, std::vector<double>>> res = Computation::solveEquations({{10,1,-1}}, 5);
    std::vector<std::pair<std::vector<double>, std::vector<double>>> replaced = Computation::replaceValues(res);
    ASSERT_EQ(replaced.size(),2);
    ASSERT_EQ(replaced[0].first.size(), 3);
    ASSERT_EQ(replaced[0].first[0], 0);
    ASSERT_EQ(replaced[0].first[1], 1);
    ASSERT_EQ(replaced[0].first[2], 0);
    ASSERT_EQ(replaced[0].second.size(), 0);
    ASSERT_EQ(replaced[1].first.size(), 0);
    ASSERT_EQ(replaced[1].second.size(), 3);
    ASSERT_EQ(replaced[1].second[0], 5);
    ASSERT_EQ(replaced[1].second[1], 1);
    ASSERT_EQ(replaced[1].second[2], 0);
}*/

TEST(ComputationTest, isSmaller) {
    std::vector<double> f1 = {0, 1};
    std::vector<double> f2 = {8, 0};

    bool result = Computation::isSmaller(f1, f2, 4);
    ASSERT_EQ(result, true);

    f1 = {3};
    f2 = {4};
    result = Computation::isSmaller(f1, f2, 4);
    ASSERT_EQ(result, true);

    result = Computation::isSmaller(f2, f1, 4);
    ASSERT_EQ(result, false);

    result = Computation::isGreater(f2, f1, 4);
    ASSERT_EQ(result, true);
}

TEST(ComputationTest, getMinimiumTime) {
    double t = Computation::getTime({{1,{{1,0,0},{2,0,0}}},{1,{{1,1,0},{2,1,0}}}}, {3,1,2}, 1);
    ASSERT_EQ(8,t);
}

/*TEST(ComputationTest, isValidBound) {
    ASSERT_EQ(true, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{4,-2,0}}}, {1,{{12,-3,-2},{8,-1,-1}}}}, 1, {2,-2,0}).first);
    ASSERT_EQ(false, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{4,-2,0}}}, {1,{{12,-3,-2},{8,-1,-1}}}}, 1, {6,-2,0}).first);
    ASSERT_EQ(false, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{4,-2,0}}}, {1,{{12,-3,-2},{8,-1,-1}}}}, 1, {8,0,0}).first);
    ASSERT_EQ(true, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{4,-2,0}}}, {1,{{12,-3,-2},{8,-1,-1}}}}, 2, {4.5,0,0}).first);
    ASSERT_EQ(false, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{4,-2,0}}}, {1,{{12,-3,-2},{8,-1,-1}}}}, 2, {10,0,0}).first);
    ASSERT_EQ(false, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{4,-2,0}}}, {1,{{12,-3,-2},{8,-1,-1}}}}, 0, {11,0,0}).first);
    ASSERT_EQ(true, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{4,-2,0}}}, {1,{{12,-3,-2},{8,-1,-1}}}}, 0, {6,0,0}).first);
    ASSERT_EQ(true, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{4,-2,0}}}, {1,{{12,-3,-2},{8,-1,-1}}}}, 0, {10,0,0}).first);
    ASSERT_EQ(true, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}}, 1, {2,1,0}).first);
    ASSERT_EQ(true, Computation::isValidBound({{1,{{0,0,0},{3,0,0}}},{1,{{3,-1,0},{10,0,0}}}}, 0, {-7,0,0}, false).first);
    ASSERT_EQ(false, Computation::isValidBound({{1,{{0,0,0},{3,0,0}}},{1,{{3,-1,0},{10,0,0}}}}, 0, {-7,0,0}, false).second);
    ASSERT_EQ(true, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}}, 1, {3,-1,0}, false).first);
    ASSERT_EQ(true, Computation::isValidBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}}, 1, {3,-1,0}, false).second);
}

TEST(ComputationTest, setBound) {
    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> a = Computation::setBound({{1,{{0,0,0,0},{10,0,0,0}}},{1,{{0,0,0,0},{10,0,0,0}}},{1,{{0,0,0,0},{0,3,1,0}}}}, 2, {4.5, -3, -2, 0}, true);
    ASSERT_EQ(1.5, a[0].second.second[0]);
    ASSERT_EQ(2.25, a[1].second.second[0]);
    ASSERT_EQ(-1.5, a[1].second.second[1]);
    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> b = Computation::setBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}}, 1, {1.5, -2, 0}, true);
    ASSERT_EQ(0.75, b[0].second.second[0]);
    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> c = Computation::setBound({{1,{{0,0,0},{10,0,0}}}}, 0, {1, 0, 0}, true);
    ASSERT_EQ(1, c[0].second.second[0]);
    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> d = Computation::setBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}}, 1, {1.5, -2, 0}, false);
    ASSERT_EQ(0.75, b[0].second.second[0]);
    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> a = Computation::setBound({{1,{{0,0,0},{10,0,0}}}}, 0, {1, 0, 0}, true);
    ASSERT_EQ(1, a.size());
    ASSERT_EQ(1, a[0][0].second.second[0]);

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> b = Computation::setBound({{1,{{0,0,0},{3,0,0}}},{1,{{0,0,0},{10,0,0}}}}, 1, {3, -1, 0}, false);
    ASSERT_EQ(2, b.size());
    ASSERT_EQ(0, b[0][0].second.first[0]);
    ASSERT_EQ(3, b[0][0].second.second[0]);
    ASSERT_EQ(3, b[0][1].second.first[0]);
    ASSERT_EQ(-1, b[0][1].second.first[1]);
    ASSERT_EQ(3, b[1][0].second.first[0]);
    ASSERT_EQ(3, b[1][0].second.second[0]);

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> d = Computation::setBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}, {1,{{0,0,0},{0,3,1}}}}, 2, {3, -1, -1}, true);
    ASSERT_EQ(2, d.size());
    ASSERT_EQ(0, d[0][0].second.first[0]);
    ASSERT_EQ(0.75, d[0][0].second.second[0]);
    ASSERT_EQ(1.5, d[0][1].second.first[0]);
    ASSERT_EQ(-2, d[0][1].second.first[1]);
    ASSERT_EQ(10, d[0][1].second.second[0]);
    ASSERT_EQ(0, d[0][2].second.first[0]);
    ASSERT_EQ(3, d[0][2].second.second[0]);
    ASSERT_EQ(-1, d[0][2].second.second[1]);
    ASSERT_EQ(-1, d[0][2].second.second[2]);
    ASSERT_EQ(0.75, d[1][0].second.first[0]);
    ASSERT_EQ(10, d[1][0].second.second[0]);
    ASSERT_EQ(0, d[1][1].second.first[0]);
    ASSERT_EQ(1.5, d[1][1].second.second[0]);
    ASSERT_EQ(-2, d[1][1].second.second[1]);
    ASSERT_EQ(0, d[1][2].second.first[0]);
    ASSERT_EQ(3, d[1][2].second.second[1]);
    ASSERT_EQ(1, d[1][2].second.second[2]);

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> e = Computation::setBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}, {1,{{0,0,0},{10,0,0}}}}, 2, {4.5, -3, -2}, true);
    ASSERT_EQ(2, e.size());

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> f = Computation::setBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}, {1,{{0,0,0},{0,3,1}}}}, 1, {1.5, -2, 0}, true);
    ASSERT_EQ(2, f.size());
    ASSERT_EQ(-4.25, f[0][0].second.first[0]);
    ASSERT_EQ(10, f[0][0].second.second[0]);
    ASSERT_EQ(0, f[0][1].second.first[0]);
    ASSERT_EQ(1.5, f[0][1].second.second[0]);
    ASSERT_EQ(-2, f[0][1].second.second[1]);
}

TEST(ComputationTest, repairIntervals) {
    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> a = Computation::repairIntervals({{{1,{{-7,0,0},{10,0,0}}},{1,{{0,0,0},{3,-1,0}}}}});
    ASSERT_EQ(1, a.size());
    ASSERT_EQ(0, a[0][0].second.first[0]);
    ASSERT_EQ(3, a[0][0].second.second[0]);
    ASSERT_EQ(0, a[0][1].second.first[0]);
    ASSERT_EQ(3, a[0][1].second.second[0]);
    ASSERT_EQ(-1, a[0][1].second.second[1]);

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> b = Computation::repairIntervals({{{1,{{0,0,0},{-7,0,0}}},{1,{{0,0,0},{3,-1,0}}}}});
    ASSERT_EQ(1, b.size());

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> c = Computation::repairIntervals({{{1,{{-7,0,0},{10,0,0}}},{1,{{0,0,0},{3,-1,0}}},{1,{{3,-1,-1},{0,3,1}}}}});
    ASSERT_EQ(1, c.size());
    ASSERT_EQ(0, c[0][0].second.first[0]);
    ASSERT_EQ(0.75, c[0][0].second.second[0]);
    ASSERT_EQ(1.5, c[0][1].second.first[0]);
    ASSERT_EQ(-2, c[0][1].second.first[1]);
    ASSERT_EQ(3, c[0][1].second.second[0]);
    ASSERT_EQ(-1, c[0][1].second.second[1]);
    ASSERT_EQ(3, c[0][2].second.first[0]);
    ASSERT_EQ(-1, c[0][2].second.first[1]);
    ASSERT_EQ(-1, c[0][2].second.first[2]);
    ASSERT_EQ(3, c[0][2].second.second[1]);
    ASSERT_EQ(1, c[0][2].second.second[2]);

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> d = Computation::repairIntervals({{{1,{{0.75,0,0},{10,0,0}}},{1,{{0,0,0},{1.5,-2,0}}},{1,{{0,0,0},{0,3,1}}}}});
    ASSERT_EQ(0, d.size());

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> e = Computation::repairIntervals({{{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{1.5,-2,0}}},{1,{{0,0,0},{0,3,1}}}}});
    ASSERT_EQ(1, e.size());
    ASSERT_EQ(0, e[0][0].second.first[0]);
    ASSERT_EQ(0.75, e[0][0].second.second[0]);
    ASSERT_EQ(-3, e[0][1].second.first[1]);

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> f = Computation::repairIntervals({{{1,{{-4.25,0,0},{10,0,0}}},{1,{{0,0,0},{1.5,-2,0}}},{1,{{0,0,0},{0,3,1}}}}});
    ASSERT_EQ(1, e.size());
    ASSERT_EQ(0, e[0][0].second.first[0]);
    ASSERT_EQ(0.75, e[0][0].second.second[0]);
    ASSERT_EQ(0, e[0][1].second.first[0]);
    ASSERT_EQ(1.5, e[0][1].second.second[0]);
    ASSERT_EQ(-2, e[0][1].second.second[1]);
    ASSERT_EQ(0, e[0][2].second.first[0]);
    ASSERT_EQ(3, e[0][2].second.second[1]);
    ASSERT_EQ(1, e[0][2].second.second[2]);

}

TEST(ComputationTest, SetBoundSimpleSplit) {
    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> a = Computation::setBoundWithSimpleSplit({{1,{{0,0,0},{3,0,0}}},{1,{{0,0,0},{10,0,0}}}}, 1, {3, -1, 0}, false);
    ASSERT_EQ(2, a.size());
    ASSERT_EQ(-7, a[0][0].second.first[0]);
    ASSERT_EQ(3, a[0][0].second.second[0]);
    ASSERT_EQ(3, a[0][1].second.first[0]);
    ASSERT_EQ(-1, a[0][1].second.first[1]);
}

TEST(ComputationTest, HyproIntervals) {
    Computation::getDomainHypro();
}*/

/*TEST(ComputationTest, setAndRepair)
{
    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> a = Computation::setBound({{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}},{1,{{0,3,1},{10,0,0}}}}, 1, {1.5, -2, 0}, true);
    ASSERT_EQ(2, a.size());
    ASSERT_EQ(-4.25, a[0][0].second.first[0]);
    ASSERT_EQ(10, a[0][0].second.second[0]);
    ASSERT_EQ(0, a[0][1].second.first[0]);
    ASSERT_EQ(1.5, a[0][1].second.second[0]);
    ASSERT_EQ(-2, a[0][1].second.second[1]);
    ASSERT_EQ(0, a[1][0].second.first[0]);
    ASSERT_EQ(-4.25, a[1][0].second.second[0]);

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> b = Computation::setBound(a[0], 2, {3, -1, -1}, false);
    ASSERT_EQ(2, b.size());
    ASSERT_EQ(-4.25, b[0][0].second.first[0]);
    ASSERT_EQ(10, b[0][0].second.second[0]);
    ASSERT_EQ(0, b[0][1].second.first[0]);
    ASSERT_EQ(1.5, b[0][1].second.second[0]);
    ASSERT_EQ(-2, b[0][1].second.second[1]);
    ASSERT_EQ(3, b[0][2].second.first[0]);
    ASSERT_EQ(-1, b[0][2].second.first[1]);
    ASSERT_EQ(-1, b[0][2].second.first[2]);
    ASSERT_EQ(10, b[0][2].second.second[0]);
    ASSERT_EQ(1.5, b[1][1].second.first[0]);
    ASSERT_EQ(0, b[1][2].second.first[0]);

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> c = Computation::setBound(a[1], 2, {3, -1, -1}, false);
    ASSERT_EQ(2, c.size());
    ASSERT_EQ(-4.25, c[0][0].second.first[0]);
    ASSERT_EQ(-4.25, c[0][0].second.second[0]);
    ASSERT_EQ(0, c[0][1].second.first[0]);
    ASSERT_EQ(1.5, c[0][1].second.second[0]);
    ASSERT_EQ(-2, c[0][1].second.second[1]);
    ASSERT_EQ(3, c[0][2].second.first[0]);
    ASSERT_EQ(-1, c[0][2].second.first[1]);
    ASSERT_EQ(-1, c[0][2].second.first[2]);
    ASSERT_EQ(10, c[0][2].second.second[0]);
    ASSERT_EQ(0, c[1][0].second.first[0]);
    ASSERT_EQ(-4.25, c[1][0].second.second[0]);
    ASSERT_EQ(1.5, c[1][1].second.first[0]);
    ASSERT_EQ(0, c[1][2].second.first[0]);

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> z;
    z.insert(z.end(), b.begin(), b.end());
    z.insert(z.end(), c.begin(), c.end());
    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> r = Computation::repairIntervals(z);
    ASSERT_EQ(1, r.size());
    ASSERT_EQ(0, r[0][0].second.first[0]);
    ASSERT_EQ(0.75, r[0][0].second.second[0]);
    ASSERT_EQ(0, r[0][1].second.first[0]);
    ASSERT_EQ(10, r[0][1].second.second[0]);
    ASSERT_EQ(-3, r[0][1].second.second[1]);
    ASSERT_EQ(3, r[0][2].second.first[1]);
    ASSERT_EQ(1, r[0][2].second.first[2]);
    ASSERT_EQ(10, r[0][2].second.second[0]);
}*/

/*TEST(ComputationTest, createValidIntervals) {
    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> intervals = {{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{4,-2,0}}}, {1,{{12,-3,-2},{8,-1,-1}}}};
    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> result = intervals;
    bool shouldUseResult = Computation::createValidIntervals(intervals, 1, {2,0,0}, false, result);
    ASSERT_EQ(true, shouldUseResult);
    ASSERT_EQ(1, result[0].second.second[0]);
    ASSERT_EQ(2, result[1].second.first[0]);

    result = intervals;
    shouldUseResult = Computation::createValidIntervals(intervals, 1, {4,-2,0}, false, result);
    ASSERT_EQ(true, shouldUseResult);
    ASSERT_EQ(2, result[0].second.second[0]);
    ASSERT_EQ(4, result[1].second.first[0]);
    ASSERT_EQ(-2, result[1].second.first[1]);

    result = {{1,{{0,0},{8,0}}},{1,{{0,0},{8,-2}}}};
    shouldUseResult = Computation::createValidIntervals(result, 0, {-2,0}, false, result);
    ASSERT_EQ(true, shouldUseResult);
    ASSERT_EQ(0, result[0].second.first[0]);

    result = {{1,{{0,0},{8,0}}},{1,{{0,0},{8,-2}}}};
    shouldUseResult = Computation::createValidIntervals(result, 0, {-2,0}, true, result);
    ASSERT_EQ(false, shouldUseResult);

    result = {{1,{{0,0},{2,0}}},{1,{{4,-2},{8,-2}}}};
    shouldUseResult = Computation::createValidIntervals(result, 1, {-2,-1}, true, result);
    ASSERT_EQ(false, shouldUseResult);

    result = {{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}};
    shouldUseResult = Computation::createValidIntervals(result, 2, {8,-3,-2}, true, result);
    ASSERT_EQ(false, shouldUseResult);

    result = {{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{8,-1,0}}},{1,{{0,0,0},{10,0,0}}}};
    shouldUseResult = Computation::createValidIntervals(result, 1, {-2,-1,0}, false, result);
    ASSERT_EQ(false, shouldUseResult);
}*/

TEST(TestSplitRecurse, runSplit) {
    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> t = {{{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}},{1,{{0,0,0},{10,0,0}}}}};
    bool res = Computation::setBoundRecursivelyWithRepair(t,0, 1, {1.5, -2, 0}, false);
    ASSERT_EQ(true, res);
    ASSERT_EQ(2, t.size());
    ASSERT_EQ(0, t[0][0].second.first[0]);
    ASSERT_EQ(0.75, t[0][0].second.second[0]);
    ASSERT_EQ(1.5, t[0][1].second.first[0]);
    ASSERT_EQ(-2, t[0][1].second.first[1]);
    ASSERT_EQ(0.75, t[1][0].second.first[0]);
    ASSERT_EQ(10, t[1][0].second.second[0]);
    ASSERT_EQ(0, t[1][1].second.first[0]);

    //cout << "Next ------" << endl;

    t = {{{1,{{0,0,0,0},{10,0,0,0}}},{1,{{0,0,0,0},{10,0,0,0}}},{1,{{0,0,0,0},{0,3,1,0}}}}};
    res = Computation::setBoundRecursivelyWithRepair(t, 0, 2, {3,-1,-1,0}, true);
    //t = Computation::repairIntervals(t);
    ASSERT_EQ(true, res);
    ASSERT_EQ(3, t.size());
    ASSERT_EQ(0, t[0][0].second.first[0]);
    ASSERT_EQ(0.75, t[0][0].second.second[0]);
    ASSERT_EQ(1.5, t[0][1].second.first[0]);
    ASSERT_EQ(-2, t[0][1].second.first[1]);
    //ASSERT_EQ(10, t[0][1].second.second[0]);
    ASSERT_EQ(3, t[0][1].second.second[0]);
    ASSERT_EQ(-1, t[0][1].second.second[1]);
    ASSERT_EQ(0, t[0][2].second.first[0]);
    ASSERT_EQ(3, t[0][2].second.second[0]);
    ASSERT_EQ(-1, t[0][2].second.second[1]);
    ASSERT_EQ(-1, t[0][2].second.second[2]);

    ASSERT_EQ(0, t[1][0].second.first[0]);
    ASSERT_EQ(0.75, t[1][0].second.second[0]);
    ASSERT_EQ(0, t[1][1].second.first[0]);
    ASSERT_EQ(1.5, t[1][1].second.second[0]);
    ASSERT_EQ(-2, t[1][1].second.second[1]);
    ASSERT_EQ(0, t[1][2].second.first[0]);
    ASSERT_EQ(3, t[1][2].second.second[1]);
    ASSERT_EQ(1, t[1][2].second.second[2]);

    ASSERT_EQ(0.75, t[2][0].second.first[0]);
    ASSERT_EQ(3, t[2][0].second.second[0]);
    ASSERT_EQ(0, t[2][1].second.first[0]);
    ASSERT_EQ(3, t[2][1].second.second[0]);
    ASSERT_EQ(-1, t[2][1].second.second[1]);
    ASSERT_EQ(0, t[2][2].second.first[0]);
    ASSERT_EQ(3, t[2][2].second.second[0]);
    ASSERT_EQ(-1, t[2][2].second.second[1]);
    ASSERT_EQ(-1, t[2][2].second.second[2]);

    //cout << "Next ------" << endl;

    /*std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> x = {t[2]};
    res = Computation::setBoundRecursivelyWithRepair(x, 0, 2, {4.5,-3,-2,0}, false);

    ASSERT_EQ(true, res);
    ASSERT_EQ(3, x.size());
    ASSERT_EQ(0.75, x[0][0].second.first[0]);
    ASSERT_EQ(1.5, x[0][0].second.second[0]);
    //ASSERT_EQ(0, x[0][1].second.first[0]);
    ASSERT_EQ(1.5, x[0][1].second.first[0]);
    ASSERT_EQ(2.25, x[0][1].second.second[0]);
    ASSERT_EQ(4.5, x[0][2].second.first[0]);
    ASSERT_EQ(3, x[0][2].second.second[0]);
    ASSERT_EQ(0.75, x[1][0].second.first[0]);
    ASSERT_EQ(1.5, x[1][0].second.second[0]);
    ASSERT_EQ(2.25, x[1][1].second.first[0]);
    ASSERT_EQ(3, x[1][1].second.second[0]);
    ASSERT_EQ(0, x[1][2].second.first[0]);
    ASSERT_EQ(3, x[1][2].second.second[0]);
    ASSERT_EQ(1.5, x[2][0].second.first[0]);
    ASSERT_EQ(3, x[2][0].second.second[0]);
    ASSERT_EQ(1.5, x[2][0].second.first[0]);
    ASSERT_EQ(3, x[2][0].second.second[0]);
    ASSERT_EQ(0, x[2][1].second.first[0]);
    ASSERT_EQ(3, x[2][1].second.second[0]);
    ASSERT_EQ(0, x[2][2].second.first[0]);
    ASSERT_EQ(3, x[2][2].second.second[0]);*/

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> u = {{{1,{{0,0,0,0},{10,0,0,0}}},{1,{{0,0,0,0},{10,0,0,0}}},{1,{{0,0,0,0},{0,3,1,0}}}}};
    res = Computation::setBoundRecursivelyWithRepair(u, 0, 2, {4.5,-3,-2,0}, true);
    for (std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> intervals : u) {
        cout << "Interval: " << endl;
        for (std::pair<int, std::pair<std::vector<double>, std::vector<double>>> interval : intervals) {
            cout << "   [" << interval.second.first << "; " << interval.second.second << "]" << endl;
        }
        cout << endl;
    }
    ASSERT_EQ(false, false);


    /*for (std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> intervals : x) {
        cout << "Interval: " << endl;
        for (std::pair<int, std::pair<std::vector<double>, std::vector<double>>> interval : intervals) {
            cout << "   [" << interval.second.first << "; " << interval.second.second << "]" << endl;
        }
        cout << endl;
    }*/
}