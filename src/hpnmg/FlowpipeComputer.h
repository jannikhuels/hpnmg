
#ifndef HPNMG_FLOWPIPECOMPUTER_H
#define HPNMG_FLOWPIPECOMPUTER_H

#include "ReadHybridPetrinet.h"
#include "datastructures/HybridPetrinet.h"
#include "datastructures/ParametricLocation.h"
#include "ParseHybridPetrinet.h"


#include <datastructures/reachability/SettingsProvider.h>
#include <datastructures/reachability/ReachTree.h>
#include <datastructures/reachability/Task.h>
#include <datastructures/reachability/workQueue/WorkQueueManager.h>
#include <datastructures/reachability/timing/EventTimingProvider.h>
#include <algorithms/reachability/workers/ContextBasedReachabilityWorker.h>
#include <util/plotting/Plotter.h>
#include <Eigen/src/Core/IO.h>

namespace hpnmg {


    class FlowpipeComputer {

    public:
        using Number = double;
        typedef hypro::HPolytope<Number> Representation;
        using State = hypro::State_t<Number>;

    private:

        // cout
        std::string sep = "\n----------------------------------------\n";
        Eigen::IOFormat Print{Eigen::StreamPrecision, Eigen::DontAlignCols, ",", ",", "[", "]", "(",")\n"};

        // class member variables ---------------------------------------------
        int dimension = -1;
        Number timeBoundN = 5;
        shared_ptr<HybridPetrinet> mPetrinet;
        hypro::WorkQueueManager<std::shared_ptr<hypro::Task<State>>> queueManager;
        hypro::WorkQueue<std::shared_ptr<hypro::Task<State>>>* globalCEXQueue = queueManager.addQueue();
        hypro::WorkQueue<std::shared_ptr<hypro::Task<State>>>* globalQueue = queueManager.addQueue();
        std::deque<std::pair<hypro::Location<Number>*,ParametricLocation>> locationQueue;
        ParseHybridPetrinet* parser;
        hypro::VariablePool& varpool = hypro::VariablePool::getInstance();
        hypro::SettingsProvider<State>& settingsProvider = hypro::SettingsProvider<State>::getInstance(); // settings provider instance as reference for readability
        hypro::ReachabilitySettings settings;

        // data structure to store computed flowpipes
        hypro::Flowpipe<State> segments;

        // class member functions ---------------------------------------------
        void setup();

        hypro::Location<Number>* processParametricLocation(ParametricLocation parametricLocation, bool isInitial, hypro::Condition<Number> state = hypro::Condition<Number>());

        hypro::Condition<Number> generateInvariant(vector<pair<shared_ptr<DeterministicTransition>, double>> nextDeterministicTransitions, map<string, shared_ptr<ContinuousPlace>> cplaces);

        hypro::rectangularFlow<Number> generateFlow(std::vector<std::pair<double,double>> driftIntervals);

        hypro::Condition<Number> generateInitialSet();

        void addStatesToQueue(std::map<const Location<Number>*, Condition<Number>> & locationStateMap);




    public:

        FlowpipeComputer(shared_ptr<HybridPetrinet> mPetrinet);

        void processPetrinet();




    };

}
#endif //HPNMG_FLOWPIPECOMPUTER_H
