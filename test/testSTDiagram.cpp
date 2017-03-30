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
        baseRegion = STDiagram::getBaseRegion(2,10);
 
        sourceEvent.setGeneralDependencies(vector<Number> {0});
        sourceEvent.setTime(0);

        Event stochasticEvent;
        stochasticEvent.setGeneralDependencies(vector<Number> {-1});
        stochasticEvent.setTime(0);

        Event deterministicEvent;
        deterministicEvent.setGeneralDependencies(vector<Number> {0});
        deterministicEvent.setTime(5);

        destinationEvents = vector<Event>(2);
        destinationEvents = {stochasticEvent,deterministicEvent};
    }
};

TEST(STDiagramTest, DimensionTest)
{
    Region baseRegion = STDiagram::getBaseRegion(2,10);
    ASSERT_EQ(baseRegion.dimension(), 2);

    Region highDimRegion = STDiagram::getBaseRegion(24,10);
    ASSERT_EQ(highDimRegion.dimension(), 24);
}

TEST(STDiagramTest, VertexTest)
{
    Region baseRegion = STDiagram::getBaseRegion(2,10);
    ASSERT_EQ(baseRegion.vertices().size(), 4);
}

TEST(STDiagramTest, DimensionErrorTest)
{
    try {
        Region baseRegion = STDiagram::getBaseRegion(1,10);
        FAIL() << "Dimension of 1 not allowed. (Dimension must be >= 2)";
    } catch (IllegalDimensionException e) {
        SUCCEED();
    }    
}

TEST(STDiagramTest, MaxTimeErrorTest)
{
    try {
        Region baseRegion = STDiagram::getBaseRegion(2,0);
        FAIL() << "MaxTime of 0 not allowed. (MaxTime must be >= 1)";
    } catch (IllegalMaxTimeException e) {
        SUCCEED();
    }    
}

TEST_F(STDiagramRegionTest, CreateRegionTest)
{
    Region region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    ASSERT_EQ(region.dimension(), 2);
    
    vector_t<Number> containedPoint = vector_t<Number>::Zero(2);
    containedPoint << 3,1;
    ASSERT_EQ(region.contains(Point<Number>(containedPoint)), true);

    containedPoint << 0,0;
    ASSERT_EQ(region.contains(Point<Number>(containedPoint)), true);
    containedPoint << 5,5;
    ASSERT_EQ(region.contains(Point<Number>(containedPoint)), true);
    containedPoint << 10,5;
    ASSERT_EQ(region.contains(Point<Number>(containedPoint)), true);
    containedPoint << 10,0;
    ASSERT_EQ(region.contains(Point<Number>(containedPoint)), true);

    vector_t<Number> notContainedPoint = vector_t<Number>::Zero(2);
    notContainedPoint << 2,8;
    ASSERT_EQ(region.contains(Point<Number>(notContainedPoint)), false);
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

TEST_F(STDiagramRegionTest, TestPrint)
{
    Region region = STDiagram::createRegion(baseRegion, sourceEvent, destinationEvents);
    vector<Region> regions{region};
    STDiagram::print(regions,false);
    SUCCEED();
}