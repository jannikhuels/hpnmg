#include <hpnmg/flowpipeConstruction/ReachRHPn.h>
#include "gtest/gtest.h"
#include "flowpipeConstruction/ReachRHPn.h"
#include "ReadHybridPetrinet.h"
#include "datastructures/HybridPetrinet.h"
#include "FlowpipeComputer.h"

#include <datastructures/reachability/SettingsProvider.h>
#include <datastructures/reachability/ReachTree.h>
#include <datastructures/reachability/Task.h>
#include <datastructures/reachability/workQueue/WorkQueueManager.h>
#include <datastructures/reachability/timing/EventTimingProvider.h>
#include <algorithms/reachability/workers/ContextBasedReachabilityWorker.h>


using namespace hpnmg;

TEST(FlowpipeConstruction, Basic)
{

   // typedef mpq_class Number;
   typedef double Number;
   typedef hypro::HPolytope<Number> Representation;

    // setup
    string filePath = "../../test/testfiles/RHPn/simplerhpn.xml";
    double tauMax = 10.0;

    // read
    ReadHybridPetrinet reader;
    shared_ptr<HybridPetrinet> hybridPetriNet = reader.readHybridPetrinet(filePath);


    //hypro::HybridAutomaton<Number> mAutomaton;
    hypro::ReachabilitySettings settings;
    ReachRHPn<Number,ReachRHPnSettings,hypro::State_t<Number>> reacher(*hybridPetriNet,settings);

    settings = reacher.settings();
    settings.timeStep = carl::convert<double,Number>(0.01);
    settings.timeBound = Number(3);
    settings.jumpDepth = 3;
    reacher.setSettings(settings);
    reacher.setRepresentationType(Representation::type());

    reacher.initQueue();
    //reacher.computeForwardReachability();

    /*// perform reachability analysis.
    auto flowpipeIndices = reacher.computeForwardReachability();

    // PLOTTING
    // plot flowpipes.
    Plotter<Number>& plotter = Plotter<Number>::getInstance();
    plotter.setFilename("simplerhpn");
    for(auto& indexPair : flowpipeIndices){
        std::vector<hypro::State_t<Number>> flowpipe = indexPair.second;
        // Plot single flowpipe
        for(auto& set : flowpipe) {
            std::vector<Point<Number>> points = set.vertices();
            if(!points.empty() && points.size() > 2) {
                for(auto& point : points) {
                    std::cout << point << endl;
                    point.reduceDimension(2);
                }
                plotter.addObject(points);
                points.clear();
            }
        }
    }
    // write output.
    plotter.plotTex();*/

    //reacher.computeForwardReachability();

    /*hypro::WorkQueue<TaskTypePtr>& mWorkingQueue = reacher.rGetQueue();

    // collect all computed reachable states
    std::vector<std::pair<unsigned, typename ReachRHPn<Number,ReachRHPnSettings,State>::flowpipe_t>> collectedReachableStates;

    std::cout << std::endl << "Queue size: " << mWorkingQueue.size() << std::flush;
    std::cout << std::endl;

    while ( !mWorkingQueue.isEmpty() ) {
    TaskTypePtr nextInitialSet = mWorkingQueue.dequeueFront();


    std::cout << "\rQueue size: " << mWorkingQueue.size() << std::flush;


    mCurrentLevel = nextInitialSet->first;
    std::cout <<"Depth " << mCurrentLevel << ", Location: " << nextInitialSet->second.getLocation()->getName() << std::endl;
    assert(int(mCurrentLevel) <= mSettings.jumpDepth);
    TRACE("hypro.reacher","Obtained set of type " << nextInitialSet->second.getSetType() << ", requested type is " << mType);
    flowpipe_t newFlowpipe = computeForwardTimeClosure(nextInitialSet->second);

    collectedReachableStates.emplace_back(std::make_pair(nextInitialSet->second.getLocation()->hash(), newFlowpipe));
    }

    return collectedReachableStates;*/

}


TEST(FlowpipeConstruction, RectangularAutomatonWorker) {
    // typedefs
    using Number = double;
    using State = hypro::State_t<Number>;

    // settings provider instance as reference for readability
    hypro::SettingsProvider<State>& settingsProvider = hypro::SettingsProvider<State>::getInstance();

    // variables
    carl::Variable x = hypro::VariablePool::getInstance().carlVarByIndex(0);
    carl::Variable y = hypro::VariablePool::getInstance().carlVarByIndex(1);
    // rectangular dynamics
    std::map<carl::Variable, carl::Interval<Number>> dynamics;
    dynamics.emplace(std::make_pair(x,carl::Interval<Number>(1,2)));
    dynamics.emplace(std::make_pair(y,carl::Interval<Number>(2,3)));

    // hybrid automaton instance
    hypro::HybridAutomaton<Number> ha;

    // create one location with rectangular flow
    hypro::Location<Number> loc1;
    hypro::rectangularFlow<Number> flow(dynamics);
    loc1.setRectangularFlow(flow);

    // add location
    ha.addLocation(loc1);

    // initial set is a unit box
    hypro::matrix_t<Number> constraints = hypro::matrix_t<Number>(4,2);
    constraints << 1,0,-1,0,0,1,0,-1;
    hypro::vector_t<Number> constants = hypro::vector_t<Number>(4);
    constants << 1,1,1,1;

    // create and add initial state
    ha.addInitialState(&loc1, hypro::Condition<Number>(constraints,constants));

    // theoretically we do not need this - check if really needed.
    settingsProvider.addStrategyElement<hypro::CarlPolytope<Number>>(mpq_class(1)/mpq_class(100), hypro::AGG_SETTING::AGG, -1);

    // set settings
    settingsProvider.setHybridAutomaton(std::move(ha));
    hypro::ReachabilitySettings settings;
    settings.useInvariantTimingInformation = false;
    settings.useGuardTimingInformation = false;
    settings.useBadStateTimingInformation = false;
    settings.timeBound = Number(100);
    settingsProvider.setReachabilitySettings(settings);

    // set up queues
    hypro::WorkQueueManager<std::shared_ptr<hypro::Task<State>>> queueManager;
    auto globalCEXQueue = queueManager.addQueue();
    auto globalQueue = queueManager.addQueue();

    // set up initial states to the correct representation - in this case CarlPolytope
    auto initialStates = settingsProvider.getHybridAutomaton().getInitialStates();
    std::vector<hypro::State_t<Number>> initialStateData;
    for (auto stateMapIt = initialStates.begin(); stateMapIt != initialStates.end(); ++stateMapIt) {

        hypro::State_t<Number> copyState;
        copyState.setLocation(stateMapIt->first);
        copyState.setTimestamp(carl::Interval<hypro::tNumber>(0));

        // if the decider is in use - convert subspaces according to mapping
        State::repVariant _temp;
        _temp = hypro::CarlPolytope<Number>(stateMapIt->second.getMatrix(), stateMapIt->second.getVector());

        copyState.setSet(_temp,0);
        initialStateData.emplace_back(copyState);
    }

    // set up tree
    hypro::ReachTree<State> tree = hypro::ReachTree<State>(new hypro::ReachTreeNode<State>());
    std::vector<hypro::ReachTreeNode<State>*> initialNodes;
    for(const auto& state : initialStateData) {
        hypro::ReachTreeNode<State>* n = new hypro::ReachTreeNode<State>(state,0,tree.getRoot());
        n->setTimestamp(0, carl::Interval<hypro::tNumber>(0));
        initialNodes.push_back(n);
        tree.getRoot()->addChild(n);
    }

    // fill task queue with initial states
    for (const auto& initialNode : initialNodes) {
        globalQueue->enqueue(std::shared_ptr<hypro::Task<State>>(new hypro::Task<State>(initialNode)));
    }

    // create worker
    hypro::ContextBasedReachabilityWorker<State> worker = hypro::ContextBasedReachabilityWorker<State>(settings);

    // data structure to store computed flowpipes
    hypro::Flowpipe<State> segments;

    while(queueManager.hasWorkable(true)) { // locking access to queues.
        auto task = queueManager.getNextWorkable(true,true); // get next task locked and dequeue from front.
        assert(task!=nullptr);
        worker.processTask(task,settingsProvider.getStrategy(), globalQueue, globalCEXQueue, segments);
    }

    for(const auto& segment : segments) {
        std::cout << segment << std::endl;
    }

}


TEST(FlowpipeConstruction, PetriNetFlowpipe) {
    // typedefs/usings
    //typedef mpq_class Number;
    //typedef double Number;
    using Number = double;
    typedef hypro::HPolytope<Number> Representation;
    using State = hypro::State_t<Number>;

    string filePath = "../../test/testfiles/RHPn/simplerhpn_1cp2ct1dp1dt.xml";
    Number timeBoundN = 3;

    // read petrinet
    std::cout << "Reading the petrinet " << filePath << std::endl;
    ReadHybridPetrinet reader;
    shared_ptr<HybridPetrinet> mPetrinet = reader.readHybridPetrinet(filePath);

    FlowpipeComputer computer{};
    computer.processPetrinet(mPetrinet);


}
