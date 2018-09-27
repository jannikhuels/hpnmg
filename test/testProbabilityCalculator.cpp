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

 TEST(ProbabilityCalculator, example){

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("example.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 100);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(3.0);

   double error;
   double result = calculator->ProbabilityCalculator::getProbabilityMonteCarloVegas(nodes, *plt0, 3.0, 50000, error);
   ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

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

   ASSERT_EQ (((result - error <= 1) && (1 <= result+error)), true);
}

TEST(ProbabilityCalculator, valuetools){

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("battery_simple.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
   //auto writer = new PLTWriter();
   //writer->writePLT(plt0, 10);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(4.0);

   //ASSERT_EQ (nodes.size(), 9);

   double error;
   double result = calculator->ProbabilityCalculator::getProbabilityMonteCarloVegas(nodes, *plt0, 4.0, 50000, error);

   ASSERT_EQ (((result - error <= 1) && (1 <= result+error)), true);
}

TEST(ProbabilityCalculator, valuetools_v2){

    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("battery_simple_v2.xml");
    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    //auto writer = new PLTWriter();
    //writer->writePLT(plt0, 10);

    auto calculator = new ProbabilityCalculator();
    vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(6.0);

    //ASSERT_EQ (nodes.size(), 9);

    double error;
    double result = calculator->ProbabilityCalculator::getProbabilityMonteCarloVegas(nodes, *plt0, 6.0, 50000, error);

    ASSERT_EQ ((((double)(result - error) <= 1) && (1 <= (double)(result + error))), true);
}

TEST(ProbabilityCalculator, valuetools_v3){

    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("battery_simple_v3.xml");
    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto writer = new PLTWriter();
    writer->writePLT(plt0, 10);

    auto calculator = new ProbabilityCalculator();
    vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(7.0);

    //ASSERT_EQ (nodes.size(), 9);

    double error;
    double result = calculator->ProbabilityCalculator::getProbabilityMonteCarloVegas(nodes, *plt0, 7.0, 50000, error);

    ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

}

TEST(ProbabilityCalculator, valuetools_v4){

    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("valuetools_1.xml");

    cout << endl << "=========" << endl;
    //cout << "Computing startet for td=" << t[i] << endl;
    //shared_ptr<DeterministicTransition> dt = dynamic_pointer_cast<DeterministicTransition>(hybridPetrinet0->getTransitionById("td0"));
    //dt->setDiscTime(t[i]);
    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto calculator = new ProbabilityCalculator();
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(7.0);
    cout << "Number of Dimensions d=" << plt0->getDimension() << endl;
    cout << "Total number of locations n=" << plt0->numberOfLocations() << endl;
    cout << "Number of candidates c=" << nodes.size() << endl;

    double error;
    double result = calculator->ProbabilityCalculator::getProbabilityMonteCarloVegas(nodes, *plt0, 7.0, 50000, error);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();


    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    cout << "The probability is: " << result << endl;
    cout << "It took " << duration << "ms." << endl;

    //auto writer = new PLTWriter();
    //writer->writePLT(plt0, 10);



    //ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

}