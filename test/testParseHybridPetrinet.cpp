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
    ASSERT_EQ(initState.getGeneralIntervalBoundRight()[0][0][0], 20);
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


TEST(ParseHybridPetrinet, RateAdaption) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/exampleRateAdaption.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode();
    ParametricLocation initLocation = initState.getParametricLocation();

    std::vector<std::vector<double>> expectedContRootMarking = {{0},{0},{0},{0},{0}};
    std::vector<double> expectedRootDrift = {0,1,2,1,0};
    ASSERT_EQ(1, plt->getChildNodes(initState).size());
    ASSERT_EQ(expectedContRootMarking, initLocation.getContinuousMarking());
    ASSERT_EQ(expectedRootDrift, initLocation.getDrift());

    auto nextState = plt->getChildNodes(initState)[0];
    ParametricLocation nextLocation = nextState.getParametricLocation();

    std::vector<std::vector<double>> expectedContMarking1 = {{0},{0.5},{1},{0.5},{0}};
    std::vector<double> expectedDrift1 = {0,1,0,2,1};
//    ASSERT_EQ(2, plt->getChildNodes(nextState).size());
    ASSERT_EQ(0.5, nextState.getParametricLocation().getSourceEvent().getTime());
    ASSERT_EQ(expectedContMarking1, nextLocation.getContinuousMarking());
//    ASSERT_EQ(expectedDrift1, nextLocation.getDrift()); todo: does not work correct
}

TEST(ParseHybridPetrinet, BoundarysMainExample) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/example.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode().getParametricLocation();

    // in paper L=0
    vector<vector<vector<double>>> expectedLeftBound {{{0}}};
    vector<vector<vector<double>>> expectedRightBound {{{20}}};
    ASSERT_EQ(expectedLeftBound, initState.getGeneralIntervalBoundLeft());
    ASSERT_EQ(expectedRightBound, initState.getGeneralIntervalBoundRight());
    ASSERT_EQ(plt->getChildNodes(plt->getRootNode()).size(), 2);

    for (ParametricLocationTree::Node childNode : plt->getChildNodes(plt->getRootNode())) {
        ParametricLocation location = childNode.getParametricLocation();
        if (location.getSourceEvent().getEventType() == EventType::General) {
            // in paper L=1
            expectedLeftBound = {{{0},{0, 0}}};
            expectedRightBound = {{{5},{20, 0}}};
            ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
            ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
            ASSERT_EQ(plt->getChildNodes(childNode).size(), 2);

            for (ParametricLocationTree::Node childChildNode : plt->getChildNodes(childNode)) {
                location = childChildNode.getParametricLocation();
                if (location.getSourceEvent().getEventType() == EventType::Continuous) {
                    // in paper L=2
                    expectedLeftBound = {{{0},{0, 0}}};
                    expectedRightBound = {{{2.5},{20, 0}}};
                    ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                    ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                    ASSERT_EQ(plt->getChildNodes(childChildNode).size(), 1);

                    childNode = plt->getChildNodes(childChildNode)[0];
                    location = childNode.getParametricLocation();
                    ASSERT_EQ(EventType::Timed, location.getSourceEvent().getEventType());
                    // in paper L=3
                    expectedLeftBound = {{{0},{0, 0}}};
                    expectedRightBound = {{{2.5},{20, 0}}};
                    ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                    ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                    ASSERT_EQ(plt->getChildNodes(childNode).size(), 0);
                } else {
                    ASSERT_EQ(EventType::Timed, location.getSourceEvent().getEventType());
                    // in paper L=4
                    expectedLeftBound = {{{2.5},{0, 0}}};
                    expectedRightBound = {{{5},{20, 0}}};
                    ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                    ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                    ASSERT_EQ(plt->getChildNodes(childChildNode).size(), 0);
                }
            }
        } else {
            ASSERT_EQ(EventType::Timed, location.getSourceEvent().getEventType());

            // in paper L=5
            expectedLeftBound = {{{5}}};
            expectedRightBound = {{{20}}};
            ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
            ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
            ASSERT_EQ(plt->getChildNodes(childNode).size(), 2);

            for (ParametricLocationTree::Node childChildNode : plt->getChildNodes(childNode)) {
                location = childChildNode.getParametricLocation();
                if (location.getSourceEvent().getEventType() == EventType::General) {
                    // in paper L=6
                    expectedLeftBound = {{{5},{0, 0}}};
                    expectedRightBound = {{{7.5},{20, 0}}};
                    ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                    ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                    ASSERT_EQ(plt->getChildNodes(childChildNode).size(), 0);
                } else {
                    ASSERT_EQ(EventType::Continuous, location.getSourceEvent().getEventType());
                    // in paper L=7
                    expectedLeftBound = {{{7.5}}};
                    expectedRightBound = {{{20}}};
                    ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                    ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                    ASSERT_EQ(plt->getChildNodes(childChildNode).size(), 1);

                    childNode = plt->getChildNodes(childChildNode)[0];
                    location = childNode.getParametricLocation();
                    ASSERT_EQ(EventType::General, location.getSourceEvent().getEventType());
                    // in paper L=8
                    expectedLeftBound = {{{7.5},{0, 0}}};
                    expectedRightBound = {{{20},{20, 0}}};
                    ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                    ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                    ASSERT_EQ(plt->getChildNodes(childNode).size(), 0);
                }
            }
        }
    }
}