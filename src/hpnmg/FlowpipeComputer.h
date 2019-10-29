
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

        int dimension;
        hypro::WorkQueueManager<std::shared_ptr<hypro::Task<State>>> queueManager;
        hypro::WorkQueue<std::shared_ptr<hypro::Task<State>>>* globalCEXQueue = queueManager.addQueue();
        hypro::WorkQueue<std::shared_ptr<hypro::Task<State>>>* globalQueue = queueManager.addQueue();

        hypro::Condition<Number> generateInvariant(vector<pair<shared_ptr<DeterministicTransition>, double>> nextDeterministicTransitions, map<string, shared_ptr<ContinuousPlace>> cplaces);

        void addStatesToQueue(std::map<const Location<Number>*, Condition<Number>> & locationStateMap);

    public:

        FlowpipeComputer();

        void processPetrinet(shared_ptr<HybridPetrinet> mPetrinet);




    };

}
#endif //HPNMG_FLOWPIPECOMPUTER_H
