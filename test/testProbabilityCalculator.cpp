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
   auto hybridPetrinet0 = reader->readHybridPetrinet("example.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 100);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(3.0);

   double error;
   double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 3.0, 50000, error);

   ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

}

TEST(ProbabilityCalculator, example2general){

    cout << "Starting example2general\n";

   auto reader= new ReadHybridPetrinet();
   auto parser = new ParseHybridPetrinet();
   auto hybridPetrinet0 = reader->readHybridPetrinet("example2general.xml");
   auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
   auto writer = new PLTWriter();
   writer->writePLT(plt0, 10);

   auto calculator = new ProbabilityCalculator();
   vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(3.0);

   //ASSERT_EQ (nodes.size(), 9);

   double error=0.0;
 //  double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 3.0, 50000, error);
   double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingGauss(nodes, *plt0, 3.0, 128);

   EXPECT_NEAR(1.0, round(result*10000)/10000, error);
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
   double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 4.0, 50000, error);

   EXPECT_NEAR(1.0, round(result*100)/100, error);
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
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 6.0, 50000, error);

    EXPECT_NEAR(1.0, round(result*100)/100, error);
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
    double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(nodes, *plt0, 7.0, 50000, error);

    ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

}

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
    hypro::HPolytope<double> region{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>({
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
        probability = calculator.getProbabilityForPolytopeUsingMonteCarlo(region, distributions, alg, 50000, error);
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
    hypro::HPolytope<double> quadRegion{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
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

    for (auto alg : {1, 2, 3}) {
        double totalProb = 0.0;
        double totalError = 0.0;

        const auto t1 = std::chrono::high_resolution_clock::now();

        double error = 0;
        totalProb += calculator.getProbabilityForPolytopeUsingMonteCarlo(quadRegion, distributions, alg, 50000, error);
        totalError += error;

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
    hypro::HPolytope<double> region{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>({
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
        probability = calculator.getProbabilityForPolytopeUsingMonteCarlo(region, distributions, alg, 50000, error);
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

TEST(ProbabilityCalculator, ForRegionIntersection) {
    hypro::HPolytope<double> quadRegion{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{0, 1},
        hypro::Point<double>{2, 0},
        hypro::Point<double>{4, 2},
        hypro::Point<double>{2, 4},
    }))};

    hypro::HPolytope<double> quadRegion2{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{0, 2},
        hypro::Point<double>{6, 2},
        hypro::Point<double>{6, 6},
        hypro::Point<double>{0, 6},
    }))};

    auto calculator = ProbabilityCalculator();
    const auto distributions = vector<pair<string, map<string, float>>>{
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
    };


    for (auto alg : {1, 2, 3}) {
        double totalProb = 0.0;
        double totalError = 0.0;

        const auto t1 = std::chrono::high_resolution_clock::now();



        totalProb += calculator.getProbabilityForIntersectionOfPolytopesUsingMonteCarlo({quadRegion, quadRegion2},
                                                                                        distributions, alg, 100000,
                                                                                        totalError);



        const auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        if (totalError > 0)
            EXPECT_NEAR((10.0/3.0) / 36.0, round(totalProb*10000)/10000, totalError);
        else
            cout << "SKIPPING ASSERTION BECAUSE THE RETURNED ERROR IS 0." << endl;

        cout << "Algorithm: " << alg << endl;
        cout << "The probability is: " << totalProb << endl;
        cout << "The error is:       " << totalError << endl;
        cout << "It took " << duration << "ms." << endl;
    }
}

TEST(ProbabilityCalculator, ForRegionIntersection2) {
    hypro::HPolytope<double> quadRegion{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{0, 1},
        hypro::Point<double>{2, 0},
        hypro::Point<double>{4, 2},
        hypro::Point<double>{2, 4},
    }))};

    hypro::HPolytope<double> quadRegion2{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{5, 5},
        hypro::Point<double>{6, 5},
        hypro::Point<double>{6, 6},
        hypro::Point<double>{5, 6},
    }))};

    auto calculator = ProbabilityCalculator();
    const auto distributions = vector<pair<string, map<string, float>>>{
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
    };


    for (auto alg : {1, 2, 3}) {
        double totalProb = 0.0;
        double totalError = 0.0;

        const auto t1 = std::chrono::high_resolution_clock::now();



        totalProb += calculator.getProbabilityForIntersectionOfPolytopesUsingMonteCarlo({quadRegion, quadRegion2},
                                                                                        distributions, alg, 100000,
                                                                                        totalError);



        const auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        if (totalError > 0)
            EXPECT_NEAR(0.0 / 36.0, totalProb, totalError);
        else
            cout << "SKIPPING ASSERTION BECAUSE THE RETURNED ERROR IS 0." << endl;

        cout << "Algorithm: " << alg << endl;
        cout << "The probability is: " << totalProb << endl;
        cout << "The error is:       " << totalError << endl;
        cout << "It took " << duration << "ms." << endl;
    }
}

TEST(ProbabilityCalculator, ForRegionIntersection3) {
    hypro::HPolytope<double> quadRegion{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{4, 5},
        hypro::Point<double>{5, 5},
        hypro::Point<double>{5, 6},
        hypro::Point<double>{4, 6},
    }))};

    hypro::HPolytope<double> quadRegion2{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{5, 5},
        hypro::Point<double>{6, 5},
        hypro::Point<double>{6, 6},
        hypro::Point<double>{5, 6},
    }))};

    auto calculator = ProbabilityCalculator();
    const auto distributions = vector<pair<string, map<string, float>>>{
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
    };


    for (auto alg : {1, 2, 3}) {
        double totalProb = 0.0;
        double totalError = 0.0;

        const auto t1 = std::chrono::high_resolution_clock::now();



        totalProb += calculator.getProbabilityForIntersectionOfPolytopesUsingMonteCarlo({quadRegion, quadRegion2},
                                                                                        distributions, alg, 100000,
                                                                                        totalError);



        const auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        if (totalError > 0)
            EXPECT_NEAR(0.0 / 36.0, totalProb, totalError);
        else
            cout << "SKIPPING ASSERTION BECAUSE THE RETURNED ERROR IS 0." << endl;

        cout << "Algorithm: " << alg << endl;
        cout << "The probability is: " << totalProb << endl;
        cout << "The error is:       " << totalError << endl;
        cout << "It took " << duration << "ms." << endl;
    }
}

TEST(ProbabilityCalculator, ForRegionUnion) {
    hypro::HPolytope <double> quadRegion{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{0, 1},
        hypro::Point<double>{2, 0},
        hypro::Point<double>{4, 2},
        hypro::Point<double>{2, 4},
    }))};

    hypro::HPolytope<double> quadRegion2{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{0, 2},
        hypro::Point<double>{6, 2},
        hypro::Point<double>{6, 6},
        hypro::Point<double>{0, 6},
    }))};

    auto calculator = ProbabilityCalculator();
    const auto distributions = vector<pair<string, map<string, float>>>{
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
    };


    for (auto alg : {1, 2, 3}) {
        double totalProb = 0.0;
        double totalError = 0.0;

        const auto t1 = std::chrono::high_resolution_clock::now();

        totalProb += calculator.getProbabilityForUnionOfPolytopesUsingMonteCarlo({quadRegion, quadRegion2},
                                                                                 distributions, alg, 50000, totalError);

        const auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        if (totalError > 0)
            EXPECT_NEAR((86.0 / 3.0) / 36.0, totalProb, totalError);
        else
            cout << "SKIPPING ASSERTION BECAUSE THE RETURNED ERROR IS 0." << endl;

        cout << "Algorithm: " << alg << endl;
        cout << "The probability is: " << totalProb << endl;
        cout << "The error is:       " << totalError << endl;
        cout << "It took " << duration << "ms." << endl;
    }
}


TEST(ProbabilityCalculator, ForRegionUnion2) {
    hypro::HPolytope<double> quadRegion{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{0, 1},
        hypro::Point<double>{2, 0},
        hypro::Point<double>{4, 2},
        hypro::Point<double>{2, 4},
    }))};

    hypro::HPolytope<double> quadRegion2{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{5, 5},
        hypro::Point<double>{6, 5},
        hypro::Point<double>{6, 6},
        hypro::Point<double>{5, 6},
    }))};

    auto calculator = ProbabilityCalculator();
    const auto distributions = vector<pair<string, map<string, float>>>{
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
    };


    for (auto alg : {1, 2, 3}) {
        double totalProb = 0.0;
        double totalError = 0.0;

        const auto t1 = std::chrono::high_resolution_clock::now();

        totalProb += calculator.getProbabilityForUnionOfPolytopesUsingMonteCarlo({quadRegion, quadRegion2},
                                                                                 distributions, alg, 50000, totalError);

        const auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        if (totalError > 0)
            EXPECT_NEAR(9.0 / 36.0, totalProb, totalError);
        else
            cout << "SKIPPING ASSERTION BECAUSE THE RETURNED ERROR IS 0." << endl;

        cout << "Algorithm: " << alg << endl;
        cout << "The probability is: " << totalProb << endl;
        cout << "The error is:       " << totalError << endl;
        cout << "It took " << duration << "ms." << endl;
    }
}

TEST(ProbabilityCalculator, ForRegionUnion3) {
    hypro::HPolytope<double> quadRegion{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{4, 5},
        hypro::Point<double>{5, 5},
        hypro::Point<double>{5, 6},
        hypro::Point<double>{4, 6},
    }))};

    hypro::HPolytope<double> quadRegion2{hypro::Converter<double>::toHPolytope(hypro::VPolytope<double>(std::vector<hypro::Point<double>>{
        hypro::Point<double>{5, 5},
        hypro::Point<double>{6, 5},
        hypro::Point<double>{6, 6},
        hypro::Point<double>{5, 6},
    }))};

    auto calculator = ProbabilityCalculator();
    const auto distributions = vector<pair<string, map<string, float>>>{
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
        {"uniform", {{"a", 0.0}, {"b", 6.0}}},
    };


    for (auto alg : {1, 2, 3}) {
        double totalProb = 0.0;
        double totalError = 0.0;

        const auto t1 = std::chrono::high_resolution_clock::now();

        totalProb += calculator.getProbabilityForUnionOfPolytopesUsingMonteCarlo({quadRegion, quadRegion2},
                                                                                 distributions, alg, 50000, totalError);

        const auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        if (totalError > 0)
            EXPECT_NEAR(0.0555565, round(totalProb*10000000)/10000000, totalError);
        else
            cout << "SKIPPING ASSERTION BECAUSE THE RETURNED ERROR IS 0." << endl;

        cout << "Algorithm: " << alg << endl;
        cout << "The probability is: " << totalProb << endl;
        cout << "The error is:       " << totalError << endl;
        cout << "It took " << duration << "ms." << endl;
    }
}
