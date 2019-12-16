#pragma once

#include <datastructures/SingularAutomaton.h>

#include "config.h"
#include "datastructures/HybridAutomaton/Location.h"
#include "datastructures/HybridAutomaton/Transition.h"
#include "datastructures/HybridAutomaton/HybridAutomaton.h"
#include "representations/GeometricObject.h"
#include "algorithms/reachability/Reach.h"
#include "util/plotting/Plotter.h"

using namespace std;
using namespace hypro;

namespace hpnmg {


    typedef mpq_class Number;

    class HybridAutomatonHandler {

    private:

        HybridAutomaton<Number> automaton;
        shared_ptr<SingularAutomaton> singularAutomaton;
        int g;
        int x;
        int c;
        int v;
        vector<unique_ptr<hypro::Location<Number>>> locations;
       // vector<hypro::Transition<Number>> transitions;
        bool flowpipesComputed = false;

    public:

        //shared_ptr<HybridAutomaton<Number>> automaton;

        HybridAutomatonHandler(shared_ptr<SingularAutomaton> singular, double maxTime);

        using flowpipe_t = hypro::reachability::Reach<Number, hypro::reachability::ReachSettings, hypro::State_t<Number>>::flowpipe_t;

        std::vector<std::pair<unsigned, flowpipe_t>> computeFlowpipes(double maxTime, double timestep, int jumpDepth);

        void plotTex(string outputfile, std::vector<std::pair<unsigned, flowpipe_t>> flowpipes);

        bool CheckIfRelevant(Point<double> toCheck, Point<double> ref, std::vector<int> transitions);

        double CalculateProbabiltyForProperty(shared_ptr<SingularAutomaton> automaton,vector<pair<string, map<string, float>>>  distributions, double tMax,  std::vector<int> continiousPlace,  std::vector<string> op,  std::vector<double> value);

        bool CompareValuesWithOp(string op, double val1, double val2);

    private:

        void addLocation(shared_ptr<SingularAutomaton::Location> originalLocation, double maxTime);

        void addTransition(shared_ptr<SingularAutomaton::Transition> originalTransition);

        void setInitialState(int initial);

    };

}