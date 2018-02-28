#include "gtest/gtest.h"
#include "ReadHybridPetrinet.h"
#include "ParseHybridPetrinet.h"

using namespace hpnmg;

TEST(ParseHybridPetrinet, InitialLocation)
{
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/example.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode().getParametricLocation();
    ASSERT_EQ(initState.getDiscreteMarking().size(), 2);
    ASSERT_EQ(initState.getDiscreteMarking()[0], 1);
    ASSERT_EQ(initState.getDiscreteMarking()[1], 1);
    ASSERT_EQ(initState.getContinuousMarking().size(), 1);
    ASSERT_EQ(initState.getContinuousMarking()[0][0], 0.0);
    ASSERT_EQ(initState.getGeneralIntervalBoundRight().size(), 1);
    ASSERT_EQ(initState.getGeneralIntervalBoundRight()[0], 20);
    ASSERT_EQ(initState.getDrift()[0], 1);
    ASSERT_EQ(initState.getDeterministicClock().size(), 1);
}

TEST(ParseHybridPetrinet, ImmediateTransitions)
{
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/exampleImmediate.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode();

    std::vector<int> expectedRootMarking = {2,0,0,1};
    ASSERT_EQ(expectedRootMarking, initState.getParametricLocation().getDiscreteMarking());

    auto children = plt->getChildNodes(initState);

    std::vector<int> expectedFirstChildMarking = {1,1,0,0};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Immediate, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFirstChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(0, children[0].getParametricLocation().getSourceEvent().getTime());

    children = plt->getChildNodes(children[0]);

    std::vector<int> expectedSecondChildMarking1 = {0,2,0,0};
    std::vector<int> expectedSecondChildMarking2 = {0,1,1,0};

    ASSERT_EQ(2, children.size());
    ASSERT_EQ(expectedSecondChildMarking1, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_NEAR(1.0 / 3, children[0].getParametricLocation().getConflictProbability(), 0.0001);
    ASSERT_EQ(expectedSecondChildMarking2, children[1].getParametricLocation().getDiscreteMarking());
    ASSERT_NEAR(2.0 / 3, children[1].getParametricLocation().getConflictProbability(), 0.0001);

    ASSERT_EQ(0, plt->getChildNodes(children[0]).size());
    ASSERT_EQ(0, plt->getChildNodes(children[1]).size());
}