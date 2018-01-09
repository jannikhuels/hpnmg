#include "gtest/gtest.h"
#include "ReadHybridPetrinet.h"

using namespace hpnmg;

TEST(ReadHybridPetrinet, DefaultConstructor)
{
    auto reader= new ReadHybridPetrinet();
    auto hybridPetrinet = reader->readHybridPetrinet("/home/pati/Desktop/hpnmg/test/testfiles/example.xml");
    ASSERT_EQ(hybridPetrinet->num_places(), 3);
}
