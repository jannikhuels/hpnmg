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
#include "hpnmg/helper/Triangulation.h"

using namespace hpnmg;
using namespace std;

TEST(ProbabilityCalculator, example){

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("../../test/testfiles/nondeterministicConflicts/nondet2_2g.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 6.0, 0.0);

   auto writer = new PLTWriter();
      writer->writePLT(plt0, 6.0);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(6.0);

   double error;
   double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 6.0, 50000, error);

   //ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

}

//TEST(ProbabilityCalculator, example2general){
//
//    cout << "Starting example2general\n";
//
//   auto reader= new ReadHybridPetrinet();
//   auto parser = new ParseHybridPetrinet();
//   auto hybridPetrinet0 = reader->readHybridPetrinet("example2general.xml");
//   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
//   auto writer = new PLTWriter();
//   writer->writePLT(plt0, 10);
//
//   auto calculator = new ProbabilityCalculator();
//   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(3.0);
//
//   //ASSERT_EQ (nodes.size(), 9);
//
//   double error=0.0;
// //  double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 3.0, 50000, error);
//   double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingGauss(nodes, *plt0, 3.0, 128);
//
//   ASSERT_EQ (((result - error <= 1) && (1 <= result+error)), true);
//}
//
//TEST(ProbabilityCalculator, valuetools){
//
//   auto reader= new ReadHybridPetrinet();
//   auto parser = new ParseHybridPetrinet();
//   auto hybridPetrinet0 = reader->readHybridPetrinet("battery_simple.xml");
//   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
//   //auto writer = new PLTWriter();
//   //writer->writePLT(plt0, 10);
//
//   auto calculator = new ProbabilityCalculator();
//   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(4.0);
//
//   //ASSERT_EQ (nodes.size(), 9);
//
//   double error;
//   double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 4.0, 50000, error);
//
//   ASSERT_EQ (((result - error <= 1) && (1 <= result+error)), true);
//}
//
//TEST(ProbabilityCalculator, valuetools_v2){
//
//    auto reader= new ReadHybridPetrinet();
//    auto parser = new ParseHybridPetrinet();
//    auto hybridPetrinet0 = reader->readHybridPetrinet("battery_simple_v2.xml");
//    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
//    //auto writer = new PLTWriter();
//    //writer->writePLT(plt0, 10);
//
//    auto calculator = new ProbabilityCalculator();
//    vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(6.0);
//
//    //ASSERT_EQ (nodes.size(), 9);
//
//    double error;
//    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 6.0, 50000, error);
//
//    ASSERT_EQ ((((double)(result - error) <= 1) && (1 <= (double)(result + error))), true);
//}
//
//TEST(ProbabilityCalculator, valuetools_v3){
//
//    auto reader= new ReadHybridPetrinet();
//    auto parser = new ParseHybridPetrinet();
//    auto hybridPetrinet0 = reader->readHybridPetrinet("battery_simple_v3.xml");
//    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
//    auto writer = new PLTWriter();
//    writer->writePLT(plt0, 10);
//
//    auto calculator = new ProbabilityCalculator();
//    vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(7.0);
//
//    //ASSERT_EQ (nodes.size(), 9);
//
//    double error;
//    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 7.0, 50000, error);
//
//    ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);
//
//}
//
//TEST(ProbabilityCalculator, valuetools_1){
//
//    auto reader= new ReadHybridPetrinet();
//    auto parser = new ParseHybridPetrinet();
//    auto hybridPetrinet0 = reader->readHybridPetrinet("valuetools_1.xml");
//
//    cout << endl << "=========" << endl;
//    //cout << "Computing startet for td=" << t[i] << endl;
//    shared_ptr<DeterministicTransition> dt = dynamic_pointer_cast<DeterministicTransition>(hybridPetrinet0->getTransitionById("td0"));
//    dt->setDiscTime(6);
//    auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
//    auto calculator = new ProbabilityCalculator();
//    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
//    vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(7.0);
//    cout << "Number of Dimensions d=" << plt0->getDimension() << endl;
//    cout << "Total number of locations n=" << plt0->numberOfLocations() << endl;
//    cout << "Number of candidates c=" << nodes.size() << endl;
//
//    double error;
//    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 7.0, 50000, error);
//    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
//
//
//    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
//    cout << "The probability is: " << result << endl;
//    cout << "It took " << duration << "ms." << endl;
//
//   ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);
//
//    //auto writer = new PLTWriter();
//    //writer->writePLT(plt0, 10);
//
//
//
//    //ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);
//
//}

// Tests Monte-Carlo Plain for a triangle "◣" with vertices (0,0), (1,0), (0,1). Each dimension is uniformly distributed over [0,1].
TEST(ProbabilityCalculator, SimpleMonteCarloPlain) {
    singleDim single;
    single.distribution = {"uniform", {{"a", 0.0}, {"b", 1.0}}};

    allDims all{};
    all.integrals.push_back(single);
    all.integrals.push_back(single);
    all.lowerBounds.push_back({0, 1, 0});
    all.upperBounds.push_back({1, 1, 0});
    all.lowerBounds.push_back({0, 0, 1});
    all.upperBounds.push_back({1, -1, 1});

    auto calculator = ProbabilityCalculator();
    double error = -1;
    double probability = -1;

    const auto t1 = std::chrono::high_resolution_clock::now();
    probability = calculator.computeMultivariateIntegralUsingMonteCarlo(5000, all, all, all, 1, error);
    const auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    cout << "The probability is: " << probability << endl;
    cout << "The error is:       " << error << endl;
    cout << "It took " << duration << "ms." << endl;

    EXPECT_NEAR(0.5 / 1.0, probability, error);
}

// Tests Monte-Carlo Plain for a triangle "▶" with vertices (1,3), (3,4), (1,5). Each dimension is uniformly distributed over [0,5].
TEST(ProbabilityCalculator, RotatetdMonteCarloPlain) {
    singleDim single;
    single.distribution = {"uniform", {{"a", 0.0}, {"b", 5.0}}};

    allDims all{};
    all.integrals.push_back(single);
    all.integrals.push_back(single);
    all.lowerBounds.push_back({1, 1, 0});
    all.upperBounds.push_back({3, 1, 0});
    all.lowerBounds.push_back({2.5, 0.5, 1});
    all.upperBounds.push_back({5.5, -0.5, 1});

    auto calculator = ProbabilityCalculator();
    double error = -1;
    double probability = -1;

    const auto t1 = std::chrono::high_resolution_clock::now();
    probability = calculator.computeMultivariateIntegralUsingMonteCarlo(5000, all, all, all, 1, error);
    const auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    cout << "The probability is: " << probability << endl;
    cout << "The error is:       " << error << endl;
    cout << "It took " << duration << "ms." << endl;

    EXPECT_NEAR(2.0 / 25.0, probability, error);
}

TEST(ProbabilityCalculator, ForRegion2D) {
    Region region{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>({
        hypro::Point<double>{1, 0},
        hypro::Point<double>{3, 0},
        hypro::Point<double>{2, 2},
    }))};

    const auto distributions = vector<pair<string, map<string, float>>>{
        {"uniform", {{"a", 0.0}, {"b", 5.0}}},
        {"uniform", {{"a", 0.0}, {"b", 5.0}}},
    };

    auto calculator = ProbabilityCalculator();
    double error = -1;
    double probability = -1;

    for (auto alg : {1, 2, 3})
    {
        const auto t1 = std::chrono::high_resolution_clock::now();
        probability = calculator.getProbabilityForRegionUsingMonteCarlo(region, distributions, alg, 50000, error);
        const auto t2 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        cout << "Algorithm: " << alg << endl;
        cout << "The probability is: " << probability << endl;
        cout << "The error is:       " << error << endl;
        cout << "It took " << duration << "ms." << endl;

        if (error > 0)
            EXPECT_NEAR(2.0 / 25.0, probability, error);
        else
            cout << "SKIPPING ASSERTION BECAUSE THE RETURNED ERROR IS 0." << endl;
    }
}

TEST(ProbabilityCalculator, ForRegionQuadrilateral) {
    Region quadRegion{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{0, 1},
        hypro::Point<double>{2, 0},
        hypro::Point<double>{4, 2},
        hypro::Point<double>{2, 4},
    }))};

    auto calculator = ProbabilityCalculator();
    const auto distributions = vector<pair<string, map<string, float>>>{
        {"uniform", {{"a", 0.0}, {"b", 4.0}}},
        {"uniform", {{"a", 0.0}, {"b", 4.0}}},
    };

    const auto simplexes = Triangulation::create(quadRegion);
    for (auto alg : {1, 2, 3}) {
        double totalProb = 0.0;
        double totalError = 0.0;

        const auto t1 = std::chrono::high_resolution_clock::now();
        for (auto &region : simplexes) {
            double error = 0;
            totalProb += calculator.getProbabilityForRegionUsingMonteCarlo(region, distributions, alg, 50000, error);
            totalError += error;

        }
        const auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        if (totalError > 0)
            EXPECT_NEAR(8.0 / 16.0, totalProb, totalError);
        else
            cout << "SKIPPING ASSERTION BECAUSE THE RETURNED ERROR IS 0." << endl;

        cout << "Algorithm: " << alg << endl;
        cout << "The probability is: " << totalProb << endl;
        cout << "The error is:       " << totalError << endl;
        cout << "It took " << duration << "ms." << endl;
    }
}

TEST(ProbabilityCalculator, ForRegion3D) {
    Region region{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>({
        hypro::Point<double>{1, 0, 0},
        hypro::Point<double>{0, 1, 0},
        hypro::Point<double>{0, 0, 1},
        hypro::Point<double>{1, 1, 1},
    }))};

    const auto distributions = vector<pair<string, map<string, float>>>{
        {"uniform", {{"a", 0.0}, {"b", 1.0}}},
        {"uniform", {{"a", 0.0}, {"b", 1.0}}},
        {"uniform", {{"a", 0.0}, {"b", 1.0}}},
    };

    auto calculator = ProbabilityCalculator();
    double error = -1;
    double probability = -1;

    for (auto alg : {1, 2, 3})
    {
        const auto t1 = std::chrono::high_resolution_clock::now();
        probability = calculator.getProbabilityForRegionUsingMonteCarlo(region, distributions, alg, 50000, error);
        const auto t2 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        cout << "Algorithm: " << alg << endl;
        cout << "The probability is: " << probability << endl;
        cout << "The error is:       " << error << endl;
        cout << "It took " << duration << "ms." << endl;

        if (error > 0)
            EXPECT_NEAR(1.0 / 3.0, probability, error);
        else
            cout << "SKIPPING ASSERTION BECAUSE THE RETURNED ERROR IS 0." << endl;
    }
}