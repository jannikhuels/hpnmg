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

double getBoundedTime(vector<int> generalTransitionsFired,
                                           vector<vector<vector<double>>> generalBounds,
                                           vector<vector<vector<double>>> oppositeGeneralBounds,
                                           vector<double> time) {
    assert(time.size() == generalTransitionsFired.size() + 1);
    vector<int> counter = vector<int>(generalBounds.size());
    fill(counter.begin(), counter.end(), 0);
    for (int &firedTransition : generalTransitionsFired)
        counter[firedTransition] += 1;
    while (time.size() > 1) {
        // get last value in timevector
        double value = time.back();
        time.pop_back();

        // get boundaries of last value
        int transitionPos = generalTransitionsFired[time.size() - 1];
        int firingTime = counter[transitionPos] - 1;
        vector<double> boundaries = generalBounds[transitionPos][firingTime];
        vector<double> oppositeBoundaries = oppositeGeneralBounds[transitionPos][firingTime];
        counter[transitionPos] -= 1;

        // add boundaries multiplicated with value to time vector
        for (int i = 0; i < time.size(); ++i)
            if (value >= 0)
                time[i] += value * boundaries[i];
            else
                time[i] += value * oppositeBoundaries[i];

    }
    return time[0];
}

TEST(Valuetools, valuetools_1){

    double detTime[] = {11};
    //double checkTimes[] = {0.0, 3.0, 4.0, 6.0, 8.0, 9.0};
    double checkTimes[] = {8.0};
    for (double time : detTime) {

        for(double checkTime : checkTimes) {
            // TODO: Check why I cannot reuse hybridPetrinet0 (When i remove the next 3 lines and move them outside the for loops, the probability is always 0)
            auto reader= new ReadHybridPetrinet();
            auto parser = new ParseHybridPetrinet();
            auto hybridPetrinet0 = reader->readHybridPetrinet("norep_2_3.xml");
            //auto hybridPetrinet0 = reader->readHybridPetrinet("valuetools_1_1.xml");

            cout << endl << "=========" << endl;
            cout << "Computing startet for td=" << time << ", checkTime=" << checkTime << endl;
            /*shared_ptr <DeterministicTransition> dt = dynamic_pointer_cast<DeterministicTransition>(
                    hybridPetrinet0->getTransitionById("td0"));
            dt->setDiscTime(time);*/

            auto plt0 = parser->parseHybridPetrinet(hybridPetrinet0, 10);
            auto calculator = new ProbabilityCalculator();
            auto writer = new PLTWriter();
            writer->writePLT(plt0, 10);

            cout << "Discrete initial Marking: [" << plt0->getRootNode().getParametricLocation().getDiscreteMarking() << "]" << endl;
            //cout << "Continous Marking" << plt0->getRootNode().getParametricLocation().getContinuousMarking()[0] << endl;

            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
            vector <ParametricLocationTree::Node> nodes = plt0->getCandidateLocationsForTime(checkTime);
            cout << "Number of Dimensions d=" << plt0->getDimension() << endl;
            cout << "Total number of locations n=" << plt0->numberOfLocations() << endl;
            cout << "Number of candidates c=" << nodes.size() << endl;


            vector <ParametricLocationTree::Node> propertySatisfyingNodes;
            for (ParametricLocationTree::Node c : nodes) {

                /*double minLevel = getBoundedTime(c.getParametricLocation().getGeneralTransitionsFired(),
                                                 c.getParametricLocation().getGeneralIntervalBoundLeft(),
                                                 c.getParametricLocation().getGeneralIntervalBoundRight(),
                                                 c.getParametricLocation().getContinuousMarking()[0]);

                double maxLevel = getBoundedTime(c.getParametricLocation().getGeneralTransitionsFired(),
                                                 c.getParametricLocation().getGeneralIntervalBoundRight(),
                                                 c.getParametricLocation().getGeneralIntervalBoundLeft(),
                                                 c.getParametricLocation().getContinuousMarking()[0]);*/

                /*cout << "NodeID: " << c.getNodeID() << endl;
                cout << "minLevel: " << minLevel << endl;
                cout << "maxLevel: " << maxLevel << endl;
                cout << "drift: " << c.getParametricLocation().getDrift()[0] << endl;*/

                //cout << "Current marktin "  << c.getParametricLocation().getDiscreteMarking() << endl;
                /*if ((minLevel <= 0.0 || maxLevel <= 0)) {
                    propertySatisfyingNodes.push_back(c);
                }*/
                //cout << "Current marking "  << c.getParametricLocation().getDiscreteMarking() << endl;
                /*if (c.getParametricLocation().getDiscreteMarking()[0] <= 0) {
                    propertySatisfyingNodes.push_back(c);
                }*/
                /*if (c.getParametricLocation().getDiscreteMarking()[0] <= 0 && c.getParametricLocation().getDiscreteMarking()[1] <= 0 && c.getParametricLocation().getDiscreteMarking()[2] <= 0 && c.getParametricLocation().getDiscreteMarking()[3] <= 0) {
                    propertySatisfyingNodes.push_back(c);
                }*/

                /*if (c.getParametricLocation().getDiscreteMarking()[3] <= 0) {
                    propertySatisfyingNodes.push_back(c);
                }*/
                propertySatisfyingNodes.push_back(c);
            }

            cout << "Number of candidates that satisfy the property " << propertySatisfyingNodes.size() << endl;

            double error;
            double result = calculator->ProbabilityCalculator::getTotalProbabilityUsingMonteCarloVegas(propertySatisfyingNodes,
                                                                                             *plt0, checkTime, 50000,
                                                                                             error);
            std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();


            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            cout << "The probability is: " << result << endl;
            cout << "The error is: " << error << endl;
            cout << "It took " << duration << "ms." << endl;

            //ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result + error))), true);
        }
    }


    //auto writer = new PLTWriter();
    //writer->writePLT(plt0, 10);



    //ASSERT_EQ (((round(result - error) <= 1) && (1 <= round(result+error))), true);

}