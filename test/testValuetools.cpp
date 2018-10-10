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

TEST(Valuetools, valuetools_1){

    auto reader= new ReadHybridPetrinet();
    auto parser = new ParseHybridPetrinet();
    auto hybridPetrinet0 = reader->readHybridPetrinet("valuetools_1_1.xml");

    //double detTime[] = {11,10,9,8,7,6,5,4,3,2,1};
    double detTime[]= {20};
    double checkTime = 2.0;
    for (double time : detTime) {


        cout << endl << "=========" << endl;
        cout << "Computing startet for td=" << time << ", checkTime=" << checkTime << endl;
        shared_ptr<DeterministicTransition> dt = dynamic_pointer_cast<DeterministicTransition>(hybridPetrinet0->getTransitionById("td0"));
        dt->setDiscTime(time);
        auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
        auto calculator = new ProbabilityCalculator();
        auto writer = new PLTWriter();
        writer->writePLT(plt0,10);

        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        vector<ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);
        cout << "Number of Dimensions d=" << plt0->getDimension() << endl;
        cout << "Total number of locations n=" << plt0->numberOfLocations() << endl;
        cout << "Number of candidates c=" << nodes.size() << endl;

        double error;
        double result = calculator->ProbabilityCalculator::getProbabilityMonteCarloVegas(nodes, *plt0, checkTime, 50000,
                                                                                         error);
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();


        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
        cout << "The probability is: " << result << endl;
        cout << "It took " << duration << "ms." << endl;

        ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);
    }


    //auto writer = new PLTWriter();
    //writer->writePLT(plt0, 10);



    //ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

}