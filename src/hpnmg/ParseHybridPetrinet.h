#pragma once

#include <vector>
#include "datastructures/HybridPetrinet.h"
#include "ParametricLocationTree.h"
#include "AleatoryVariable.h"

namespace hpnmg {
    class ParseHybridPetrinet {
    private:
        shared_ptr<ParametricLocationTree> parametriclocationTree;
        vector<string> discretePlaceIDs;
        vector<string> continuousPlaceIDs;
        vector<string> deterministicTransitionIDs;
        vector<string> generalTransitionIDs;
        vector<ParametricLocationTree::Node> locationQueue;

        // Aleatory variable counter for deterministic transitions and continuous places
        // {idDeterministicTransition: [<aleatoryVariable, amount>]}
        map<string, vector<tuple<shared_ptr<AleatoryVariable>, double>>> deterTransAleatoryVariables;
        map<string, vector<tuple<shared_ptr<AleatoryVariable>, double>>> contPlaceAleatoryVariables;

    public:
        ParseHybridPetrinet();

        ~ParseHybridPetrinet();

        shared_ptr<ParametricLocationTree>
        parseHybridPetrinet(shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime);

        ParametricLocation generateRootParametricLocation(shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime);

        void processNode(ParametricLocationTree::Node node, shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime);

        vector<double> getTimeDelta(shared_ptr<GuardArc> arc, vector<int> generalTransitionsFired,
                                    vector<vector<vector<double>>> generalIntervalBoundLeft,
                                    vector<vector<vector<double>>> generalIntervalBoundRight,
                                    vector<vector<double>> levels, vector<double> drifts);

        vector<double> computeUnequationCut(vector<double> time1, vector<double> time2);

        double getBoundedTime(vector<int> generalTransitionsFired, vector<vector<vector<double>>> generalBounds,
                              vector<vector<vector<double>>> oppositeGeneralBounds, vector<double> time);

        bool transitionIsEnabled(vector<int> discreteMarking, vector<vector<double>> continousMarking,
                                 shared_ptr<Transition> transition, shared_ptr<HybridPetrinet> hybridPetrinet);

        void addLocationForImmediateEvent(shared_ptr<ImmediateTransition> transition,
                                          ParametricLocationTree::Node parentNode, float probability,
                                          shared_ptr<HybridPetrinet> hybridPetrinet);

        void addLocationForGeneralEvent(shared_ptr<GeneralTransition> transition, double maxTime,
                                        vector<double> timeDelta, vector<vector<double>> timeDeltas,
                                        ParametricLocationTree::Node parentNode,
                                        shared_ptr<HybridPetrinet> hybridPetrinet);

        void addLocationForGuardEvent(vector<double> timeDelta, vector<vector<double>> timeDeltas,
                                      ParametricLocationTree::Node parentNode,
                                      shared_ptr<HybridPetrinet> hybridPetrinet);

        void addLocationForDeterministicEvent(shared_ptr<DeterministicTransition> transition, double probability,
                                              vector<double> timeDelta, vector<vector<double>> timeDeltas,
                                              ParametricLocationTree::Node parentNode,
                                              shared_ptr<HybridPetrinet> hybridPetrinet);

        void addLocationForBoundaryEvent(vector<double> timeDelta, vector<vector<double>> timeDeltas, ParametricLocationTree::Node parentNode,
                                         shared_ptr<HybridPetrinet> hybridPetrinet);

        vector<double> getDrift(vector<int> discreteMarking, vector<vector<double>> continuousMarking,
                                shared_ptr<HybridPetrinet> hybridPetrinet);
    };
}
