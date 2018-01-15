#include "gtest/gtest.h"
#include "ReadHybridPetrinet.h"

using namespace hpnmg;

TEST(ReadHybridPetrinet, ReadPlaces)
{
    auto reader= new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/example.xml");
    ASSERT_EQ(hybridPetrinet->num_places(), 3);
}

TEST(ReadHybridPetrinet, ReadTransitions)
{
    auto reader= new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/example.xml");
    ASSERT_EQ(hybridPetrinet->num_transistions(), 4);
}

TEST(ReadHybridPetrinet, AddArcsToTransitions)
{
    auto reader= new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/example.xml");
    auto gTrans = hybridPetrinet->getTransitionById("tg1");
    ASSERT_EQ(gTrans->getDiscreteInputArcs().size(), 1);
    ASSERT_EQ(gTrans->getFluidInputArcs().size(), 0);
    ASSERT_EQ(gTrans->getGuardInputArcs().size(), 0);
    ASSERT_EQ(gTrans->getDiscreteOutputArcs().size(), 0);
    ASSERT_EQ(gTrans->getFluidOutputArcs().size(), 0);
    ASSERT_EQ(gTrans->getGuardOutputArcs().size(), 0);
}