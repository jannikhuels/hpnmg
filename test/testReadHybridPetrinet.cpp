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

TEST(ReadHybridPetrinet, ReadArcs)
{
    auto reader= new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/example.xml");
    ASSERT_EQ(hybridPetrinet->num_arcs(), 6);
}
