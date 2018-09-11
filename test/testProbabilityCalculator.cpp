#include "ReadHybridPetrinet.h"
#include <iostream>
#include <ParseHybridPetrinet.h>
#include <PLTWriter.h>
#include <ctime>
#include <ProbabilityCalculator.h>
#include "ParametricLocationTree.h"
#include "gtest/gtest.h"
#include "PLTWriter.h"

using namespace hpnmg;
using namespace std;

TEST(ProbabilityCalculator, example){

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("example.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 100);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(3.0);

   //ASSERT_EQ (nodes.size(), 9);

   double error;
   double result = calculator->ProbabilityCalculator::getProbabilityMonteCarloVegas(nodes, *plt0, 3.0, 50000, error);
   //double result = calculator->ProbabilityCalculator::getProbabilityGauss(nodes, *plt0, 3.0, 128);


}

TEST(ProbabilityCalculator, example2generalsimple){

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("example2general.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
   //auto writer = new PLTWriter();
   //writer->writePLT(plt0, 10);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(3.0);

   //ASSERT_EQ (nodes.size(), 9);

   double error;
   double result = calculator->ProbabilityCalculator::getProbabilityMonteCarloVegas(nodes, *plt0, 3.0, 50000, error);
   //double result = calculator->ProbabilityCalculator::getProbabilityGauss(nodes, *plt0, 3.0, 128);

}

TEST(ProbabilityCalculator, valuetools){

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("valuetools.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
   //auto writer = new PLTWriter();
   //writer->writePLT(plt0, 10);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(3.0);

   //ASSERT_EQ (nodes.size(), 9);

   double error;
   double result = calculator->ProbabilityCalculator::getProbabilityMonteCarloVegas(nodes, *plt0, 3.0, 50000, error);
   //double result = calculator->ProbabilityCalculator::getProbabilityGauss(nodes, *plt0, 3.0, 128);

   //ASSERT_EQ (result, 1.0);

}