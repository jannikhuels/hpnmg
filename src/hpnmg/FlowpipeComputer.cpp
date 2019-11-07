
#include "FlowpipeComputer.h"

namespace hpnmg {

    FlowpipeComputer::FlowpipeComputer(shared_ptr<HybridPetrinet> petrinet) {
        parser = new ParseHybridPetrinet(true);
        varpool.clear();

        // theoretically we do not need this - check if really needed.
        settingsProvider.addStrategyElement<hypro::CarlPolytope<Number>>(mpq_class(1)/mpq_class(1), hypro::AGG_SETTING::AGG, -1);
        // set settings
        settings.useInvariantTimingInformation = false;
        settings.useGuardTimingInformation = false;
        settings.useBadStateTimingInformation = false;
        settings.timeBound = Number(timeBoundN);
        settingsProvider.setReachabilitySettings(settings);

        mPetrinet = petrinet;
    }

    void FlowpipeComputer::processPetrinet() {

        setup();
        hypro::ContextBasedReachabilityWorker<State> worker = hypro::ContextBasedReachabilityWorker<State>(settings);

        // get root location
        std::cout << "Getting the root location." << std::endl;
        ParametricLocation rootloc = parser->ParseHybridPetrinet::generateRootParametricLocation(mPetrinet,carl::toDouble(timeBoundN));
        hypro::Location<Number>* location = processParametricLocation(rootloc, true);
        locationQueue = {{location,rootloc}};

        bool stop = false;
        int loopNumber = 0;
        // process tasks from global queue
        while(queueManager.hasWorkable(true)) { // using locked access to queues.
            loopNumber++;
            std::cout << sep2 << "Loop number " << loopNumber << std::endl;
            std::cout << "There are still " << locationQueue.size() << " locations in the Queue." << std::endl;

            auto task = queueManager.getNextWorkable(true,true); // get next task locked and dequeue from front.
            assert(task!=nullptr);
            std::cout << "Process task." << std::endl;
            worker.processTask(task,settingsProvider.getStrategy(), globalQueue, globalCEXQueue, segments);

            std::cout << sep2 << "Segments up to this point:" << std::endl;
            for(const auto& segment : segments) {
                std::cout << segment << std::endl;
            }
            std::cout << sep2 << std::endl;

            if(loopNumber == 8) {
                std::cout << "Breaking now. (manually by bool stop)" << std::endl;
                break;
            }

            std::cout << "Adding new locations?" << std::endl;
            // new state
            const auto& segment = segments.getStates()[loopNumber];
            hypro::Location<Number>* currentLocation = locationQueue.front().first;
            ParametricLocation currentParametricLocation = locationQueue.front().second;
            std::cout << "- current location is " << currentLocation << " with name: " << currentLocation->getName() << std::endl;
            locationQueue.pop_front();
            hypro::Condition<Number> currentCondition = currentLocation->getInvariant();
            hypro::matrix_t<Number> invariantMatrix = currentCondition.getMatrix();
            hypro::vector_t<Number> invariantVector = currentCondition.getVector();
            //for ()
            for (int i=0; i<invariantMatrix.rows(); i++) {
                std::cout << "invariantMatrix i= " << i << std::endl;
               // time
                if(invariantMatrix(i,0)!=0) {// add test if this invariant had influence on the region
                double timePassed = invariantVector(i);
                    std::cout << "Invariant: t <= " << timePassed << " is deterministic" << std::endl;

                    hypro::matrix_t<Number> halfspaceMatrix = hypro::matrix_t<Number>::Zero(2,dimension);
                    hypro::vector_t<Number> halfspaceVector = hypro::vector_t<Number>(2);
                    halfspaceMatrix(0,0) = 1;
                    halfspaceMatrix(1,0) = -1;
                    halfspaceVector(0) = timePassed;
                    halfspaceVector(1) = -timePassed;

                    auto newStatePair = segment.satisfiesHalfspaces(halfspaceMatrix, halfspaceVector);
                    if(newStatePair.first == hypro::CONTAINMENT::NO ){
                        std::cout << "Skipping the invariant since the segment is empty!" << std::endl;
                        continue;
                    }
                    std::cout << "New state for deterministic event is \n" << newStatePair.second.getSet() << std::endl;

                    auto constraintset = hypro::Converter<Number>::toConstraintSet(boost::get<hypro::CarlPolytope<Number>>(newStatePair.second.getSet()));
                    hypro::Condition<Number> condition = hypro::Condition<Number>(constraintset);


                    std::cout << sep2 << "Creating new Location for deterministic event." << std::endl;
                    ParametricLocation newLoc = parser->addLocationForDeterministicEvent(mPetrinet->getDeterministicTransitions()["td0"],1.0,timePassed, currentParametricLocation, mPetrinet);
                    hypro::Location<Number>* newLocation = processParametricLocation(newLoc, false, "deterministic event t=" + std::to_string(timePassed), condition);
                    std::cout << sep2 << "Adding the new location to the locationQueue." << std::endl;
                    locationQueue.push_back({newLocation,newLoc});
                }
                for(int j=1; j<invariantMatrix.cols(); j++) {
                    std::cout << "invariantMatrix j= " << j << std::endl;
                    if(invariantMatrix(i,j)!=0) {
                        double boundary = invariantVector(i);
                        std::cout << "Invariant: x_"<< j << "<=" << boundary << " is boundary." << std::endl;

                        hypro::matrix_t<Number> halfspaceMatrix = hypro::matrix_t<Number>::Zero(2,dimension);
                        hypro::vector_t<Number> halfspaceVector = hypro::vector_t<Number>(2);
                        halfspaceMatrix(0,j) = 1;
                        halfspaceMatrix(1,j) = -1;
                        halfspaceVector(0) = boundary;
                        halfspaceVector(1) = -boundary;

                        std::cout << "state set before the intersection " << segment.getSets() << std::endl;

                        auto newStatePair = segment.satisfiesHalfspaces(halfspaceMatrix, halfspaceVector);
                        if(newStatePair.first == hypro::CONTAINMENT::NO || boundary==0) {  // fix test if x=0 the whole time (boundary == 0 is NOT safe)
                            std::cout << "Skipping the invariant since the segment is empty!" << std::endl;
                            continue;
                        }
                        //auto state = segment.intersectHalfspaces(halfspaceMatrix,halfspaceVector);
                        auto state = newStatePair.second;
                        auto set = state.getSet();
                        std::cout << "New state for boundary event is \n" << set << std::endl;

                        auto constraintset = hypro::Converter<Number>::toConstraintSet(boost::get<hypro::CarlPolytope<Number>>(set));
                        hypro::Condition<Number> condition = hypro::Condition<Number>(constraintset);

                        // this probably can be done easier?
                        auto carlPolytope = boost::get<hypro::CarlPolytope<Number>>(set);

                        //std::cout << "carl polytope is " << carlPolytope << " and " << carlPolytope.empty() << std::endl;
                        //std::cout << "carl polytope matrix is " << carlPolytope.matrix() << std::endl;
                        //std::cout << "carl polytope vector is " << carlPolytope.vector() << std::endl;

                        carlPolytope.make_rectangular();
                        std::vector<carl::Interval<Number>> carlIntervalVector = carlPolytope.getIntervals();
                        carl::Interval<Number> carlInterval = carlIntervalVector[0];


                        std::cout << "interval vector" << carlIntervalVector.size() << std::endl;
                        // time passed is not the right time delta!!
                        std::pair<double,double> timeInterval = {carlInterval.lower(), carlInterval.upper()};
                        std::cout << sep2 << "Creating new Location for boundary event (with time interval [" << carlInterval.lower() << "," << carlInterval.upper() << "])." << std::endl;
                        //std::cout << sep2 << "Creating new Location for boundary event (with time interval [" << timeInterval.first << "," << timeInterval.second << "])." << std::endl;
                        ParametricLocation newLoc = parser->addLocationForBoundaryEventByContinuousPlaceMember(mPetrinet->getContinuousPlaces()["pc1"],timeInterval, currentParametricLocation, mPetrinet);
                        hypro::Location<Number>* newLocation = processParametricLocation(newLoc, false, "boundary event x=" + std::to_string(boundary), condition);
                        std::cout << sep2 <<"Adding the new location to the locationQueue." << std::endl;
                        locationQueue.push_back({newLocation,newLoc});

                    }
                }


            }

/*
            //location erzeugen auslagern
            hypro::Location<Number>* loc1 = new hypro::Location<Number>();
            hypro::rectangularFlow<Number> flow = generateFlow({{1,3}});

            //hypro::matrix_t<Number> invariantMatrix = hypro::matrix_t<Number>::Zero(dimension*2,dimension);
            //hypro::vector_t<Number> invariantVector = hypro::vector_t<Number>::Zero(dimension*2);

            invariantMatrix(1,0) = 0;
            invariantVector(1) = 0;

            // erstmal nur die place boundaries
            for  (int k=1; k<dimension; k++) { // zeile,spalte
                invariantMatrix(k*2,k) = 1; // upper bound
                invariantVector(k*2) = 10;
                invariantMatrix(k*2+1,k) = -1;  // lower bound
                invariantVector(k*2+1) = 0;
            }

            std::cout << "Invariant Matrix: " << invariantMatrix.format(Print);
            std::cout << "Invariant Vector: " << invariantVector.format(Print);

            hypro::Condition<Number> invariant(invariantMatrix,invariantVector);
            loc1->setRectangularFlow(flow);
            loc1->setInvariant(invariant);

            std::map<const Location<Number>*, Condition<Number>> initialStates;
            initialStates.emplace(std::make_pair(loc1,condition));
            addStatesToQueue(initialStates);*/


            stop = true;
        }

        // output
        hypro::Plotter<Number>& plotter = hypro::Plotter<Number>::getInstance();
        std::cout << sep2 << "Reachability analysis results in the following locations: \n" << std::endl;
        for(const auto& segment : segments) {
            std::cout << segment << std::endl;
            plotter.addObject(segment.vertices());
        }

        plotter.plot2d();



    }

        hypro::Location<FlowpipeComputer::Number>* FlowpipeComputer::processParametricLocation(ParametricLocation parametricLocation, bool isInitial, string name, hypro::Condition<Number> state) {
        // get deterministic transition firings (-> invariant && transition in HA)
        vector<pair<shared_ptr<DeterministicTransition>, double>> nextDeterministicTransitions = parser->ParseHybridPetrinet::processLocation(parametricLocation, mPetrinet, carl::toDouble(timeBoundN));
        std::cout << "The next deterministic transitions are:" << std::endl;
        for (auto& element : nextDeterministicTransitions) {
            std::cout << "- transition " << element.first->getId() << " at time " << element.second << std::endl;
        }

        // create one location with flow + invariant
        std::cout << "Generating a location." << std::endl;
        hypro::Location<Number>* loc1 = new hypro::Location<Number>();
//        hypro::Location<Number> loc1;
        hypro::rectangularFlow<Number> flow = generateFlow(parametricLocation.getDriftIntervals());
        hypro::Condition<Number> invariant = generateInvariant(nextDeterministicTransitions, mPetrinet->getContinuousPlaces());
        loc1->setRectangularFlow(flow);
        loc1->setInvariant(invariant);

        // generate Constraint Set for initial state
        if(isInitial) {
            state = generateInitialSet();
            loc1->setName("loc_root");
        } else {
            loc1->setName("loc_" + name);
        }

        // set up initial states to the correct representation - in this case CarlPolytope
        std::map<const Location<Number>*, Condition<Number>> initialStates;
        initialStates.emplace(std::make_pair(loc1,state));
//        initialStates.emplace(std::make_pair(&loc1,state));
        addStatesToQueue(initialStates);

        return loc1;
    }

    void FlowpipeComputer::setup() {
        // setup
        std::cout << sep << "Setting up stuff." << std::endl;
        std::cout << "Time bound is " << timeBoundN << "." << std::endl;
        this->dimension = mPetrinet->getContinuousPlaces().size()+1; // plus 1 adds the dimension for the time
        parser->ParseHybridPetrinet::init(mPetrinet);

        // add variables
        std::cout << sep << "Generating the variables (" << dimension << " in total):" << std::endl;
        varpool.newCarlVariable("time");
        std::cout << "- time" << std::endl;
        auto continuousPlaces = parser->getContinuousPlaceIDs();
        for (int k = 0; k < dimension-1; ++k) {
            string varName = continuousPlaces[k];
            varpool.newCarlVariable(varName);
            std::cout << "- " << varName << std::endl;
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
            //invariantVector(0) = nextDeterministicTransitions[0].second; // next det trans. this is wrong, since we need the time from the beginning...
            invariantVector(0) = nextDeterministicTransitions[0].first->getDiscTime(); // this is wrong for transitions which havent been enabled from the beginning! but should work for now
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

        //std::cout << sep << "Tree Stats are: \n" << tree->getTreeStats();

    }

    rectangularFlow<FlowpipeComputer::Number> FlowpipeComputer::generateFlow(std::vector<std::pair<double,double>> driftIntervals) {
        std::cout << sep << "Generating the rectangular flow." << std::endl;
        //carl::Variable t = hypro::VariablePool::getInstance().carlVarByIndex(0);
        std::map<carl::Variable, carl::Interval<Number>> dynamics;
        dynamics.emplace(std::make_pair(varpool.carlVarByIndex(0),carl::Interval<Number>(1,1)));
        std::cout << "- flow dynamics for " << "t" << " : " << "1, 1" << std::endl;
        // for (int i=1; i<=dimension; i++) { dynamics.emplace(std::make_pair(hypro::VariablePool::getInstance().carlVarByIndex(i),carl::Interval<Number>(2,3))); }
        for (int k = 0; k < dimension-1; ++k) {
            string varName = parser->getContinuousPlaceIDs()[k];
            std::cout << "- flow dynamics for " << varName << " : " << driftIntervals[k] << std::endl;
            dynamics.emplace(std::make_pair(varpool.carlVarByIndex(k+1),carl::Interval<Number>(Number(driftIntervals[k].first),Number(driftIntervals[k].second))));
//            dynamics.emplace(std::make_pair(varpool.newCarlVariable(varName),carl::Interval<Number>(Number(driftIntervals[k].first),Number(driftIntervals[k].second))));
        }
        hypro::rectangularFlow<Number> flow(dynamics);
        return flow;
    }

    Condition<FlowpipeComputer::Number> FlowpipeComputer::generateInitialSet() {
        std::cout << sep << "Generating the initial set." << std::endl;
        hypro::matrix_t<Number> constraints = hypro::matrix_t<Number>::Zero(dimension*2,dimension);
//        constraints << 1,0,-1,0,0,1,0,-1;
        constraints(0,0) = 1;
        constraints(1,0) = -1;
        for  (int k=0; k<dimension; k++) { // zeile,spalte
            constraints(k*2,k) = 1; // upper bound
            constraints(k*2+1,k) = -1;  // lower bound
        }
        hypro::vector_t<Number> constants = hypro::vector_t<Number>::Zero(dimension*2);
        std::cout << "State set constraint matrix: " << constraints.format(Print);
        std::cout << "State set constraint (constants) vector: " << constants.format(Print);
        Condition<Number> state(constraints,constants);
        return state;
    }




}