#pragma once

#include <datastructures/SingularAutomaton.h>

#include "config.h"
#include "datastructures/HybridAutomaton/LocationManager.h"
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
        vector<hypro::Location<Number>> locations;
        vector<hypro::Transition<Number>> transitions;
        bool automatonExists = false;
        bool flowpipesComputed = false;

    public:

        HybridAutomatonHandler();

        HybridAutomaton<Number> convertAutomaton(shared_ptr<SingularAutomaton> singular, double maxTime);

        std::vector<std::pair<unsigned, hypro::reachability::flowpipe_t<Number>>> computeFlowpipes(double maxTime, double timestep, int jumpDepth);

        void plotTex(string outputfile, std::vector<std::pair<unsigned, hypro::reachability::flowpipe_t<Number>>> flowpipes);

    private:

        void addLocation(shared_ptr<SingularAutomaton::Location> originalLocation, double maxTime);

        void addTransition(shared_ptr<SingularAutomaton::Transition> originalTransition);

        void setInitialState(int initial);

    };

}