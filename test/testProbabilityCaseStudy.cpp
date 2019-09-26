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

TEST(CaseStudy, norep_1_1){

    double detTime = 11;
    double checkTime = 8.0;
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("norep_1_1.xml");

    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto calculator = new ProbabilityCalculator();

    vector <ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);


    vector <ParametricLocationTree::Node> propertySatisfyingNodes;
    for (ParametricLocationTree::Node c : nodes) {
        propertySatisfyingNodes.push_back(c);
    }

    double error;
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(propertySatisfyingNodes,
                                                                                               *plt0, checkTime, 50000,
                                                                                               error);
    ASSERT_NEAR(1.0, result, 0.01+error);
}

TEST(CaseStudy, norep_1_2){

    double detTime = 11;
    double checkTime = 8.0;
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("norep_1_2.xml");

    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto calculator = new ProbabilityCalculator();

    vector <ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);


    vector <ParametricLocationTree::Node> propertySatisfyingNodes;
    for (ParametricLocationTree::Node c : nodes) {
        propertySatisfyingNodes.push_back(c);
    }

    double error;
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(propertySatisfyingNodes,
                                                                                               *plt0, checkTime, 50000,
                                                                                               error);
    ASSERT_NEAR(1.0, result, 0.01+error);
}

TEST(CaseStudy, norep_1_3){

    double detTime = 11;
    double checkTime = 8.0;
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("norep_1_3.xml");

    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto calculator = new ProbabilityCalculator();

    vector <ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);


    vector <ParametricLocationTree::Node> propertySatisfyingNodes;
    for (ParametricLocationTree::Node c : nodes) {
        propertySatisfyingNodes.push_back(c);
    }

    double error;
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(propertySatisfyingNodes,
                                                                                               *plt0, checkTime, 50000,
                                                                                               error);
    ASSERT_NEAR(1.0, result, 0.01+error);
}

TEST(CaseStudy, norep_1_4){

    double detTime = 11;
    double checkTime = 8.0;
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("norep_1_4.xml");

    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto calculator = new ProbabilityCalculator();

    vector <ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);


    vector <ParametricLocationTree::Node> propertySatisfyingNodes;
    for (ParametricLocationTree::Node c : nodes) {
        propertySatisfyingNodes.push_back(c);
    }

    double error;
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(propertySatisfyingNodes,
                                                                                               *plt0, checkTime, 50000,
                                                                                               error);
    ASSERT_NEAR(1.0, result, 0.01+error);
}

TEST(CaseStudy, norep_2_2){

    double detTime = 11;
    double checkTime = 8.0;
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("norep_2_2.xml");

    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto calculator = new ProbabilityCalculator();

    vector <ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);


    vector <ParametricLocationTree::Node> propertySatisfyingNodes;
    for (ParametricLocationTree::Node c : nodes) {
        propertySatisfyingNodes.push_back(c);
    }

    double error;
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(propertySatisfyingNodes,
                                                                                               *plt0, checkTime, 50000,
                                                                                               error);
    ASSERT_NEAR(1.0, result, 0.01+error);
}

TEST(CaseStudy, norep_2_3){

    double detTime = 11;
    double checkTime = 8.0;
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("norep_2_3.xml");

    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto calculator = new ProbabilityCalculator();

    vector <ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);


    vector <ParametricLocationTree::Node> propertySatisfyingNodes;
    for (ParametricLocationTree::Node c : nodes) {
        propertySatisfyingNodes.push_back(c);
    }

    double error;
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(propertySatisfyingNodes,
                                                                                               *plt0, checkTime, 50000,
                                                                                               error);
    ASSERT_NEAR(1.0, result, 0.01+error);
}

TEST(CaseStudy, EasyRepition){

    double detTime = 11;
    double checkTime = 8.0;
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("grid_available_test.xml");

    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto calculator = new ProbabilityCalculator();

    vector <ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);


    vector <ParametricLocationTree::Node> propertySatisfyingNodes;
    for (ParametricLocationTree::Node c : nodes) {
        if (c.getParametricLocation().getDiscreteMarking()[0] == 1) {
            propertySatisfyingNodes.push_back(c);
        }
    }

    double error;
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(propertySatisfyingNodes,
                                                                                               *plt0, checkTime, 50000,
                                                                                               error);
    ASSERT_NEAR(.3, result, 0.01+error);
}

TEST(CaseStudy, ProbTwoConflict){

    double detTime = 11;
    double checkTime = 8.0;
    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("prob_two_conflict.xml");

    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
    auto calculator = new ProbabilityCalculator();
    auto writer = new PLTWriter();
    writer->writePLT(plt0, 10);

    vector <ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);

    vector <ParametricLocationTree::Node> propertySatisfyingNodes;
    for (ParametricLocationTree::Node c : nodes) {
        if (c.getParametricLocation().getDiscreteMarking()[1] == 1) {
            propertySatisfyingNodes.push_back(c);
        }
    }

    double error;
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(propertySatisfyingNodes,
                                                                                               *plt0, checkTime, 50000,
                                                                                               error);
    ASSERT_NEAR(.04, result, 0.01+error);
}