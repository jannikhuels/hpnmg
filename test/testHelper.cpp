#include <gtest/gtest.h>
#include <hpnmg/ReadHybridPetrinet.h>
#include <hpnmg/ParseHybridPetrinet.h>
#include <hpnmg/STDiagram.h>
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
    Triangulation t(candidates[0]);
    std::vector<Region> triangles = t.getObjects();
    ASSERT_EQ(triangles.size(), 2);
    ASSERT_EQ(triangles[0].dimension(), 2);

    ASSERT_EQ(triangles[0].contains(Point<double>({0,0})), true);
    ASSERT_EQ(triangles[0].contains(Point<double>({20,0})), true);
    ASSERT_EQ(triangles[0].contains(Point<double>({20,5})), true);
    ASSERT_EQ(triangles[0].contains(Point<double>({6,5})), false);
    ASSERT_EQ(triangles[1].contains(Point<double>({6,5})), true);
    ASSERT_EQ(triangles[1].contains(Point<double>({20,5})), true);
    ASSERT_EQ(triangles[1].contains(Point<double>({5,5})), true);
    ASSERT_EQ(triangles[1].contains(Point<double>({6,20})), false);
}

TEST(TriangulationTest, Test3D) {
    // Similar to having 2 RVs
    Point<double> p1({0,0,0});
    Point<double> p2({4,0,0});
    Point<double> p3({4,4,0});
    Point<double> p4({0,4,0});
    Point<double> p5({0,0,4});
    Point<double> p6({4,0,4});
    Point<double> p7({0,4,4});
    Point<double> p8({4,4,4});

    Region region = STDiagram::createRegionForVertices({p1,p2,p3,p4,p5,p6,p7,p8});
    Triangulation t(region);

    std::vector<Region> triangles = t.getObjects();

    ASSERT_EQ(triangles.size(), 6);
    ASSERT_EQ(triangles[0].vertices().size(), 4);
    ASSERT_EQ(triangles[0].dimension(), 3);
}

TEST(TriangulationTest, TestIsTriangle3D) {
    Point<double> p11({0,0});
    Point<double> p22({5,5});
    Point<double> p33({4,4});

    std::vector<vector_t<double>> dir = STDiagram::directionVectorsFromHspVectors({p11.rawCoordinates(),p22.rawCoordinates()});
    vector_t<double> planeNormal = Halfspace<double>::computePlaneNormal(dir);
    double planeOffset = Halfspace<double>::computePlaneOffset(-planeNormal, p11);
    Halfspace<double> h1(planeNormal, planeOffset);

    Region r = STDiagram::createBaseRegion(2,10);
    Region l(r);
    l.insert(h1);

    dir = STDiagram::directionVectorsFromHspVectors({p22.rawCoordinates(),p33.rawCoordinates()});
    planeNormal = Halfspace<double>::computePlaneNormal(dir);


    STDiagram::print({l}, true);

}

TEST(TriangulationTest, TestVerticalSplit2D) {
    //Having 1RV
    Point<double> p1({0,0});
    Point<double> p2({5,5});
    Point<double> p3({4,6});

    Region region = STDiagram::createRegionForVertices({p1,p2,p3});
    Triangulation t(region);

    std::vector<Region> triangles = t.getSplitVertical();

    ASSERT_EQ(triangles.size(), 1);
    ASSERT_EQ(triangles[0].contains(Point<double>({6,6})), false);
    ASSERT_EQ(triangles[0].contains(p3), true);
    ASSERT_EQ(triangles[0].contains(p1), true);
    ASSERT_EQ(triangles[0].contains(p2), true);
}

TEST(TriangulationTest, TestVerticalSplit3DLow) {
    //Having 2RV
    Point<double> p1({0,0,0});
    Point<double> p2({5,5,0});
    Point<double> p3({4,6,0});
    Point<double> p4({0,0,5});

    Region region = STDiagram::createRegionForVertices({p1,p2,p3,p4});
    Triangulation t(region);

    std::vector<Region> triangles = t.getSplitVertical();

    ASSERT_EQ(triangles.size(), 2);
    ASSERT_EQ(triangles[0].dimension(), 3);
    ASSERT_EQ(triangles[0].contains(Point<double>({4,4,4})), true);
    ASSERT_EQ(triangles[0].contains(p3), true);
    ASSERT_EQ(triangles[0].contains(p1), true);
    ASSERT_EQ(triangles[0].contains(p4), true);
    ASSERT_EQ(triangles[0].contains(p2), false);

    ASSERT_EQ(triangles[1].contains(Point<double>({4,4,4})), true);
    ASSERT_EQ(triangles[1].contains(p3), true);
    ASSERT_EQ(triangles[1].contains(p1), false);
    ASSERT_EQ(triangles[1].contains(p2), true);
    ASSERT_EQ(triangles[1].contains(p4), true);
}

/*TEST(TriangulationTest, TestVerticalSplit3DUp) {
    //Having 1RV
    Point<double> p1({0,0});
    Point<double> p2({5,5});
    Point<double> p3({4,3});

    Region region = STDiagram::createRegionForVertices({p1,p2,p3});
    Triangulation t(region);

    std::vector<Region> triangles = t.getSplitVertical();

    ASSERT_EQ(triangles.size(), 2);
    ASSERT_EQ(triangles[0].contains(Point<double>({4,4})), true);
    ASSERT_EQ(triangles[0].contains(p3), true);
    ASSERT_EQ(triangles[0].contains(p1), true);
    ASSERT_EQ(triangles[0].contains(p2), false);

    ASSERT_EQ(triangles[1].contains(Point<double>({4,4})), true);
    ASSERT_EQ(triangles[1].contains(p3), true);
    ASSERT_EQ(triangles[1].contains(p1), false);
    ASSERT_EQ(triangles[1].contains(p2), true);
}*/

