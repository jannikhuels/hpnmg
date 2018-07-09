#include "gtest/gtest.h"
#include "STDiagram.h"
#include "exceptions/STDiagramExceptions.h"
#include "datastructures/Event.h"

using namespace hpnmg;

class STDiagramRegionTest : public ::testing::Test {
protected:
    Region baseRegion;
    Event sourceEvent;
    vector<Event> destinationEvents;

    STDiagramRegionTest()  {
        baseRegion = STDiagram::createBaseRegion(2,10);
 
        sourceEvent.setGeneralDependencies(vector<double> {0});
        sourceEvent.setTime(0);

        Event stochasticEvent;
        stochasticEvent.setGeneralDependencies(vector<double> {1});
        stochasticEvent.setTime(0);

        Event deterministicEvent;
        deterministicEvent.setGeneralDependencies(vector<double> {0});
        deterministicEvent.setTime(5);

        destinationEvents = vector<Event>(2);
        destinationEvents = {stochasticEvent,deterministicEvent};
    }
};

TEST(STDiagramTest, DimensionTest)
{
    Region baseRegion = STDiagram::createBaseRegion(2,10);
    ASSERT_EQ(baseRegion.dimension(), 2);

    Region highDimRegion = STDiagram::createBaseRegion(24,10);
    ASSERT_EQ(highDimRegion.dimension(), 24);
}

TEST(STDiagramTest, VertexTest)
{
    Region baseRegion = STDiagram::createBaseRegion(2,10);
    ASSERT_EQ(baseRegion.vertices().size(), 4);
}

//TEST(STDiagramTest, DimensionErrorTest)
//{
//    try {
//        Region baseRegion = STDiagram::createBaseRegion(1,10);
//        FAIL() << "Dimension of 1 not allowed. (Dimension must be >= 2)";
//    } catch (IllegalDimensionException e) {
//        SUCCEED();
//    }
//}

TEST(STDiagramTest, MaxTimeErrorTest)
{
    try {
        Region baseRegion = STDiagram::createBaseRegion(2,0);
        FAIL() << "MaxTime of 0 not allowed. (MaxTime must be >= 1)";
    } catch (IllegalMaxTimeException e) {
        SUCCEED();
    }    
}

TEST(STDiagramTest, RegionFromVerticesTest) {
    Point<double> p1({0,0});
    Point<double> p2({5,5});
    Point<double> p3({20,0});
    Region region = STDiagram::createRegionForVertices({p1,p2,p3});

    ASSERT_EQ(region.contains(Point<double>({0,0})), true);
    ASSERT_EQ(region.contains(Point<double>({1,1})), true);
    ASSERT_EQ(region.contains(Point<double>({21,0})), false);
}

TEST_F(STDiagramRegionTest, CreateRegionTest)
{
    Region region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    ASSERT_EQ(region.dimension(), 2);
    
    vector_t<double> containedPoint = vector_t<double>::Zero(2);
    containedPoint << 3,1;
    ASSERT_EQ(region.contains(Point<double>(containedPoint)), true);

    containedPoint << 0,0;
    ASSERT_EQ(region.contains(Point<double>(containedPoint)), true);
    containedPoint << 5,5;
    ASSERT_EQ(region.contains(Point<double>(containedPoint)), true);
    containedPoint << 10,5;
    ASSERT_EQ(region.contains(Point<double>(containedPoint)), true);
    containedPoint << 10,0;
    ASSERT_EQ(region.contains(Point<double>(containedPoint)), true);

    vector_t<double> notContainedPoint = vector_t<double>::Zero(2);
    notContainedPoint << 2,8;
    ASSERT_EQ(region.contains(Point<double>(notContainedPoint)), false);
}

TEST_F(STDiagramRegionTest, CreateRegionOnlySourceEvent)
{
    Event testEvent;
    testEvent.setGeneralDependencies(vector<double> {1});
    testEvent.setTime(0);

    std::vector<Event> noDestEvents(0);
    Region region = STDiagram::createRegion(baseRegion, testEvent, noDestEvents);

    ASSERT_EQ(region.contains(Point<double>{5,2}),false);
    ASSERT_EQ(region.contains(Point<double>{5,6}),true);
}

TEST_F(STDiagramRegionTest, CreateRegionErrorTest)
{
    //TODO: Check if we need this. Changed from isValidEvent to makeValidEvent. We should not have invalid events now.
    Event notInitialized;
    try {
        Region region = STDiagram::createRegion(baseRegion, notInitialized, destinationEvents);
        SUCCEED();
    } catch(IllegalArgumentException e) {
        FAIL();
    }

    vector<Event> notInitializedEvents;
    notInitializedEvents.push_back(notInitialized);
    try {
        Region region = STDiagram::createRegion(baseRegion, sourceEvent, notInitializedEvents);
        SUCCEED();
    } catch(IllegalArgumentException e) {
        FAIL();
    }
}

TEST(STDiagramIntersection, IntersectionTest2D)
{
    Region baseRegion = STDiagram::createBaseRegion(2,10);
    Event event0 = Event(EventType::Immediate, vector<double> {0}, 0);
    Event event1 = Event(EventType::Immediate, vector<double> {1}, 0);
    Event event2 = Event(EventType::Immediate, vector<double> {0}, 5);
    Event event3 = Event(EventType::Immediate, vector<double> {0}, 7);

    Region region1 = STDiagram::createRegion(baseRegion, event0, std::vector<Event>{event1,event2});
    Region region2 = STDiagram::createRegion(baseRegion, event2, std::vector<Event>{event3});

    ASSERT_EQ(STDiagram::regionIsCandidateForTime(2,region1,2).first,true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(6,region1,2).first,false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(1,3), region1, 2).first, true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,6), region1, 2).first, true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(7,10), region1, 2).first, false);

    ASSERT_EQ(STDiagram::regionIsCandidateForTime(2,region2,2).first,false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(6,region2,2).first,true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(7,region2,2).first,true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(1,3), region2, 2).first, false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(5,7), region2, 2).first, true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(6,8), region2, 2).first, true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,6), region2, 2).first, true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,3), region2, 2).first, false);    
}

TEST(STDiagramIntersection, IntersectionTest3D)
{
    Region baseRegion = STDiagram::createBaseRegion(3,10);
    Event event0 = Event(EventType::Immediate, vector<double> {0,0}, 0);
    Event event1 = Event(EventType::Immediate, vector<double> {1,0}, 0);
    Event event2 = Event(EventType::Immediate, vector<double> {0,0}, 5);
    Event event3 = Event(EventType::Immediate, vector<double> {0,0}, 7);

    Region region1 = STDiagram::createRegion(baseRegion, event0, std::vector<Event>{event1,event2});
    Region region2 = STDiagram::createRegion(baseRegion, event2, std::vector<Event>{event3});

    ASSERT_EQ(STDiagram::regionIsCandidateForTime(2,region1,3).first,true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(6,region1,3).first,false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(1,3), region1, 3).first, true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,6), region1, 3).first, true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(7,10), region1, 3).first, false);  
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(2,region2,3).first,false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(6,region2,3).first,true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(7,region2,3).first,true); 
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(5,7), region2, 3).first, true);  
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(7,10), region2, 3).first, true);  
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(4,5), region2, 3).first, true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,3), region2, 3).first, false);   
}

TEST(STDiagramIntersection, IntersectionErrorTest)
{
    Region baseRegion = STDiagram::createBaseRegion(2,10);

    try {
        STDiagram::regionIsCandidateForTime(-1,baseRegion,3);
        FAIL() << " Time is less than 0.";
    } catch (IllegalArgumentException e) {
        SUCCEED();
    }

    try {
        STDiagram::regionIsCandidateForTime(2,baseRegion,-1);
        FAIL() << " Dimension is less than 2.";
    } catch (IllegalArgumentException e) {
        SUCCEED();
    }

    try {
        STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(3,2),baseRegion,-1);
        FAIL() << " Start time < Stop time.";
    } catch (IllegalArgumentException e) {
        SUCCEED();
    }

    try {
        STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(-1,2),baseRegion,-1);
        FAIL() << " Start time is negative.";
    } catch (IllegalArgumentException e) {
        SUCCEED();
    }
}

TEST_F(STDiagramRegionTest, TestRegionNoEvent) {
    Event event = Event(EventType::Immediate, vector<double> {0}, 5);
    Region regionNoEvent = STDiagram::createRegionNoEvent(baseRegion, event, std::vector<double>{2}, std::vector<double>{5});
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(4,regionNoEvent,2).first,false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(5,regionNoEvent,2).first,true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(10,regionNoEvent,2).first,true);

    Region baseRegion3D = STDiagram::createBaseRegion(3,10);
    Event event3D = Event(EventType::Immediate, vector<double> {0,0}, 5);
    Region regionNoEvent3D = STDiagram::createRegionNoEvent(baseRegion3D, event3D, std::vector<double>{2,2}, std::vector<double>{5,5});
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(4,regionNoEvent,2).first,false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(5,regionNoEvent,2).first,true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(10,regionNoEvent,2).first,true);
}

TEST_F(STDiagramRegionTest, TestRegionContinuousLevelIntersection) {
    Region resultNoDep = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{0,0}, 1, 5);
    ASSERT_EQ(resultNoDep.contains(Point<double>{2,2}), true);
    ASSERT_EQ(resultNoDep.contains(Point<double>{2,6}), false);
    
    
    Region resultPosDep = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{-1,2}, 1, 5);
    ASSERT_EQ(resultPosDep.contains(Point<double>{1,2}), true);
    ASSERT_EQ(resultPosDep.contains(Point<double>{1,5}), false);

    Region resultNegDep = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{1,2}, 1, 5);
    ASSERT_EQ(resultNegDep.contains(Point<double>{2,2}), true);
    ASSERT_EQ(resultNegDep.contains(Point<double>{2,6}), false);
}

TEST_F(STDiagramRegionTest, TestRegionContinuousLevelIntersectionZeroDrift) {
    Region resultNoDep = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{-1,2}, 0, 5);
    ASSERT_EQ(resultNoDep.contains(Point<double>{2,2}), true);
    ASSERT_EQ(resultNoDep.contains(Point<double>{6,2}), false);

    Region result = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{0,0}, 1, 4);
    ASSERT_EQ(result.contains(Point<double>{2,2}), true);
    ASSERT_EQ(result.contains(Point<double>{3,2}), true);
    ASSERT_EQ(result.contains(Point<double>{1,5}), false);

    Region resultNeg = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{0,6}, -1, 4);
    ASSERT_EQ(resultNeg.contains(Point<double>{1,1}), false);
    ASSERT_EQ(resultNeg.contains(Point<double>{1,3}), true);

    Region region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    Region intersectedRegion = STDiagram::intersectRegionForContinuousLevel(region, std::vector<double>{0,2}, 0, 3);
    ASSERT_EQ(intersectedRegion.empty(), true);
}

TEST_F(STDiagramRegionTest, TestTimeRegionCreation) {
    Region region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    Region timeRegion = STDiagram::createTimeRegion(region,5,2);

    ASSERT_EQ(timeRegion.contains(Point<double>{5,5}), true);
    ASSERT_EQ(timeRegion.contains(Point<double>{4,5}), false);
    ASSERT_EQ(timeRegion.contains(Point<double>{6,5}), true);
    ASSERT_EQ(timeRegion.contains(Point<double>{5,6}), false);
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
    ASSERT_EQ(resPartR[0][0].lower(),4);
    ASSERT_EQ(resPartR[0][0].upper(),5);

    // Cut Interval on the right
    std::vector<Intervals> resPartL = STDiagram::differenceOfIntervals({{i1}},{{i2}});
    ASSERT_EQ(resPartL.size(), 1);
    ASSERT_EQ(resPartL[0][0].lower(),2);
    ASSERT_EQ(resPartL[0][0].upper(),3);

    // Cut Interval in the middle
    std::vector<Intervals> resTwo = STDiagram::differenceOfIntervals({{i3}},{{i1}});
    ASSERT_EQ(resTwo.size(), 2);
    ASSERT_EQ(resTwo[0][0].lower(), 1);
    ASSERT_EQ(resTwo[0][0].upper(), 2);
    ASSERT_EQ(resTwo[1][0].lower(), 4);
    ASSERT_EQ(resTwo[1][0].upper(), 6);

    // No Cut
    std::vector<Intervals> resPartNo = STDiagram::differenceOfIntervals({{i1}},{{i4}});
    ASSERT_EQ(resPartNo.size(), 1);
    ASSERT_EQ(resPartNo[0][0].lower(),2);
    ASSERT_EQ(resPartNo[0][0].upper(),4);

    // Empty Cut
    std::vector<Intervals> resEmpty = STDiagram::differenceOfIntervals({{i1}},{{i1}});
    ASSERT_EQ(resEmpty.size(), 0);
    //ASSERT_EQ(resEmpty[0][0].lower(),0);
    //ASSERT_EQ(resEmpty[0][0].upper(),0);

    // Complete 
    std::vector<Intervals> resComplete = STDiagram::differenceOfIntervals({{i1}},{{i3}});
    ASSERT_EQ(resComplete.size(),0);
    //ASSERT_EQ(resComplete[0][0].lower(),0);
    //ASSERT_EQ(resComplete[0][0].upper(),0);

    std::vector<Intervals> res = STDiagram::differenceOfIntervals({{i5}},{{i6}});
    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0][0].lower(),0);
    ASSERT_EQ(res[0][0].upper(),3);
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
    ASSERT_EQ(resPart[0][0].lower(),3);
    ASSERT_EQ(resPart[0][0].upper(),4);  
    ASSERT_EQ(resPart[0][1].lower(),7);
    ASSERT_EQ(resPart[0][1].upper(),8);   

    std::vector<Intervals> resEmpty = STDiagram::differenceOfIntervals({{i1,i4}},{{i3,i2}});
    ASSERT_EQ(resEmpty.size(), 0);
    //ASSERT_EQ(resEmpty[0][0].lower(),3);
    //ASSERT_EQ(resEmpty[0][0].upper(),4);  
    //ASSERT_EQ(resEmpty[0][1].lower(),0);
    //ASSERT_EQ(resEmpty[0][1].upper(),0); 

    std::vector<Intervals> resTwo = STDiagram::differenceOfIntervals({{i1,i5}},{{i3,i6}});
    ASSERT_EQ(resTwo.size(), 2);
    ASSERT_EQ(resTwo[0].size(), 2);
    ASSERT_EQ(resTwo[1].size(), 2);
    ASSERT_EQ(resTwo[0][0].lower(),3);
    ASSERT_EQ(resTwo[0][0].upper(),4);
    ASSERT_EQ(resTwo[0][1].lower(),7);
    ASSERT_EQ(resTwo[0][1].upper(),8);
    ASSERT_EQ(resTwo[1][0].lower(),3);
    ASSERT_EQ(resTwo[1][0].upper(),4);
    ASSERT_EQ(resTwo[1][1].lower(),9);
    ASSERT_EQ(resTwo[1][1].upper(),10);
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
    ASSERT_EQ(res[0][0].lower(),2);
    ASSERT_EQ(res[0][0].upper(),4);
    ASSERT_EQ(res[0][1].lower(),7);
    ASSERT_EQ(res[0][1].upper(),8);
    ASSERT_EQ(res[0][2].lower(),1);
    ASSERT_EQ(res[0][2].upper(),2);
    ASSERT_EQ(res[1][0].lower(),2);
    ASSERT_EQ(res[1][0].upper(),4);
    ASSERT_EQ(res[1][1].lower(),9);
    ASSERT_EQ(res[1][1].upper(),10);
    ASSERT_EQ(res[1][2].lower(),1);
    ASSERT_EQ(res[1][2].upper(),2);
    ASSERT_EQ(res[2][0].lower(),2);
    ASSERT_EQ(res[2][0].upper(),4);
    ASSERT_EQ(res[2][1].lower(),7);
    ASSERT_EQ(res[2][1].upper(),8);
    ASSERT_EQ(res[2][2].lower(),3);
    ASSERT_EQ(res[2][2].upper(),5);
    ASSERT_EQ(res[3][0].lower(),2);
    ASSERT_EQ(res[3][0].upper(),4);
    ASSERT_EQ(res[3][1].lower(),9);
    ASSERT_EQ(res[3][1].upper(),10);
    ASSERT_EQ(res[3][2].lower(),3);
    ASSERT_EQ(res[3][2].upper(),5);
}

TEST(STDiagramIntervalTest, TestIntersection1D) 
{
    carl::Interval<double> i1(2,4);
    carl::Interval<double> i2(1,5);
    carl::Interval<double> i3(1,3);
    carl::Interval<double> i4(7,8);

    std::vector<Intervals> resPart = STDiagram::intersectionOfIntervals({{i1}},{{i3}});
    ASSERT_EQ(resPart.size(), 1);
    ASSERT_EQ(resPart[0][0].lower(),2);
    ASSERT_EQ(resPart[0][0].upper(),3);   

    std::vector<Intervals> resComplete = STDiagram::intersectionOfIntervals({{i1}},{{i2}});
    ASSERT_EQ(resComplete.size(), 1);
    ASSERT_EQ(resComplete[0][0].lower(),2);
    ASSERT_EQ(resComplete[0][0].upper(),4);

    std::vector<Intervals> resEmpty = STDiagram::intersectionOfIntervals({{i1}},{{i4}});
    ASSERT_EQ(resEmpty.size(),0);
    //ASSERT_EQ(resEmpty[0][0].lower(),0);
    //ASSERT_EQ(resEmpty[0][0].upper(),0);
}

TEST(STDiagramIntervalTest, TestIntersection2D)
{
    carl::Interval<double> i1(2,4);
    carl::Interval<double> i2(7,9);
    carl::Interval<double> i3(1,3);
    carl::Interval<double> i4(7,8);

    std::vector<Intervals> resPart = STDiagram::intersectionOfIntervals({{i1,i2}},{{i3,i4}});
    ASSERT_EQ(resPart.size(), 1);
    ASSERT_EQ(resPart[0][0].lower(),2);
    ASSERT_EQ(resPart[0][0].upper(),3);  
    ASSERT_EQ(resPart[0][1].lower(),7);
    ASSERT_EQ(resPart[0][1].upper(),8); 

    std::vector<Intervals> resEmpty = STDiagram::intersectionOfIntervals({{i1,i2}},{{i4,i4}});
    ASSERT_EQ(resEmpty.size(), 0);
    //ASSERT_EQ(resEmpty[0][0].lower(),0);
    //ASSERT_EQ(resEmpty[0][0].upper(),0);  
    //ASSERT_EQ(resEmpty[0][1].lower(),7);
    //ASSERT_EQ(resEmpty[0][1].upper(),8);
} 

TEST(STDiagramIntervalTest, TestUnion1D)
{
    carl::Interval<double> i1(2,4);
    carl::Interval<double> i2(1,5);
    carl::Interval<double> i3(1,3);
    carl::Interval<double> i4(7,8);

    std::vector<Intervals> resPart = STDiagram::unionOfIntervals({{i1}},{{i3}});
    ASSERT_EQ(resPart.size(), 1);
    ASSERT_EQ(resPart[0][0].lower(),1);
    ASSERT_EQ(resPart[0][0].upper(),4);   

    std::vector<Intervals> resComplete = STDiagram::unionOfIntervals({{i1}},{{i2}});
    ASSERT_EQ(resComplete.size(), 1);
    ASSERT_EQ(resComplete[0][0].lower(),1);
    ASSERT_EQ(resComplete[0][0].upper(),5);

    std::vector<Intervals> resEmpty = STDiagram::unionOfIntervals({{i1}},{{i4}});
    ASSERT_EQ(resEmpty.size(),2);
    ASSERT_EQ(resEmpty[0][0].lower(), 2);
    ASSERT_EQ(resEmpty[0][0].upper(), 4);
    ASSERT_EQ(resEmpty[1][0].lower(), 7);
    ASSERT_EQ(resEmpty[1][0].upper(), 8);

    std::vector<Intervals> resSame = STDiagram::unionOfIntervals({{i1}},{{i1}});
    ASSERT_EQ(resSame.size(),1);

    std::vector<Intervals> resOvelap = STDiagram::unionOfIntervals({{i3}},{{i2}});
    ASSERT_EQ(resSame.size(),1);
}

TEST(STDiagramIntervalTest, TestUnion2D) 
{
    carl::Interval<double> i1(1,3);
    carl::Interval<double> i2(2,4);
    carl::Interval<double> i3(1,7);
    carl::Interval<double> i4(5,6);

    std::vector<Intervals> dup = STDiagram::unionOfIntervals({{i1,i2}},{{i3,i4}});
    ASSERT_EQ(dup.size(),2);
    ASSERT_EQ(dup[0][0].lower(), 1);
    ASSERT_EQ(dup[0][0].upper(), 7);
    ASSERT_EQ(dup[0][1].lower(), 2);
    ASSERT_EQ(dup[0][1].upper(), 4);
    ASSERT_EQ(dup[1][0].lower(), 1);
    ASSERT_EQ(dup[1][0].upper(), 7);
    ASSERT_EQ(dup[1][1].lower(), 5);
    ASSERT_EQ(dup[1][1].upper(), 6);
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
    ASSERT_EQ(dup[0][0].lower(), 1);
    ASSERT_EQ(dup[0][0].upper(), 7);
    ASSERT_EQ(dup[0][1].lower(), 2);
    ASSERT_EQ(dup[0][1].upper(), 4);
    ASSERT_EQ(dup[0][2].lower(), 2);
    ASSERT_EQ(dup[0][2].upper(), 7);
    ASSERT_EQ(dup[1][0].lower(), 1);
    ASSERT_EQ(dup[1][0].upper(), 7);
    ASSERT_EQ(dup[1][1].lower(), 5);
    ASSERT_EQ(dup[1][1].upper(), 6);
    ASSERT_EQ(dup[1][2].lower(), 2);
    ASSERT_EQ(dup[1][2].upper(), 7);
    ASSERT_EQ(dup[2][0].lower(), 1);
    ASSERT_EQ(dup[2][0].upper(), 7);
    ASSERT_EQ(dup[2][1].lower(), 2);
    ASSERT_EQ(dup[2][1].upper(), 4);
    ASSERT_EQ(dup[2][2].lower(), 9);
    ASSERT_EQ(dup[2][2].upper(), 11);
    ASSERT_EQ(dup[3][0].lower(), 1);
    ASSERT_EQ(dup[3][0].upper(), 7);
    ASSERT_EQ(dup[3][1].lower(), 5);
    ASSERT_EQ(dup[3][1].upper(), 6);
    ASSERT_EQ(dup[3][2].lower(), 9);
    ASSERT_EQ(dup[3][2].upper(), 11);
}

TEST(STDiagramConstraintsTest, TestConstraints) {
    Point<double> p1({0,0});
    Point<double> p2({5,5});
    Point<double> p3({20,0});
    Point<double> p4({20,5});
    Region region = STDiagram::createRegionForVertices({p1,p2,p3,p4});
    std::vector<std::vector<double>> constraints = STDiagram::stochasticConstraints(region);

    ASSERT_EQ(constraints.size(),4);
    ASSERT_EQ(constraints[0].size(), 2);
    ASSERT_EQ(constraints[0][0], 0);
    ASSERT_EQ(constraints[0][1], -1);
    ASSERT_EQ(constraints[2][0], 5);
    ASSERT_EQ(constraints[2][1], 0);
}



/*TEST_F(STDiagramRegionTest, TestPrint)
{
    Region region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    vector<Region> regions{region};
    STDiagram::print(regions,false);
    SUCCEED();
}*/