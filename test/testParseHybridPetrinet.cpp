#include "gtest/gtest.h"
#include "ReadHybridPetrinet.h"
#include "ParseHybridPetrinet.h"

using namespace hpnmg;

TEST(ParseHybridPetrinet, InitialLocation)
{
    auto reader= new ReadHybridPetrinet();
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
}