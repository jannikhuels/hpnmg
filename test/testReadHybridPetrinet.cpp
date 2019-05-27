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
    EXPECT_EQ(4, hybridPetrinet->num_transitions());
    EXPECT_EQ(3, hybridPetrinet->num_places());
    shared_ptr<DynamicTransition> dynamicTransition = dynamic_pointer_cast<DynamicTransition>(hybridPetrinet->getTransitionById("td0"));
    EXPECT_EQ(5,dynamicTransition->getRate());
    EXPECT_EQ(2,dynamicTransition->getTransitions().size());
    EXPECT_EQ(2,dynamicTransition->getTransitionFactors().size());
    EXPECT_EQ(2,dynamicTransition->getTransitionFactors()[0]);
    EXPECT_EQ(1,dynamicTransition->getTransitionFactors()[1]);
    EXPECT_EQ(2,dynamicTransition->getTransitions()[0]->getRate());
    EXPECT_EQ(2,dynamicTransition->getTransitions()[1]->getRate());
    EXPECT_EQ(1,dynamicTransition->getFactor());
    EXPECT_EQ(-1,dynamicTransition->getConstant());
    EXPECT_EQ(0,dynamicTransition->getParameter());

    shared_ptr<ContinuousTransition> tf0 = dynamic_pointer_cast<ContinuousTransition>(hybridPetrinet->getTransitionById("tf0"));
    shared_ptr<ContinuousTransition> tf1 = dynamic_pointer_cast<ContinuousTransition>(hybridPetrinet->getTransitionById("tf1"));
    shared_ptr<ContinuousTransition> tf2 = dynamic_pointer_cast<ContinuousTransition>(hybridPetrinet->getTransitionById("tf2"));
    EXPECT_EQ(2,tf0->getRate());
    EXPECT_EQ(2,tf1->getRate());
    EXPECT_EQ(2,tf2->getRate());
}
