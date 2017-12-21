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
        stochasticEvent.setGeneralDependencies(vector<double> {-1});
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

TEST(STDiagramTest, DimensionErrorTest)
{
    try {
        Region baseRegion = STDiagram::createBaseRegion(1,10);
        FAIL() << "Dimension of 1 not allowed. (Dimension must be >= 2)";
    } catch (IllegalDimensionException e) {
        SUCCEED();
    }    
}

TEST(STDiagramTest, MaxTimeErrorTest)
{
    try {
        Region baseRegion = STDiagram::createBaseRegion(2,0);
        FAIL() << "MaxTime of 0 not allowed. (MaxTime must be >= 1)";
    } catch (IllegalMaxTimeException e) {
        SUCCEED();
    }    
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
    testEvent.setGeneralDependencies(vector<double> {-1});
    testEvent.setTime(0);

    std::vector<Event> noDestEvents(0);
    Region region = STDiagram::createRegion(baseRegion, testEvent, noDestEvents);

    ASSERT_EQ(region.contains(Point<double>{5,2}),false);
    ASSERT_EQ(region.contains(Point<double>{5,6}),true);
}

TEST_F(STDiagramRegionTest, CreateRegionErrorTest)
{
    Event notInitialized;
    try {
        Region region = STDiagram::createRegion(baseRegion, notInitialized, destinationEvents);
        FAIL();
    } catch(IllegalArgumentException e) {
        SUCCEED();
    }

    vector<Event> notInitializedEvents;
    notInitializedEvents.push_back(notInitialized);
    try {
        Region region = STDiagram::createRegion(baseRegion, sourceEvent, notInitializedEvents);
        FAIL();
    } catch(IllegalArgumentException e) {
        SUCCEED();
    }
}

TEST(STDiagramIntersection, IntersectionTest2D)
{
    Region baseRegion = STDiagram::createBaseRegion(2,10);
    Event event0 = Event(EventType::Immediate, vector<double> {0}, 0);
    Event event1 = Event(EventType::Immediate, vector<double> {-1}, 0);
    Event event2 = Event(EventType::Immediate, vector<double> {0}, 5);
    Event event3 = Event(EventType::Immediate, vector<double> {0}, 7);

    Region region1 = STDiagram::createRegion(baseRegion, event0, std::vector<Event>{event1,event2});
    Region region2 = STDiagram::createRegion(baseRegion, event2, std::vector<Event>{event3});

    ASSERT_EQ(STDiagram::regionIsCandidateForTime(2,region1,2),true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(6,region1,2),false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(1,3), region1, 2), true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,6), region1, 2), true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(7,10), region1, 2), false);

    ASSERT_EQ(STDiagram::regionIsCandidateForTime(2,region2,2),false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(6,region2,2),true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(7,region2,2),true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(1,3), region2, 2), false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(5,7), region2, 2), true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(6,8), region2, 2), true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,6), region2, 2), true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,3), region2, 2), false);    
}

TEST(STDiagramIntersection, IntersectionTest3D)
{
    Region baseRegion = STDiagram::createBaseRegion(3,10);
    Event event0 = Event(EventType::Immediate, vector<double> {0,0}, 0);
    Event event1 = Event(EventType::Immediate, vector<double> {-1,0}, 0);
    Event event2 = Event(EventType::Immediate, vector<double> {0,0}, 5);
    Event event3 = Event(EventType::Immediate, vector<double> {0,0}, 7);

    Region region1 = STDiagram::createRegion(baseRegion, event0, std::vector<Event>{event1,event2});
    Region region2 = STDiagram::createRegion(baseRegion, event2, std::vector<Event>{event3});

    ASSERT_EQ(STDiagram::regionIsCandidateForTime(2,region1,3),true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(6,region1,3),false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(1,3), region1, 3), true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,6), region1, 3), true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(7,10), region1, 3), false);  
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(2,region2,3),false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(6,region2,3),true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(7,region2,3),true); 
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(5,7), region2, 3), true);  
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(7,10), region2, 3), true);  
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(4,5), region2, 3), true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTimeInterval(std::pair<double,double>(2,3), region2, 3), false);   
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
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(4,regionNoEvent,2),false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(5,regionNoEvent,2),true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(10,regionNoEvent,2),true);

    Region baseRegion3D = STDiagram::createBaseRegion(3,10);
    Event event3D = Event(EventType::Immediate, vector<double> {0,0}, 5);
    Region regionNoEvent3D = STDiagram::createRegionNoEvent(baseRegion3D, event3D, std::vector<double>{2,2}, std::vector<double>{5,5});
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(4,regionNoEvent,2),false);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(5,regionNoEvent,2),true);
    ASSERT_EQ(STDiagram::regionIsCandidateForTime(10,regionNoEvent,2),true);
}

TEST_F(STDiagramRegionTest, TestRegionContinuousLevelIntersection) {
    Region resultNoDep = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{0,0}, 1, 5);
    ASSERT_EQ(resultNoDep.contains(Point<double>{2,2}), true);
    ASSERT_EQ(resultNoDep.contains(Point<double>{2,6}), false);
    
    
    Region resultPosDep = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{1,2}, 1, 5);
    ASSERT_EQ(resultPosDep.contains(Point<double>{1,2}), true);
    ASSERT_EQ(resultPosDep.contains(Point<double>{1,5}), false);

    Region resultNegDep = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{-1,2}, 1, 5);
    ASSERT_EQ(resultNegDep.contains(Point<double>{2,2}), true);
    ASSERT_EQ(resultNegDep.contains(Point<double>{2,6}), false);
}

TEST_F(STDiagramRegionTest, TestRegionContinuousLevelIntersectionZeroDrift) {
    Region resultNoDep = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{1,2}, 0, 5);
    ASSERT_EQ(resultNoDep.contains(Point<double>{2,2}), true);
    ASSERT_EQ(resultNoDep.contains(Point<double>{6,2}), false);

    Region result = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{0,0}, 1, 4);
    ASSERT_EQ(result.contains(Point<double>{2,2}), true);
    ASSERT_EQ(result.contains(Point<double>{3,2}), true);
    ASSERT_EQ(result.contains(Point<double>{1,5}), false);

    Region resultNeg = STDiagram::intersectRegionForContinuousLevel(baseRegion, std::vector<double>{0,6}, -1, 4);
    ASSERT_EQ(resultNeg.contains(Point<double>{1,1}), false);
    ASSERT_EQ(resultNeg.contains(Point<double>{1,3}), true);

    try {
        Region region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
        Region intersectedRegion = STDiagram::intersectRegionForContinuousLevel(region, std::vector<double>{0,2}, 0, 3);
        FAIL() << " Intersection Arguments are illegal.";
    } catch (IllegalIntersectionLevelException e) {
        SUCCEED();
    }
}

TEST_F(STDiagramRegionTest, TestTimeRegionCreation) {
    Region region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    Region timeRegion = STDiagram::createTimeRegion(region,5,2);

    ASSERT_EQ(timeRegion.contains(Point<double>{5,5}), true);
    ASSERT_EQ(timeRegion.contains(Point<double>{4,5}), false);
    ASSERT_EQ(timeRegion.contains(Point<double>{6,5}), true);
    ASSERT_EQ(timeRegion.contains(Point<double>{5,6}), false);
}

/*TEST_F(STDiagramRegionTest, TestPrint)
{
    Region region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    vector<Region> regions{region};
    STDiagram::print(regions,false);
    SUCCEED();
}*/