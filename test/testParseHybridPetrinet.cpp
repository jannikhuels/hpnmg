#include <hpnmg/PLTWriter.h>
#include "gtest/gtest.h"
#include "ReadHybridPetrinet.h"
#include "ParseHybridPetrinet.h"

using namespace hpnmg;

TEST(ParseHybridPetrinet, InitialLocation)
{
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("../../test/example.xml");
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
    auto hybridPetrinet = reader->readHybridPetrinet("exampleImmediate.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode();
    auto writer = new PLTWriter();
    writer->writePLT(plt, 20);

    std::vector<int> expectedRootMarking = {2,0,0,1};
    ASSERT_EQ(expectedRootMarking, initState.getParametricLocation().getDiscreteMarking());

    auto children = plt->getChildNodes(initState);

    std::vector<int> expectedFirstChildMarking = {1,1,0,1};
    ASSERT_EQ(2, children.size());
    ASSERT_EQ(EventType::Immediate, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFirstChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(0, children[0].getParametricLocation().getSourceEvent().getTime());

    children = plt->getChildNodes(children[0]);

    std::vector<int> expectedSecondChildMarking1 = {0,2,0,1};
    std::vector<int> expectedSecondChildMarking2 = {0,1,1,1};

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
    auto hybridPetrinet = reader->readHybridPetrinet("exampleRateAdaption.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode();
    ParametricLocation initLocation = initState.getParametricLocation();

    std::vector<std::vector<double>> expectedContRootMarking = {{0},{0},{0},{0},{0}};
    std::vector<double> expectedRootDrift = {0,0,2,1,1};
    ASSERT_EQ(1, plt->getChildNodes(initState).size());
    ASSERT_EQ(expectedContRootMarking, initLocation.getContinuousMarking());
    ASSERT_EQ(expectedRootDrift, initLocation.getDrift());

    auto nextState = plt->getChildNodes(initState)[0];
    ParametricLocation nextLocation = nextState.getParametricLocation();

    std::vector<std::vector<double>> expectedContMarking1 = {{0},{0},{1},{0.5},{0.5}};
    std::vector<double> expectedDrift1 = {0,1,0,2,1};
//    ASSERT_EQ(2, plt->getChildNodes(nextState).size());
    ASSERT_EQ(0.5, nextState.getParametricLocation().getSourceEvent().getTime());
    //    ASSERT_EQ(expect/home/annabell/src/carl/build/resources/lib/edContMarking1, nextLocation.getContinuousMarking());
//    ASSERT_EQ(expectedDrift1, nextLocation.getDrift()); todo: does not work correct
}

TEST(ParseHybridPetrinet, BoundarysMainExample) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("example.xml");
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

TEST(ParseHybridPetrinet, Example2General) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("example2general.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode().getParametricLocation();

    // in example L=1
    vector<vector<vector<double>>> expectedLeftBound{{{0}},{{0}}};
    vector<vector<vector<double>>> expectedRightBound{{{20}},{{20}}};
    ASSERT_EQ(expectedLeftBound, initState.getGeneralIntervalBoundLeft());
    ASSERT_EQ(expectedRightBound, initState.getGeneralIntervalBoundRight());
    ASSERT_EQ(plt->getChildNodes(plt->getRootNode()).size(), 3);
    vector<vector<double>> expectedContMarking {{0}};
    vector<int> expectedDiscMarking {1, 1, 1};
    vector<double> expectedDrift {1};
    ASSERT_EQ(expectedContMarking, initState.getContinuousMarking());
    ASSERT_EQ(expectedDiscMarking, initState.getDiscreteMarking());
    ASSERT_EQ(expectedDrift, initState.getDrift());
    vector<vector<double>> expectedDetClocks {{0}};
    vector<vector<double>> expectedGenClocks {{0}, {0}};
    vector<double> expectedGeneralDependencies {};
    ASSERT_EQ(0, initState.getSourceEvent().getTime());
    ASSERT_EQ(expectedGeneralDependencies, initState.getSourceEvent().getGeneralDependencies());
    ASSERT_EQ(expectedDetClocks, initState.getDeterministicClock());
    ASSERT_EQ(expectedGenClocks, initState.getGeneralClock());

    for (ParametricLocationTree::Node childNode : plt->getChildNodes(plt->getRootNode())) {
        ParametricLocation location = childNode.getParametricLocation();
        if (location.getSourceEvent().getEventType() == EventType::General) {
            vector<int> wantedDiscreteMarking {0,1,1};
            if (location.getDiscreteMarking() == wantedDiscreteMarking) {
                // in example L=2
                expectedLeftBound = {{{0}, {0, 0}}, {{0, 1}}};
                expectedRightBound = {{{8}, {20, 0}}, {{20, 0}}};
                ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                ASSERT_EQ(plt->getChildNodes(childNode).size(), 4);
                expectedContMarking = {{0, 1}};
                expectedDiscMarking = {0, 1, 1};
                expectedDrift = {2};
                ASSERT_EQ(expectedContMarking, location.getContinuousMarking());
                ASSERT_EQ(expectedDiscMarking, location.getDiscreteMarking());
                ASSERT_EQ(expectedDrift, location.getDrift());
                expectedDetClocks = {{0, 1}};
                expectedGenClocks = {{0, 0}, {0, 1}};
                expectedGeneralDependencies = {1};
                ASSERT_EQ(0, location.getSourceEvent().getTime());
                ASSERT_EQ(expectedGeneralDependencies, location.getSourceEvent().getGeneralDependencies());
                ASSERT_EQ(expectedDetClocks, location.getDeterministicClock());
                ASSERT_EQ(expectedGenClocks, location.getGeneralClock());

                vector<vector<vector<double>>> expectedLeftBound1 {{{0}, {0, 0}},{{0, 1}, {0, 0, 0}}};
                vector<vector<vector<double>>> expectedRightBound1 {{{6}, {20, 0}},{{5, 0.5}, {20, 0, 0}}};
                vector<vector<vector<double>>> expectedLeftBound2 {{{6}, {0, 0}},{{0, 1}, {0, 0, 0}}};
                vector<vector<vector<double>>> expectedRightBound2 {{{8}, {20, 0}},{{8, 0}, {20, 0, 0}}};
                ParametricLocationTree::Node node = plt->getChildNodes(childNode)[0];
                location = node.getParametricLocation();
                ParametricLocationTree::Node node5 = node;
                ParametricLocationTree::Node node6 = node;
                ParametricLocation location5 = location;
                ParametricLocation location6 = location;
                if (location.getGeneralIntervalBoundLeft() == expectedLeftBound1) {
                    node6 = plt->getChildNodes(childNode)[1];
                    location6 = node6.getParametricLocation();
                } else {
                    node5 = plt->getChildNodes(childNode)[1];
                    location5 = node5.getParametricLocation();
                }

                // in example L=5
                ASSERT_EQ(EventType::General, location5.getSourceEvent().getEventType());
                ASSERT_EQ(expectedLeftBound1, location5.getGeneralIntervalBoundLeft());
                ASSERT_EQ(expectedRightBound1, location5.getGeneralIntervalBoundRight());
                ASSERT_EQ(2, plt->getChildNodes(node5).size());
                expectedContMarking = {{0, -1, 2}};
                expectedDiscMarking = {0, 0, 1};
                expectedDrift = {1};
                ASSERT_EQ(expectedContMarking, location5.getContinuousMarking());
                ASSERT_EQ(expectedDiscMarking, location5.getDiscreteMarking());
                ASSERT_EQ(expectedDrift, location5.getDrift());
                expectedDetClocks = {{0, 0, 1}};
                expectedGenClocks = {{0, 0, 0}, {0, 0, 0}};
                expectedGeneralDependencies = {0, 1};
                ASSERT_EQ(0, location5.getSourceEvent().getTime());
                ASSERT_EQ(expectedGeneralDependencies, location5.getSourceEvent().getGeneralDependencies());
                ASSERT_EQ(expectedDetClocks, location5.getDeterministicClock());
                ASSERT_EQ(expectedGenClocks, location5.getGeneralClock());

                expectedLeftBound1 = {{{0}, {0, 0}},{{0, 1}, {0, 0, 0}}};
                expectedRightBound1 = {{{6}, {20, 0}},{{5, 0.5}, {20, 0, 0}}};
                for (ParametricLocationTree::Node childChildNode : plt->getChildNodes(node5)) {
                    location = childChildNode.getParametricLocation();
                    if (location.getSourceEvent().getEventType() == EventType::Timed) {
                        // in example L=9
                        expectedGeneralDependencies = {0, 0};
                        ASSERT_EQ(8, location.getSourceEvent().getTime());
                        ASSERT_EQ(expectedGeneralDependencies, location.getSourceEvent().getGeneralDependencies());
                        expectedLeftBound = {{{0}, {0, 0}},{{0, 1}, {0, 0, 0}}};
                        expectedRightBound = {{{6}, {20, 0}},{{2, 1}, {20, 0, 0}}};
                        ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                        ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                        ASSERT_EQ(0, plt->getChildNodes(childChildNode).size());
                        expectedContMarking = {{8, -1, 1}};
                        expectedDiscMarking = {0, 0, 0};
                        expectedDrift = {0};
                        ASSERT_EQ(expectedContMarking, location.getContinuousMarking());
                        ASSERT_EQ(expectedDiscMarking, location.getDiscreteMarking());
                        ASSERT_EQ(expectedDrift, location.getDrift());
                        expectedDetClocks = {{0, 0, 0}};
                        expectedGenClocks = {{0, 0, 0}, {0, 0, 0}};
                        expectedGeneralDependencies = {0, 0};
                        ASSERT_EQ(8, location.getSourceEvent().getTime());
                        ASSERT_EQ(expectedGeneralDependencies, location.getSourceEvent().getGeneralDependencies());
                        ASSERT_EQ(expectedDetClocks, location.getDeterministicClock());
                        ASSERT_EQ(expectedGenClocks, location.getGeneralClock());
                    } else {
                        // in example L=10
                        ASSERT_EQ(EventType::Continuous, location.getSourceEvent().getEventType());
                        expectedGeneralDependencies = {1, -1};
                        ASSERT_EQ(10, location.getSourceEvent().getTime());
                        ASSERT_EQ(expectedGeneralDependencies, location.getSourceEvent().getGeneralDependencies());
                        expectedLeftBound = {{{0}, {0, 0}},{{2, 1}, {0, 0, 0}}};
                        expectedRightBound = {{{6}, {20, 0}},{{5, 0.5}, {20, 0, 0}}};
                        ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                        ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                        ASSERT_EQ(1, plt->getChildNodes(childChildNode).size());
                        expectedContMarking = {{10, 0, 0}};
                        expectedDiscMarking = {0, 0, 1};
                        expectedDrift = {0};
                        ASSERT_EQ(expectedContMarking, location.getContinuousMarking());
                        ASSERT_EQ(expectedDiscMarking, location.getDiscreteMarking());
                        ASSERT_EQ(expectedDrift, location.getDrift());
                        expectedDetClocks = {{10, 1, -1}};
                        expectedGenClocks = {{0, 0, 0}, {0, 0, 0}};
                        expectedGeneralDependencies = {1, -1};
                        ASSERT_EQ(10, location.getSourceEvent().getTime());
                        ASSERT_EQ(expectedGeneralDependencies, location.getSourceEvent().getGeneralDependencies());
                        ASSERT_EQ(expectedDetClocks, location.getDeterministicClock());
                        ASSERT_EQ(expectedGenClocks, location.getGeneralClock());
                    }
                }

                // in example L=6
                ASSERT_EQ(EventType::General, location6.getSourceEvent().getEventType());
                ASSERT_EQ(expectedLeftBound2, location6.getGeneralIntervalBoundLeft());
                ASSERT_EQ(expectedRightBound2, location6.getGeneralIntervalBoundRight());
                ASSERT_EQ(2, plt->getChildNodes(node6).size());
                expectedContMarking = {{0, -1, 2}};
                expectedDiscMarking = {0, 0, 1};
                expectedDrift = {1};
                ASSERT_EQ(expectedContMarking, location6.getContinuousMarking());
                ASSERT_EQ(expectedDiscMarking, location6.getDiscreteMarking());
                ASSERT_EQ(expectedDrift, location6.getDrift());
                expectedDetClocks = {{0, 0, 1}};
                expectedGenClocks = {{0, 0, 0}, {0, 0, 0}};
                expectedGeneralDependencies = {0, 1};
                ASSERT_EQ(0, location6.getSourceEvent().getTime());
                ASSERT_EQ(expectedGeneralDependencies, location6.getSourceEvent().getGeneralDependencies());
                ASSERT_EQ(expectedDetClocks, location6.getDeterministicClock());
                ASSERT_EQ(expectedGenClocks, location6.getGeneralClock());

                expectedLeftBound1 = {{{6}, {0, 0}},{{0, 1}, {0, 0, 0}}};
                expectedRightBound1 = {{{8}, {20, 0}},{{8, 0}, {20, 0, 0}}};
                for (ParametricLocationTree::Node childChildNode : plt->getChildNodes(node6)) {
                    location = childChildNode.getParametricLocation();
                    if (location.getSourceEvent().getEventType() == EventType::Timed) {
                        // in example L=11
                        expectedGeneralDependencies = {0, 0};
                        ASSERT_EQ(8, location.getSourceEvent().getTime());
                        ASSERT_EQ(expectedGeneralDependencies, location.getSourceEvent().getGeneralDependencies());
                        expectedLeftBound = {{{6}, {0, 0}},{{0, 1}, {0, 0, 0}}};
                        expectedRightBound = {{{8}, {20, 0}},{{2, 1}, {20, 0, 0}}};
                        ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                        ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                        ASSERT_EQ(0, plt->getChildNodes(childChildNode).size());
                        expectedContMarking = {{8, -1, 1}};
                        expectedDiscMarking = {0, 0, 0};
                        expectedDrift = {0};
                        ASSERT_EQ(expectedContMarking, location.getContinuousMarking());
                        ASSERT_EQ(expectedDiscMarking, location.getDiscreteMarking());
                        ASSERT_EQ(expectedDrift, location.getDrift());
                        expectedDetClocks = {{0, 0, 0}};
                        expectedGenClocks = {{0, 0, 0}, {0, 0, 0}};
                        expectedGeneralDependencies = {0, 0};
                        ASSERT_EQ(8, location.getSourceEvent().getTime());
                        ASSERT_EQ(expectedGeneralDependencies, location.getSourceEvent().getGeneralDependencies());
                        ASSERT_EQ(expectedDetClocks, location.getDeterministicClock());
                        ASSERT_EQ(expectedGenClocks, location.getGeneralClock());
                    } else {
                        // in example L=12
                        ASSERT_EQ(EventType::Continuous, location.getSourceEvent().getEventType());
                        expectedGeneralDependencies = {1, -1};
                        ASSERT_EQ(10, location.getSourceEvent().getTime());
                        ASSERT_EQ(expectedGeneralDependencies, location.getSourceEvent().getGeneralDependencies());
                        expectedLeftBound = {{{6}, {0, 0}},{{2, 1}, {0, 0, 0}}};
                        expectedRightBound = {{{8}, {20, 0}},{{8, 0}, {20, 0, 0}}};
                        ASSERT_EQ(expectedLeftBound, location.getGeneralIntervalBoundLeft());
                        ASSERT_EQ(expectedRightBound, location.getGeneralIntervalBoundRight());
                        ASSERT_EQ(1, plt->getChildNodes(childChildNode).size());
                        expectedContMarking = {{10, 0, 0}};
                        expectedDiscMarking = {0, 0, 1};
                        expectedDrift = {0};
                        ASSERT_EQ(expectedContMarking, location.getContinuousMarking());
                        ASSERT_EQ(expectedDiscMarking, location.getDiscreteMarking());
                        ASSERT_EQ(expectedDrift, location.getDrift());
                        expectedDetClocks = {{10, 1, -1}};
                        expectedGenClocks = {{0, 0, 0}, {0, 0, 0}};
                        expectedGeneralDependencies = {1, -1};
                        ASSERT_EQ(10, location.getSourceEvent().getTime());
                        ASSERT_EQ(expectedGeneralDependencies, location.getSourceEvent().getGeneralDependencies());
                        ASSERT_EQ(expectedDetClocks, location.getDeterministicClock());
                        ASSERT_EQ(expectedGenClocks, location.getGeneralClock());
                    }
                }
            } else {
                // in example L=3
                vector<int> expectedDiscreteMarking {1,0,1};
                ASSERT_EQ(expectedDiscreteMarking, location.getDiscreteMarking());
            }
        } else {
            // in example L=4
            ASSERT_EQ(EventType::Timed, location.getSourceEvent().getEventType());
        }
    }
}

TEST(ParseHybridPetrinet, GuardArcContinuous)
{
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("guard_continuous.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode();

    std::vector<int> expectedRootMarking = {1,0};
    ASSERT_EQ(expectedRootMarking, initState.getParametricLocation().getDiscreteMarking());

    auto children = plt->getChildNodes(initState);

    std::vector<int> expectedFirstChildMarking = {1,0};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Continuous, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFirstChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(5, children[0].getParametricLocation().getSourceEvent().getTime());

    children= plt->getChildNodes(children[0]);

    std::vector<int> expectedSecondChildMarking = {0,1};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Immediate, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedSecondChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(5, children[0].getParametricLocation().getSourceEvent().getTime());
}

TEST(ParseHybridPetrinet, GuardArcContinuousNegation)
{
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("guard_continuous_negation.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode();

    std::vector<int> expectedRootMarking = {1,0};
    ASSERT_EQ(expectedRootMarking, initState.getParametricLocation().getDiscreteMarking());

    auto children = plt->getChildNodes(initState);

    std::vector<int> expectedFirstChildMarking = {1,0};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Continuous, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFirstChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(3, children[0].getParametricLocation().getSourceEvent().getTime());

    children= plt->getChildNodes(children[0]);

    std::vector<int> expectedSecondChildMarking = {0,1};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Immediate, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedSecondChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(3, children[0].getParametricLocation().getSourceEvent().getTime());
}

TEST(ParseHybridPetrinet, GuardArcFillLevelTest)
{
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("guard_fill_level_test.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode();

    std::vector<int> expectedRootMarking = {1,0,1,0};
    ASSERT_EQ(expectedRootMarking, initState.getParametricLocation().getDiscreteMarking());

    auto children = plt->getChildNodes(initState);

    std::vector<int> expectedFirstChildMarking = {1,0,1,0};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Continuous, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFirstChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(5, children[0].getParametricLocation().getSourceEvent().getTime());

    children= plt->getChildNodes(children[0]);

    std::vector<int> expectedSecondChildMarking = {0,1,1,0};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Immediate, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedSecondChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(5, children[0].getParametricLocation().getSourceEvent().getTime());

    children= plt->getChildNodes(children[0]);

    std::vector<int> expectedThirdChildMarking = {0,1,0,1};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Timed, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedThirdChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(6, children[0].getParametricLocation().getSourceEvent().getTime());

    children= plt->getChildNodes(children[0]);

    std::vector<int> expectedFourthChildMarking = {0,1,0,1};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Continuous, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFourthChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(7, children[0].getParametricLocation().getSourceEvent().getTime());

    children= plt->getChildNodes(children[0]);

    std::vector<int> expectedFifthChildMarking = {1,0,0,1};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Immediate, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFifthChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(7, children[0].getParametricLocation().getSourceEvent().getTime());
}

TEST(ParseHybridPetrinet, GuardArcContinuousConflict)
{
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("guard_continuous_conflict.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto initState = plt->getRootNode();

    std::vector<int> expectedRootMarking = {1,0,1,0};
    ASSERT_EQ(expectedRootMarking, initState.getParametricLocation().getDiscreteMarking());

    auto children = plt->getChildNodes(initState);

    std::vector<int> expectedFirstChildMarking = {1,0,1,0};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Continuous, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFirstChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(5, children[0].getParametricLocation().getSourceEvent().getTime());
}

TEST(ParseHybridPetrinet, GuardArcEmptyTest)
{
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("guard_empty_test.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 10);
    auto initState = plt->getRootNode();

    std::vector<int> expectedRootMarking = {1,0,1,0};
    ASSERT_EQ(expectedRootMarking, initState.getParametricLocation().getDiscreteMarking());

    auto children = plt->getChildNodes(initState);

    std::vector<int> expectedFirstChildMarking = {1,0,1,0};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Continuous, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFirstChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_NEAR(4.99, children[0].getParametricLocation().getSourceEvent().getTime(), 0.01);

    children= plt->getChildNodes(children[0]);
}

TEST(ParseHybridPetrinet, GuardDiscreteConflict)
{
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("guard_discrete_conflict.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 10);
    auto initState = plt->getRootNode();

    std::vector<int> expectedRootMarking = {1,0,1,0,0};
    ASSERT_EQ(expectedRootMarking, initState.getParametricLocation().getDiscreteMarking());

    auto children = plt->getChildNodes(initState);

    std::vector<int> expectedFirstChildMarking = {1,0,1,0,1};
    ASSERT_EQ(1, children.size());
    ASSERT_EQ(EventType::Timed, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_EQ(expectedFirstChildMarking, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_NEAR(7, children[0].getParametricLocation().getSourceEvent().getTime(), 0.01);

    children= plt->getChildNodes(children[0]);

    ASSERT_EQ(2, children.size());
    ASSERT_EQ(EventType::Immediate, children[0].getParametricLocation().getSourceEvent().getEventType());
    ASSERT_NEAR(7, children[0].getParametricLocation().getSourceEvent().getTime(), 0.01);
    ASSERT_EQ(0.5, children[0].getParametricLocation().getConflictProbability());
}

TEST(ParseHybridPetrinet, RateAdaptionNew) {
    auto reader= new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("exampleperformanceeval.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto writer = new PLTWriter();
    writer->writePLT(plt, 10);
}

TEST(ParseHybridPetrinet, GeneralActivatingAnotherGeneral) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("one_gt_enabling_another_gt.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);
    auto jumpChild = plt->getChildNodes(plt->getRootNode())[0];
    auto childOfInterest = plt->getChildNodes(jumpChild)[1];

    EXPECT_EQ(5, childOfInterest.getNodeID());

    vector<vector<vector<double>>> expectedLeftBound{{{0}, {0,0}},
                                                     {{3,-1}}};
    vector<vector<vector<double>>> expectedRightBound{{{3}, {20,0}},
                                                      {{20}}};

    EXPECT_EQ(expectedLeftBound, childOfInterest.getParametricLocation().getGeneralIntervalBoundLeft());
    EXPECT_EQ(expectedRightBound, childOfInterest.getParametricLocation().getGeneralIntervalBoundRight());
}

TEST(ParseHybridPetrinet, ContinuousConflict) {
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("continuous_conflict.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt = parser->parseHybridPetrinet(hybridPetrinet, 20);

    // Two continuous places that get drained at the exact same time resulted in two locations in the plt.
    // These events should be consolidated.

    auto continuousChilds = plt->getChildNodes(plt->getRootNode());

    EXPECT_EQ(1, continuousChilds.size());
    EXPECT_EQ(Continuous, continuousChilds[0].getParametricLocation().getSourceEvent().getEventType());

    auto discreteChilds = plt->getChildNodes(continuousChilds[0]);

    EXPECT_EQ(1, discreteChilds.size());
    EXPECT_EQ(Timed, discreteChilds[0].getParametricLocation().getSourceEvent().getEventType());
}

TEST(ParseHybridPetrinet, MaxtimeToChecktime)
{
    //maxtime20
    //checktime10
    auto reader = new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("example.xml");
    auto parser = new ParseHybridPetrinet();
    auto plt1 = parser->parseHybridPetrinet(hybridPetrinet, 10);
    //auto initState = plt->getRootNode();
    //auto writer = new PLTWriter();
    //ist 20 die Zeit? jupp
    //writer->writePLT(plt, 10);
    //ist das das gleiche wie oben i zwei schritten?
    auto hpng = ReadHybridPetrinet{}.readHybridPetrinet("example.xml");
    auto modelChecker = RegionModelChecker(*hpng, 20);
    //wie komme ich an den plt2?
    auto plt2 = modelChecker -> plt;

    auto children1 = plt1->getChildNodes(plt1->getRootNode());
    auto children2 = plt2->getChildNodes(plt2->getRootNode());

     ASSERT_EQ(children1.size(), children2.size());

     //lieber auto?
    std::vector<ParametricLocationTree::Node> vec1 = plt1->getAllLocations();
    std::vector<ParametricLocationTree::Node> vec2 = plt2->getAllLocations();
    ASSERT_EQ(vec1.size(), vec2.size());
    for(i:=0, i<vec2.size(),i++){

        ASSERT_EQ(children1[i].getDiscreteMarking(), children1[i].getDiscreteMarking());
        ASSERT_EQ(children1[i].getGeneralIntervalBoundRight(), children2[i].getGeneralIntervalBoundRight());
        ASSERT_EQ(children1[i].getGeneralIntervalBoundLeft(), children2[i].getGeneralIntervalBoundLeft());
        ASSERT_EQ(children1[i].getGeneralClock(), children2[i].getGeneralClock());
        ASSERT_EQ(children1[i].getSourceEvent(), children2[i].getSourceEvent());
        //ist MinimumTime die Zeit beim Verlassen?
        ASSERT_EQ(children1[i].getEarliestEntryTime(), children2[i].getEarliestEntryTime());
        ASSERT_EQ(children1[i].getLatestEntryTime(), children2[i].getLatestEntryTime());


    }



    //inspiration
    //auto result = modelChecker.satisfies(Formula(std::make_shared<DiscreteAtomicProperty>("pin1", 2)), 10);
    //EXPECT_NEAR(1.0, round(result.first*10)/10, result.second);

    std::vector<int> expectedRootMarking = {2,0,0,1};
    ASSERT_EQ(expectedRootMarking, initState.getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(2, children.size());
    ASSERT_EQ(EventType::Immediate, children[0].getParametricLocation().getSourceEvent().getEventType())
    ASSERT_EQ(0, children[0].getParametricLocation().getSourceEvent().getTime());

    children = plt->getChildNodes(children[0]);
    ASSERT_EQ(expectedSecondChildMarking1, children[0].getParametricLocation().getDiscreteMarking());
    ASSERT_EQ(0, plt->getChildNodes(children[0]).size());
    ASSERT_EQ(0, plt->getChildNodes(children[1]).size());
}