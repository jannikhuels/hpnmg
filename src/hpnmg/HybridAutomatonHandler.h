#pragma once

#include <datastructures/SingularAutomaton.h>

#include "config.h"
#include "datastructures/HybridAutomaton/Location.h"
#include "datastructures/HybridAutomaton/Transition.h"
#include "datastructures/HybridAutomaton/HybridAutomaton.h"
#include "representations/GeometricObject.h"
#include "algorithms/reachability/Reach.h"
#include "util/plotting/Plotter.h"

using namespace hypro;

namespace hpnmg {
    using namespace std;

    typedef mpq_class Number;
    typedef HPolytope<Number> Representation;

    class HybridAutomatonHandler {

    private:

        HybridAutomaton<Number> automaton;
        shared_ptr<SingularAutomaton> singularAutomaton;
        reachability::Reach<Number, reachability::ReachSettings, hypro::State_t<Number>> reacher;
        int g;
        int x;
        int c;
        int v;
        vector<unique_ptr<hypro::Location<Number>>> locations;
       // vector<hypro::Transition<Number>> transitions;
        bool flowpipesComputed = false;
        int numberGeneralTransitions;
        bool aggregation;
        int clusterBound;

    public:

        HybridAutomatonHandler(shared_ptr<SingularAutomaton> singular, double maxTime, bool dimPerFiring, map<int,pair<int,int>> mapGeneralTransitions, int numberGeneralTransitions, bool aggregation = true, int clusterBound = 5);

        using flowpipe_t = hypro::reachability::Reach<Number, hypro::reachability::ReachSettings, hypro::State_t<Number>>::flowpipe_t;

        std::vector<std::pair<unsigned, flowpipe_t>> computeFlowpipes(double maxTime, double timestep, int jumpDepth, bool print = false);

        void plotTex(string outputfile, std::vector<std::pair<unsigned, flowpipe_t>> flowpipes);

        ReachTree<hypro::State_t<Number>>* getReachTree();

    private:

        void addLocation(shared_ptr<SingularAutomaton::Location> originalLocation, double maxTime, bool dimPerFiring, map<int,pair<int,int>> mapGeneralTransitions);

        void addTransition(shared_ptr<SingularAutomaton::Transition> originalTransition, bool dimPerFiring, map<int,pair<int,int>> mapGeneralTransitions);

        void setInitialState(vector<int> initial, bool dimPerFiring, map<int,pair<int,int>> mapGeneralTransitions) ;

    };

}