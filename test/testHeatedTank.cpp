#include "ReadHybridPetrinet.h"
#include <iostream>
#include <ParseHybridPetrinet.h>
#include <PLTWriter.h>
#include <ctime>
#include <ProbabilityCalculator.h>
#include "ParametricLocationTree.h"
#include "gtest/gtest.h"
#include "PLTWriter.h"
#include <chrono>

using namespace hpnmg;
using namespace std;

TEST(HeatedTank, example){

   double maxTime = 500.0;

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("../../test/testfiles/heatedTank/heatedtank_v2.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, maxTime);

   auto writer = new PLTWriter();
      writer->writePLT(plt0, maxTime);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(maxTime);

   double error;
   double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, maxTime, 10000, error);

   //ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

}
