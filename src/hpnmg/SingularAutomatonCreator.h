#pragma once

#include <datastructures/HybridPetrinet.h>
#include <datastructures/SingularAutomaton.h>
#include "ParametricLocationTree.h"
#include "ParseHybridPetrinet.h"
#include <memory>

using namespace std;

namespace hpnmg {


    class SingularAutomatonCreator {
//    private:
//        // struct for non-stochastic entering state
//        struct PetriNetState {
//            vector<int> discreteMarking;
//            vector<vector<vector<double>>> orderedContinuousMarking;
//            vector<vector<vector<double>>> orderedDeterministicClock;
//
//            // define how to compare such two states
//            bool operator<(const PetriNetState &other) const {
//                // compare the discrete markings
//                if (this->discreteMarking != other.discreteMarking) {
//                    // if the discrete markings are unequal, return if this is smaller than other
//                    return this->discreteMarking < other.discreteMarking;
//                }
//                // compare the ordered continuous markings
//                if (compareOrderedVectors(this->orderedContinuousMarking, other.orderedContinuousMarking) xor
//                    compareOrderedVectors(other.orderedContinuousMarking, this->orderedContinuousMarking)) {
//                    // if the ordered continuous markings are unequal, return if this is smaller than other
//                    return compareOrderedVectors(this->orderedContinuousMarking, other.orderedContinuousMarking);
//                }
//                // return return if this is smaller than other, concerning the ordered continuous markings
//                return compareOrderedVectors(this->orderedDeterministicClock, other.orderedDeterministicClock);
//            }
//        };
//
//    private:
//        map<PetriNetState, shared_ptr<SingularAutomaton::Location>> mapStateToLocation;

    public:
        pair<shared_ptr<ParametricLocationTree>, shared_ptr<SingularAutomaton>> transformIntoSingularAutomaton(shared_ptr<HybridPetrinet> hybridPetriNet, double maxTime, int mode = 2);
        shared_ptr<SingularAutomaton> addDistributions(shared_ptr<SingularAutomaton> automaton, vector<pair<string, map<string, float>>> distributionsNormalized);

    private:
        // Adds children of currentNode
        void addLocationsForChildren(shared_ptr<ParseHybridPetrinet> hybridPetriNetParser,
                                     shared_ptr<ParametricLocationTree> parametricLocationTree,
                                     shared_ptr<SingularAutomaton> singularAutomaton,
                                     ParametricLocationTree::Node *parentNode,
                                     shared_ptr<SingularAutomaton::Location> locationForParent);

        void transformEventIntoTransition(shared_ptr<ParseHybridPetrinet> hybridPetriNetParser,
                                          shared_ptr<ParametricLocationTree> parametricLocationTree,
                                          shared_ptr<SingularAutomaton> singularAutomaton,
                                          ParametricLocation* parentParametricLocation,
                                          shared_ptr<SingularAutomaton::Location> locationForParent,
                                          ParametricLocation* childParametricLocation,
                                          shared_ptr<SingularAutomaton::Location> locationForChild);

        void mergeIdenticalLocations(shared_ptr<SingularAutomaton> singularAutomaton);

        //vector<vector<vector<double>>> sortByOrder(const vector<vector<double>> &values, const vector<int> &order);

        // Compares two ordered vectors
        // Returns true, if lhs < rhs and false otherwise
        //static bool compareOrderedVectors(const vector<vector<vector<double>>> &lhs, const vector<vector<vector<double>>> &rhs);

    };
}