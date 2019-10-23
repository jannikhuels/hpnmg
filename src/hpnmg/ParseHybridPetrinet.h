#pragma once

#include <vector>
#include "datastructures/HybridPetrinet.h"
#include "ParametricLocationTree.h"

namespace hpnmg {
    class ParseHybridPetrinet {
    private:
        shared_ptr<ParametricLocationTree> parametriclocationTree;
        vector<string> discretePlaceIDs;
        vector<string> continuousPlaceIDs;
        vector<string> deterministicTransitionIDs;
        vector<string> generalTransitionIDs;
        vector<ParametricLocationTree::Node> locationQueue;
        bool isNondeterministic = false;

        std::vector<std::vector<pair<shared_ptr<DeterministicTransition>, vector<double>>>> sortByEqualTimeDelta(std::vector<pair<shared_ptr<DeterministicTransition>, vector<double>>> deterministicTransitions);

    public:
        //TODO Check to make it a singleton
        //TODO Change public / private accessor
        ParseHybridPetrinet(bool isNondeterministic = false);

        ~ParseHybridPetrinet();

        // Parses a hybrid petrinet recursively until reaching maximal time and generates a parametric location tree
        //
        // @param hybridPetrinet    hybrid petrinet that should be parsed
        // @param maxTime           maximal time for resulting parametric location tree
        // @return                  resulting parametric location tree
        shared_ptr<ParametricLocationTree>
        parseHybridPetrinet(shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime, int mode = 0);

        // Generates the root parametric location
        //
        // @param hybridPetrinet    hybrid petrinet
        // @param maxTime           maximal time of parsing (only used for right bounds of random variables)
        // @return                  resulting root location
        ParametricLocation generateRootParametricLocation(shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime);

        // Generates all child parametric locations for a node, and adds them to the list of nodes to process
        //
        // @param node              node that should be processed
        // @param hybridPetrinet    hybrid petrinet
        // @param maxTime           maximal time for resulting parametric location tree
        // @param mode              mode to handle nondterministic choices
        // @return                  resulting parametric location tree
        void processNode(ParametricLocationTree::Node node, shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime, int mode);


        vector<pair<shared_ptr<DeterministicTransition>, double>> processLocation(ParametricLocation location, shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime);

        vector<double> getTimeDelta(shared_ptr<GuardArc> arc, vector<int> generalTransitionsFired,
                                    vector<vector<vector<double>>> generalIntervalBoundLeft,
                                    vector<vector<vector<double>>> generalIntervalBoundRight,
                                    vector<vector<double>> levels, vector<double> drifts);

        // Computes for 2 of the comparison of 2 linear functions
        //
        // Example:
        // Linear functions: f1 = 2 + 1 * s1 + 2 * s2, f2 = -1 * s1 + s2
        // Input: time1: [2, 1, 2], time2: [0, -1, 1]
        // Return: [-2, -2]
        // Result : s2 <= -2 -2 s1
        vector<double> computeUnequationCut(vector<double> time1, vector<double> time2);

        // Gets a linear function and the bounds for each random variable in this funktion and returns the minimun or the maximum
        //
        // Example:
        // f = 2 - s1 + s2,  s1 in [0,1], s2 in [2, 2+s1], we want the minimum
        // Input: generaltransitionsfired: [0, 0], generalBounds: [[[0],[2,0]]], oppgeneralbounds: [[[1],[2,1]]], time: [1,-1,2]
        // Return: 3
        double getBoundedTime(vector<int> generalTransitionsFired, vector<vector<vector<double>>> generalBounds,
                              vector<vector<vector<double>>> oppositeGeneralBounds, vector<double> time);

        // Returns if a given transition is enabled
        bool transitionIsEnabled(vector<int> discreteMarking, vector<vector<double>> continousMarking,
                                 shared_ptr<Transition> transition, shared_ptr<HybridPetrinet> hybridPetrinet,
                                 vector<vector<vector<double>>> lowerBounds, vector<vector<vector<double>>>
                                 upperBounds, vector<int> generalTransitionsFired);

        bool transitionIsEnabled(vector<int> discreteMarking, vector<pair<double,double>> continousMarkingIntervals,
                                 shared_ptr<Transition> transition, shared_ptr<HybridPetrinet> hybridPetrinet);

        void addLocationForImmediateEvent(shared_ptr<ImmediateTransition> transition,
                                          ParametricLocationTree::Node parentNode, float probability,
                                          shared_ptr<HybridPetrinet> hybridPetrinet);

        void addLocationForGeneralEvent(shared_ptr<GeneralTransition> transition, double maxTime,
                                        vector<double> timeDelta, vector<vector<double>> timeDeltas,
                                        ParametricLocationTree::Node parentNode,
                                        shared_ptr<HybridPetrinet> hybridPetrinet);

        void addLocationForDeterministicEvent(shared_ptr<DeterministicTransition> transition, double probability,
                                              vector<double> timeDelta, vector<vector<double>> timeDeltas,
                                              ParametricLocationTree::Node parentNode,
                                              shared_ptr<HybridPetrinet> hybridPetrinet);


        void addLocationForBoundaryEventByArcMember(shared_ptr<GuardArc> arcMember, vector<double> timeDelta, vector<vector<double>> timeDeltas, ParametricLocationTree::Node parentNode, shared_ptr<HybridPetrinet> hybridPetrinet);
        void addLocationForBoundaryEventByContinuousPlaceMember(shared_ptr<ContinuousPlace> placeMember, vector<double> timeDelta, vector<vector<double>> timeDeltas, ParametricLocationTree::Node parentNode, shared_ptr<HybridPetrinet> hybridPetrinet);
        //void addLocationForBoundaryEvent(vector<double> timeDelta, vector<vector<double>> timeDeltas, ParametricLocationTree::Node parentNode, shared_ptr<HybridPetrinet> hybridPetrinet, std::string);


        long getIndexOfModelMember(string id, vector<string> vectorOfIDs) const;

        long getIndexOfDiscretePlace(shared_ptr<DiscretePlace> discretePlace) const;

        long getIndexOfContinuousPlace(shared_ptr<ContinuousPlace> continuousPlace) const;

        long getIndexOfDeterministicTransition(shared_ptr<DeterministicTransition> deterministicTransition) const;

        long getIndexOfGeneralTransition(shared_ptr<GeneralTransition> generalTransition) const;

        vector<double> getDrift(vector<int> discreteMarking, vector<vector<double>> continuousMarking, shared_ptr<HybridPetrinet> hybridPetrinet, vector<vector<vector<double>>> lowerBounds, vector<vector<vector<double>>> upperBounds, vector<int> generalTransitionsFired);

        vector<pair<double,double>> getDriftIntervals(vector<int> discreteMarking, vector<pair<double,double>> continuousMarkingIntervals, shared_ptr<HybridPetrinet> hybridPetrinet);

        void init(shared_ptr<HybridPetrinet> hybridPetrinet);

        vector<string> getContinuousPlaceIDs();

        std::vector<double> reduceVector(const std::vector<std::vector<double>>* deterministicClocks);

    };
}
