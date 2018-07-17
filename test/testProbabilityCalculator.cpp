#include "ReadHybridPetrinet.h"
#include <iostream>
#include <ParseHybridPetrinet.h>
#include <PLTWriter.h>
#include <ctime>
#include <ProbabilityCalculator.h>
#include "ParametricLocationTree.h"
#include "gtest/gtest.h"

using namespace hpnmg;
using namespace std;

TEST(ProbabilityCalculator, example2generalsimple){

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("example2generalsimple.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(5.0);

   //ASSERT_EQ (nodes.size(), 9);

   double result = calculator->ProbabilityCalculator::getProbability(nodes, *plt0, 5.0);

   //ASSERT_EQ (result, 1.0);

}