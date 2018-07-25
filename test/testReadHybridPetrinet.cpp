#include "gtest/gtest.h"
#include "ReadHybridPetrinet.h"

using namespace hpnmg;


//TODO: Check the use of new in the whole Project
TEST(ReadHybridPetrinet, ReadPlaces)
{
    auto reader= new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("example.xml");
    ASSERT_EQ(hybridPetrinet->num_places(), 3);
}

TEST(ReadHybridPetrinet, ReadTransitions)
{
    auto reader= new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("example.xml");
    ASSERT_EQ(hybridPetrinet->num_transitions(), 4);
}

TEST(ReadHybridPetrinet, AddArcsToTransition)
{
    auto reader= new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("example.xml");
    auto gTrans = hybridPetrinet->getTransitionById("tg1");
    ASSERT_EQ(gTrans->getDiscreteInputArcs().size(), 1);
    ASSERT_EQ(gTrans->getContinuousInputArcs().size(), 0);
    ASSERT_EQ(gTrans->getGuardInputArcs().size(), 0);
    ASSERT_EQ(gTrans->getDiscreteOutputArcs().size(), 0);
    ASSERT_EQ(gTrans->getContinuousOutputArcs().size(), 0);
    ASSERT_EQ(gTrans->getGuardOutputArcs().size(), 0);
}

TEST(ReadHybridPetrinet, DynamicTransitions)
{
    ReadHybridPetrinet reader;
    shared_ptr<hpnmg::HybridPetrinet> hybridPetrinet = reader.readHybridPetrinet("exampleDynamic.xml");
    
}