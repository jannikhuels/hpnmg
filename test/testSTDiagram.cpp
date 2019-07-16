#include "gtest/gtest.h"
#include "STDiagram.h"
#include "exceptions/STDiagramExceptions.h"
#include "datastructures/Event.h"

using namespace hpnmg;

class STDiagramRegionTest : public ::testing::Test {
protected:
    STDPolytope<double> baseRegion;
    Event sourceEvent;
    vector<Event> destinationEvents;

    STDiagramRegionTest()  {
        baseRegion = STDiagram::createBaseRegion(2,10);
 
        sourceEvent.setGeneralDependencies(vector<double> {0});
        sourceEvent.setGeneralDependenciesNormed(vector<double> {0, 0});
        sourceEvent.setTime(0);

        Event stochasticEvent;
        stochasticEvent.setGeneralDependencies(vector<double> {1});
        stochasticEvent.setGeneralDependenciesNormed(vector<double> {0, 1});
        stochasticEvent.setTime(0);

        Event deterministicEvent;
        deterministicEvent.setGeneralDependencies(vector<double> {0});
        deterministicEvent.setGeneralDependenciesNormed(vector<double> {5, 0});
        deterministicEvent.setTime(5);

        destinationEvents = vector<Event>(2);
        destinationEvents = {stochasticEvent,deterministicEvent};
    }
};

TEST(STDiagramTest, DimensionTest)
{
    STDPolytope<double> baseRegion = STDiagram::createBaseRegion(2,10);
    EXPECT_EQ(baseRegion.dimension(), 2);

    STDPolytope<double> highDimRegion = STDiagram::createBaseRegion(24,10);
    EXPECT_EQ(highDimRegion.dimension(), 24);
}

TEST(STDiagramTest, VertexTest)
{
    STDPolytope<double> baseRegion = STDiagram::createBaseRegion(2,10);
    EXPECT_TRUE(baseRegion.contains(Point<double>{0, 0}));
}

TEST(STDiagramTest, MaxTimeErrorTest)
{
    try {
        STDPolytope<double> baseRegion = STDiagram::createBaseRegion(2,0);
        FAIL() << "MaxTime of 0 not allowed. (MaxTime must be >= 1)";
    } catch (IllegalMaxTimeException e) {
        SUCCEED();
    }    
}

TEST(STDiagramTest, RegionFromVerticesTest) {
    Point<double> p1({0,0});
    Point<double> p2({5,5});
    Point<double> p3({20,0});
    STDPolytope<double> region = STDiagram::createRegionForVertices({p1,p2,p3});

    EXPECT_EQ(region.contains(Point<double>({0,0})), true);
    EXPECT_EQ(region.contains(Point<double>({1,1})), true);
    EXPECT_EQ(region.contains(Point<double>({21,0})), false);
}

TEST_F(STDiagramRegionTest, CreateRegionTest)
{
    STDPolytope<double> region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    ASSERT_EQ(region.dimension(), 2);
    
    vector_t<double> containedPoint = vector_t<double>::Zero(2);
    containedPoint << 3,1;
    EXPECT_EQ(region.contains(Point<double>(containedPoint)), true);

    containedPoint << 0,0;
    EXPECT_EQ(region.contains(Point<double>(containedPoint)), true);
    containedPoint << 5,5;
    EXPECT_EQ(region.contains(Point<double>(containedPoint)), true);
    containedPoint << 10,5;
    EXPECT_EQ(region.contains(Point<double>(containedPoint)), true);
    containedPoint << 10,0;
    EXPECT_EQ(region.contains(Point<double>(containedPoint)), true);

    vector_t<double> notContainedPoint = vector_t<double>::Zero(2);
    notContainedPoint << 2,8;
    EXPECT_EQ(region.contains(Point<double>(notContainedPoint)), false);
}

TEST_F(STDiagramRegionTest, CreateRegionOnlySourceEvent)
{
    Event testEvent;
    testEvent.setGeneralDependencies(vector<double> {1});
    testEvent.setGeneralDependenciesNormed(vector<double> {0, 1});
    testEvent.setTime(0);

    std::vector<Event> noDestEvents(0);
    STDPolytope<double> region = STDiagram::createRegion(baseRegion, testEvent, noDestEvents);

    EXPECT_EQ(region.contains(Point<double>{5,2}),false);
    EXPECT_EQ(region.contains(Point<double>{5,6}),true);
}

TEST_F(STDiagramRegionTest, TestRegionContinuousLevelIntersection) {
    STDPolytope<double> resultNoDep = STDiagram::legacyIntersectRegionForContinuousLevel(baseRegion, std::vector<double>{0,0}, 1, 5);
    EXPECT_EQ(resultNoDep.contains(Point<double>{2,2}), true);
    EXPECT_EQ(resultNoDep.contains(Point<double>{2,6}), false);
    
    
    STDPolytope<double> resultPosDep = STDiagram::legacyIntersectRegionForContinuousLevel(baseRegion, std::vector<double>{-1,2}, 1, 5);
    EXPECT_EQ(resultPosDep.contains(Point<double>{1,2}), true);
    EXPECT_EQ(resultPosDep.contains(Point<double>{1,5}), false);

    STDPolytope<double> resultNegDep = STDiagram::legacyIntersectRegionForContinuousLevel(baseRegion, std::vector<double>{1,2}, 1, 5);
    EXPECT_EQ(resultNegDep.contains(Point<double>{2,2}), true);
    EXPECT_EQ(resultNegDep.contains(Point<double>{2,6}), false);
}

TEST_F(STDiagramRegionTest, TestRegionContinuousLevelIntersectionZeroDrift) {
    STDPolytope<double> resultNoDep = STDiagram::legacyIntersectRegionForContinuousLevel(baseRegion, std::vector<double>{-1,2}, 0, 5);
    EXPECT_EQ(resultNoDep.contains(Point<double>{2,2}), true);
    EXPECT_EQ(resultNoDep.contains(Point<double>{6,2}), false);

    STDPolytope<double> result = STDiagram::legacyIntersectRegionForContinuousLevel(baseRegion, std::vector<double>{0,0}, 1, 4);
    EXPECT_EQ(result.contains(Point<double>{2,2}), true);
    EXPECT_EQ(result.contains(Point<double>{3,2}), true);
    EXPECT_EQ(result.contains(Point<double>{1,5}), false);

    STDPolytope<double> resultNeg = STDiagram::legacyIntersectRegionForContinuousLevel(baseRegion, std::vector<double>{0,6}, -1, 4);
    EXPECT_EQ(resultNeg.contains(Point<double>{1,1}), false);
    EXPECT_EQ(resultNeg.contains(Point<double>{1,3}), true);

    STDPolytope<double> region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    STDPolytope<double> intersectedRegion = STDiagram::legacyIntersectRegionForContinuousLevel(region, std::vector<double>{0,2}, 0, 3);
    EXPECT_EQ(intersectedRegion.empty(), true);
}

TEST(STDiagramIntervalTest, TestDifference1D) 
{
    carl::Interval<double> i1(2,4);
    carl::Interval<double> i2(3,5);
    carl::Interval<double> i3(1,6);
    carl::Interval<double> i4(7,9);
    carl::Interval<double> i5(0,20);
    carl::Interval<double> i6(3,20);

    // Cut Interval on the left
    std::vector<Intervals> resPartR = STDiagram::differenceOfIntervals({{i2}},{{i1}});
    ASSERT_EQ(resPartR.size(), 1);
    EXPECT_EQ(resPartR[0][0].lower(),4);
    EXPECT_EQ(resPartR[0][0].upper(),5);

    // Cut Interval on the right
    std::vector<Intervals> resPartL = STDiagram::differenceOfIntervals({{i1}},{{i2}});
    ASSERT_EQ(resPartL.size(), 1);
    EXPECT_EQ(resPartL[0][0].lower(),2);
    EXPECT_EQ(resPartL[0][0].upper(),3);

    // Cut Interval in the middle
    std::vector<Intervals> resTwo = STDiagram::differenceOfIntervals({{i3}},{{i1}});
    ASSERT_EQ(resTwo.size(), 2);
    EXPECT_EQ(resTwo[0][0].lower(), 1);
    EXPECT_EQ(resTwo[0][0].upper(), 2);
    EXPECT_EQ(resTwo[1][0].lower(), 4);
    EXPECT_EQ(resTwo[1][0].upper(), 6);

    // No Cut
    std::vector<Intervals> resPartNo = STDiagram::differenceOfIntervals({{i1}},{{i4}});
    ASSERT_EQ(resPartNo.size(), 1);
    EXPECT_EQ(resPartNo[0][0].lower(),2);
    EXPECT_EQ(resPartNo[0][0].upper(),4);

    // Empty Cut
    std::vector<Intervals> resEmpty = STDiagram::differenceOfIntervals({{i1}},{{i1}});
    ASSERT_EQ(resEmpty.size(), 0);
    //EXPECT_EQ(resEmpty[0][0].lower(),0);
    //EXPECT_EQ(resEmpty[0][0].upper(),0);

    // Complete 
    std::vector<Intervals> resComplete = STDiagram::differenceOfIntervals({{i1}},{{i3}});
    ASSERT_EQ(resComplete.size(),0);
    //EXPECT_EQ(resComplete[0][0].lower(),0);
    //EXPECT_EQ(resComplete[0][0].upper(),0);

    std::vector<Intervals> res = STDiagram::differenceOfIntervals({{i5}},{{i6}});
    ASSERT_EQ(res.size(), 1);
    EXPECT_EQ(res[0][0].lower(),0);
    EXPECT_EQ(res[0][0].upper(),3);
}

TEST(STDiagramIntervalTest, TestDifference2D) 
{
    carl::Interval<double> i1(2,4);
    carl::Interval<double> i2(7,9);
    carl::Interval<double> i3(1,3);
    carl::Interval<double> i4(7,8);
    carl::Interval<double> i5(7,10);
    carl::Interval<double> i6(8,9);

    std::vector<Intervals> resPart = STDiagram::differenceOfIntervals({{i1,i2}},{{i3,i6}});
    ASSERT_EQ(resPart.size(), 1);
    EXPECT_EQ(resPart[0][0].lower(),3);
    EXPECT_EQ(resPart[0][0].upper(),4);
    EXPECT_EQ(resPart[0][1].lower(),7);
    EXPECT_EQ(resPart[0][1].upper(),8);

    std::vector<Intervals> resEmpty = STDiagram::differenceOfIntervals({{i1,i4}},{{i3,i2}});
    ASSERT_EQ(resEmpty.size(), 0);
    //EXPECT_EQ(resEmpty[0][0].lower(),3);
    //EXPECT_EQ(resEmpty[0][0].upper(),4);
    //EXPECT_EQ(resEmpty[0][1].lower(),0);
    //EXPECT_EQ(resEmpty[0][1].upper(),0);

    std::vector<Intervals> resTwo = STDiagram::differenceOfIntervals({{i1,i5}},{{i3,i6}});
    ASSERT_EQ(resTwo.size(), 2);
    ASSERT_EQ(resTwo[0].size(), 2);
    ASSERT_EQ(resTwo[1].size(), 2);
    EXPECT_EQ(resTwo[0][0].lower(),3);
    EXPECT_EQ(resTwo[0][0].upper(),4);
    EXPECT_EQ(resTwo[0][1].lower(),7);
    EXPECT_EQ(resTwo[0][1].upper(),8);
    EXPECT_EQ(resTwo[1][0].lower(),3);
    EXPECT_EQ(resTwo[1][0].upper(),4);
    EXPECT_EQ(resTwo[1][1].lower(),9);
    EXPECT_EQ(resTwo[1][1].upper(),10);
}

TEST(STDiagramIntervalTest, TestDifference3D)
{
    carl::Interval<double> i1(2,4);
    carl::Interval<double> i2(7,10);
    carl::Interval<double> i3(1,5);
    carl::Interval<double> i4(7,8);
    carl::Interval<double> i5(8,9);
    carl::Interval<double> i6(2,3);

    std::vector<Intervals> res = STDiagram::differenceOfIntervals({{i1,i2,i3}},{{i4,i5,i6}});
    ASSERT_EQ(res.size(), 4);
    EXPECT_EQ(res[0][0].lower(),2);
    EXPECT_EQ(res[0][0].upper(),4);
    EXPECT_EQ(res[0][1].lower(),7);
    EXPECT_EQ(res[0][1].upper(),8);
    EXPECT_EQ(res[0][2].lower(),1);
    EXPECT_EQ(res[0][2].upper(),2);
    EXPECT_EQ(res[1][0].lower(),2);
    EXPECT_EQ(res[1][0].upper(),4);
    EXPECT_EQ(res[1][1].lower(),9);
    EXPECT_EQ(res[1][1].upper(),10);
    EXPECT_EQ(res[1][2].lower(),1);
    EXPECT_EQ(res[1][2].upper(),2);
    EXPECT_EQ(res[2][0].lower(),2);
    EXPECT_EQ(res[2][0].upper(),4);
    EXPECT_EQ(res[2][1].lower(),7);
    EXPECT_EQ(res[2][1].upper(),8);
    EXPECT_EQ(res[2][2].lower(),3);
    EXPECT_EQ(res[2][2].upper(),5);
    EXPECT_EQ(res[3][0].lower(),2);
    EXPECT_EQ(res[3][0].upper(),4);
    EXPECT_EQ(res[3][1].lower(),9);
    EXPECT_EQ(res[3][1].upper(),10);
    EXPECT_EQ(res[3][2].lower(),3);
    EXPECT_EQ(res[3][2].upper(),5);
}

TEST(STDiagramIntervalTest, TestIntersection1D) 
{
    carl::Interval<double> i1(2,4);
    carl::Interval<double> i2(1,5);
    carl::Interval<double> i3(1,3);
    carl::Interval<double> i4(7,8);

    std::vector<Intervals> resPart = STDiagram::intersectionOfIntervals({{i1}},{{i3}});
    ASSERT_EQ(resPart.size(), 1);
    EXPECT_EQ(resPart[0][0].lower(),2);
    EXPECT_EQ(resPart[0][0].upper(),3);

    std::vector<Intervals> resComplete = STDiagram::intersectionOfIntervals({{i1}},{{i2}});
    ASSERT_EQ(resComplete.size(), 1);
    EXPECT_EQ(resComplete[0][0].lower(),2);
    EXPECT_EQ(resComplete[0][0].upper(),4);

    std::vector<Intervals> resEmpty = STDiagram::intersectionOfIntervals({{i1}},{{i4}});
    ASSERT_EQ(resEmpty.size(),0);
    //EXPECT_EQ(resEmpty[0][0].lower(),0);
    //EXPECT_EQ(resEmpty[0][0].upper(),0);
}

TEST(STDiagramIntervalTest, TestIntersection2D)
{
    carl::Interval<double> i1(2,4);
    carl::Interval<double> i2(7,9);
    carl::Interval<double> i3(1,3);
    carl::Interval<double> i4(7,8);

    std::vector<Intervals> resPart = STDiagram::intersectionOfIntervals({{i1,i2}},{{i3,i4}});
    ASSERT_EQ(resPart.size(), 1);
    EXPECT_EQ(resPart[0][0].lower(),2);
    EXPECT_EQ(resPart[0][0].upper(),3);
    EXPECT_EQ(resPart[0][1].lower(),7);
    EXPECT_EQ(resPart[0][1].upper(),8);

    std::vector<Intervals> resEmpty = STDiagram::intersectionOfIntervals({{i1,i2}},{{i4,i4}});
    ASSERT_EQ(resEmpty.size(), 0);
    //EXPECT_EQ(resEmpty[0][0].lower(),0);
    //EXPECT_EQ(resEmpty[0][0].upper(),0);
    //EXPECT_EQ(resEmpty[0][1].lower(),7);
    //EXPECT_EQ(resEmpty[0][1].upper(),8);
} 

TEST(STDiagramIntervalTest, TestUnion1D)
{
    carl::Interval<double> i1(2,4);
    carl::Interval<double> i2(1,5);
    carl::Interval<double> i3(1,3);
    carl::Interval<double> i4(7,8);

    std::vector<Intervals> resPart = STDiagram::unionOfIntervals({{i1}},{{i3}});
    ASSERT_EQ(resPart.size(), 1);
    EXPECT_EQ(resPart[0][0].lower(),1);
    EXPECT_EQ(resPart[0][0].upper(),4);

    std::vector<Intervals> resComplete = STDiagram::unionOfIntervals({{i1}},{{i2}});
    ASSERT_EQ(resComplete.size(), 1);
    EXPECT_EQ(resComplete[0][0].lower(),1);
    EXPECT_EQ(resComplete[0][0].upper(),5);

    std::vector<Intervals> resEmpty = STDiagram::unionOfIntervals({{i1}},{{i4}});
    ASSERT_EQ(resEmpty.size(),2);
    EXPECT_EQ(resEmpty[0][0].lower(), 2);
    EXPECT_EQ(resEmpty[0][0].upper(), 4);
    EXPECT_EQ(resEmpty[1][0].lower(), 7);
    EXPECT_EQ(resEmpty[1][0].upper(), 8);

    std::vector<Intervals> resSame = STDiagram::unionOfIntervals({{i1}},{{i1}});
    EXPECT_EQ(resSame.size(),1);

    std::vector<Intervals> resOvelap = STDiagram::unionOfIntervals({{i3}},{{i2}});
    EXPECT_EQ(resSame.size(),1);
}

TEST(STDiagramIntervalTest, TestUnion2D) 
{
    carl::Interval<double> i1(1,3);
    carl::Interval<double> i2(2,4);
    carl::Interval<double> i3(1,7);
    carl::Interval<double> i4(5,6);

    std::vector<Intervals> dup = STDiagram::unionOfIntervals({{i1,i2}},{{i3,i4}});
    ASSERT_EQ(dup.size(),2);
    EXPECT_EQ(dup[0][0].lower(), 1);
    EXPECT_EQ(dup[0][0].upper(), 7);
    EXPECT_EQ(dup[0][1].lower(), 2);
    EXPECT_EQ(dup[0][1].upper(), 4);
    EXPECT_EQ(dup[1][0].lower(), 1);
    EXPECT_EQ(dup[1][0].upper(), 7);
    EXPECT_EQ(dup[1][1].lower(), 5);
    EXPECT_EQ(dup[1][1].upper(), 6);
}

TEST(STDiagramIntervalTest, TestUnion3D) 
{
    carl::Interval<double> i1(1,3);
    carl::Interval<double> i2(2,4);
    carl::Interval<double> i3(1,7);
    carl::Interval<double> i4(5,6);
    carl::Interval<double> i5(2,7);
    carl::Interval<double> i6(9,11);

    std::vector<Intervals> dup = STDiagram::unionOfIntervals({{i1,i2,i5}},{{i3,i4,i6}});
    ASSERT_EQ(dup.size(),4);
    EXPECT_EQ(dup[0][0].lower(), 1);
    EXPECT_EQ(dup[0][0].upper(), 7);
    EXPECT_EQ(dup[0][1].lower(), 2);
    EXPECT_EQ(dup[0][1].upper(), 4);
    EXPECT_EQ(dup[0][2].lower(), 2);
    EXPECT_EQ(dup[0][2].upper(), 7);
    EXPECT_EQ(dup[1][0].lower(), 1);
    EXPECT_EQ(dup[1][0].upper(), 7);
    EXPECT_EQ(dup[1][1].lower(), 5);
    EXPECT_EQ(dup[1][1].upper(), 6);
    EXPECT_EQ(dup[1][2].lower(), 2);
    EXPECT_EQ(dup[1][2].upper(), 7);
    EXPECT_EQ(dup[2][0].lower(), 1);
    EXPECT_EQ(dup[2][0].upper(), 7);
    EXPECT_EQ(dup[2][1].lower(), 2);
    EXPECT_EQ(dup[2][1].upper(), 4);
    EXPECT_EQ(dup[2][2].lower(), 9);
    EXPECT_EQ(dup[2][2].upper(), 11);
    EXPECT_EQ(dup[3][0].lower(), 1);
    EXPECT_EQ(dup[3][0].upper(), 7);
    EXPECT_EQ(dup[3][1].lower(), 5);
    EXPECT_EQ(dup[3][1].upper(), 6);
    EXPECT_EQ(dup[3][2].lower(), 9);
    EXPECT_EQ(dup[3][2].upper(), 11);
}
