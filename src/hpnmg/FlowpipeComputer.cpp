
#include "FlowpipeComputer.h"

namespace hpnmg {

    FlowpipeComputer::FlowpipeComputer() {
    }

    void FlowpipeComputer::processPetrinet(shared_ptr<HybridPetrinet> mPetrinet) {
        // setup
        std::cout << sep << "Setting up stuff." << std::endl;
        Number timeBoundN = 3;
        hypro::VariablePool& varpool = hypro::VariablePool::getInstance();
        varpool.clear();
        std::cout << "Time bound is " << timeBoundN << "." << std::endl;
        this->dimension = mPetrinet->getContinuousPlaces().size()+1; // plus 1 adds the dimension for the time

        // get root location
        std::cout << "Getting the root location." << std::endl;
        auto parser = new ParseHybridPetrinet(true);
        double timeBound = carl::toDouble(timeBoundN);
        parser->ParseHybridPetrinet::init(mPetrinet);
        ParametricLocation rootloc = parser->ParseHybridPetrinet::generateRootParametricLocation(mPetrinet,timeBound);

        // printing stuff
        std::cout << sep << "About the Hybrid Petrinet:" << std::endl;
        std::vector<pair<double,double>> path = rootloc.getContinuousMarkingIntervals();
        std::cout << "Continuous Marking Intervals: ";
        carl::operator<<(std::cout,path) << std::endl;
        std::vector<std::pair<double,double>> driftIntervals = rootloc.getDriftIntervals();
        std::cout << "Drift Intervals: ";
        carl::operator<<(std::cout,driftIntervals) << std::endl;

        // get deterministic transition firings (-> invariant && transition in HA)
        vector<pair<shared_ptr<DeterministicTransition>, double>> nextDeterministicTransitions = parser->ParseHybridPetrinet::processLocation(rootloc, mPetrinet, timeBound);
        std::cout << "The next deterministic transitions are:" << std::endl;
        for (auto& element : nextDeterministicTransitions) {
            std::cout << "- transition " << element.first->getId() << " at time " << element.second << std::endl;
        }

        // generate invariant constraint set
        auto cplaces = mPetrinet->getContinuousPlaces();
        hypro::Condition<Number> invariant = generateInvariant(nextDeterministicTransitions, cplaces);

        // generate Constraint Set for initial state
        std::cout << sep << "Generating the initial set." << std::endl;
        hypro::matrix_t<Number> constraints = hypro::matrix_t<Number>(dimension*2,dimension);
        constraints << 1,0,-1,0,0,1,0,-1;
        hypro::vector_t<Number> constants = hypro::vector_t<Number>::Zero(dimension*2);
        //hypro::vector_t<Number> constants = hypro::vector_t<Number>(4);
        //constants << 1,1,1,1;
        Condition<Number> state(constraints,constants);
        std::cout << "State set constraint matrix: " << constraints.format(Print);
        std::cout << "State set constraint (constants) vector: " << constants.format(Print);

        // settings provider instance as reference for readability
        hypro::SettingsProvider<State>& settingsProvider = hypro::SettingsProvider<State>::getInstance();

        // variables and rectangular dynamics
        std::cout << sep << "Generating the rectangular flow and variables." << std::endl;
        //carl::Variable t = hypro::VariablePool::getInstance().carlVarByIndex(0);
        carl::Variable t = varpool.newCarlVariable("time");
        std::map<carl::Variable, carl::Interval<Number>> dynamics;
        dynamics.emplace(std::make_pair(t,carl::Interval<Number>(1,1)));
        std::cout << "- flow dynamics for " << "t" << " : " << "1, 1" << std::endl;
        // for (int i=1; i<=dimension; i++) { dynamics.emplace(std::make_pair(hypro::VariablePool::getInstance().carlVarByIndex(i),carl::Interval<Number>(2,3))); }
        for (int k = 0; k < dimension-1; ++k) {
            string varName = parser->getContinuousPlaceIDs()[k];
            std::cout << "- flow dynamics for " << varName << " : " << driftIntervals[k] << std::endl;
            dynamics.emplace(std::make_pair(varpool.newCarlVariable(varName),carl::Interval<Number>(Number(driftIntervals[k].first),Number(driftIntervals[k].second))));
        }
        hypro::rectangularFlow<Number> flow(dynamics);

        // create one location with rectangular flow
        std::cout << "Generating a location." << std::endl;
        hypro::Location<Number> loc1;
        loc1.setRectangularFlow(flow);
        loc1.setInvariant(invariant);

        // theoretically we do not need this - check if really needed.
        settingsProvider.addStrategyElement<hypro::CarlPolytope<Number>>(mpq_class(1)/mpq_class(100), hypro::AGG_SETTING::AGG, -1);

        // set settings
        hypro::ReachabilitySettings settings;
        settings.useInvariantTimingInformation = false;
        settings.useGuardTimingInformation = false;
        settings.useBadStateTimingInformation = false;
        settings.timeBound = Number(100);
        settingsProvider.setReachabilitySettings(settings);

        // set up initial states to the correct representation - in this case CarlPolytope
        std::map<const Location<Number>*, Condition<Number>> initialStates;
        initialStates.emplace(std::make_pair(&loc1,state));
        addStatesToQueue(initialStates);

        // create worker
        hypro::ContextBasedReachabilityWorker<State> worker = hypro::ContextBasedReachabilityWorker<State>(settings);

        // data structure to store computed flowpipes
        hypro::Flowpipe<State> segments;

        while(queueManager.hasWorkable(true)) { // using locked access to queues.
            auto task = queueManager.getNextWorkable(true,true); // get next task locked and dequeue from front.
            assert(task!=nullptr);
            worker.processTask(task,settingsProvider.getStrategy(), globalQueue, globalCEXQueue, segments);
        }

        std::cout << sep << "Reachability analysis results in the following locations: \n" << std::endl;
        for(const auto& segment : segments) {
            std::cout << segment << std::endl;
        }

    }

    hypro::Condition<FlowpipeComputer::Number> FlowpipeComputer::generateInvariant(
            vector<pair<shared_ptr<DeterministicTransition>, double>> nextDeterministicTransitions,
            map<string, shared_ptr<ContinuousPlace>> cplaces) {

        std::cout << sep << "Generating the invariants." << std::endl;
        hypro::matrix_t<Number> invariantMatrix = hypro::matrix_t<Number>::Zero(dimension*2,dimension);
        hypro::vector_t<Number> invariantVector = hypro::vector_t<Number>::Zero(dimension*2);

        // time invariants (only upper)
        if (nextDeterministicTransitions.size() != 0) {
            invariantMatrix(0,0) = 1;
            invariantVector(0) = nextDeterministicTransitions[0].second; // next det trans.
        }
        invariantMatrix(1,0) = 0;
        invariantVector(1) = 0;

        // erstmal nur die place boundaries
        int k = 1; // dimension
        for  (auto& cplace : cplaces) { // zeile,spalte
            invariantMatrix(k*2,k) = 1; // upper bound
            invariantVector(k*2) = cplace.second->getCapacity();
            invariantMatrix(k*2+1,k) = -1;  // lower bound
            invariantVector(k*2+1) = 0;
            k=k+1;
        }

        std::cout << "Invariant Matrix: " << invariantMatrix.format(Print);
        std::cout << "Invariant Vector: " << invariantVector.format(Print);

        hypro::Condition<Number> invariant(invariantMatrix,invariantVector);

//        // transition
//        hypro::Transition<Number> trans = hypro::Transition<Number>();
//        trans.setAggregation(hypro::Aggregation::none);
//
//        hypro::matrix_t<Number> guardMatrix = hypro::matrix_t<Number>::Zero(2,2);
//        hypro::vector_t<Number> guardVector = hypro::vector_t<Number>::Zero(2);
//        guardVector(0) = nextDeterministicTransitions[0].second; // funktioniert natürlich nur mit dettr
//        guardVector(1) = nextDeterministicTransitions[0].second;
//        guardMatrix(0,0) = 1;
//        guardMatrix(0,1) = 0;
//        guardMatrix(1,0) = -1;
//        guardMatrix(1,1) = 0;
//        std::cout << "Guard Matrix: " << guardMatrix.format(Print);
//        std::cout << "Guard Vector: " << guardVector.format(Print);
//        hypro::Condition<Number> guard(guardMatrix,guardVector);
//        trans.setGuard(guard);

        return invariant;

    }


        void FlowpipeComputer::addStatesToQueue(map<const Location<Number> *, Condition<Number>> &locationStateMap) {

        std::vector<hypro::State_t<Number>> initialStateData;
        for (auto stateMapIt = locationStateMap.begin(); stateMapIt != locationStateMap.end(); ++stateMapIt) {

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
        hypro::ReachTree<State>* tree = new hypro::ReachTree<State>(new hypro::ReachTreeNode<State>());
        std::vector<hypro::ReachTreeNode<State>*> initialNodes;
        for(const auto& state : initialStateData) {
            hypro::ReachTreeNode<State>* n = new hypro::ReachTreeNode<State>(state,0,tree->getRoot());
            n->setTimestamp(0, carl::Interval<hypro::tNumber>(0));
            initialNodes.push_back(n);
            tree->getRoot()->addChild(n);
        }

        // fill task queue with initial states
        for (const auto& initialNode : initialNodes) {
            globalQueue->enqueue(std::shared_ptr<hypro::Task<State>>(new hypro::Task<State>(initialNode)));
        }

        std::cout << sep << "Tree Stats are: \n" << tree->getTreeStats();

    }


}