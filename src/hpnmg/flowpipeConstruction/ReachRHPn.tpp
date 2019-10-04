#include "ReachRHPn.h"
#include "datastructures/ParametricLocation.h"
#include "ParseHybridPetrinet.h"
#include <representations/ConstraintSet/ConstraintSet.h>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <carl/io/streamingOperators.h>


using namespace hypro;


namespace hpnmg {

	using clock = std::chrono::high_resolution_clock;
	using timeunit = std::chrono::microseconds;

	template<typename Number, typename ReacherSettings, typename State>
	ReachRHPn<Number,ReacherSettings,State>::ReachRHPn(const HybridPetrinet& _petrinet, const ReachabilitySettings& _settings)
		: mPetrinet (_petrinet), mSettings(_settings), mCurrentLevel(0), mIntersectedBadStates(false) {
			//mAutomaton.addArtificialDimension();
		}



	template<typename Number, typename ReacherSettings, typename State>
	void ReachRHPn<Number,ReacherSettings,State>::initQueue() {
	    // get root location
        shared_ptr<hpnmg::HybridPetrinet> mPetrinetPtr = std::make_shared<hpnmg::HybridPetrinet>(mPetrinet);
        auto parser = new ParseHybridPetrinet(true);
        double timeBound = carl::toDouble(mSettings.timeBound);
        parser->ParseHybridPetrinet::init(mPetrinetPtr);
        ParametricLocation rootloc = parser->ParseHybridPetrinet::generateRootParametricLocation(mPetrinetPtr,timeBound);

        // printing stuff
        std::vector<pair<double,double>> path = rootloc.getContinuousMarkingIntervals();
        std::cout << "Continuous Marking Intervals: ";
        carl::operator<<(std::cout,path) << std::endl;
        std::vector<std::pair<double,double>> driftIntervals = rootloc.getDriftIntervals();
        std::cout << "Drift Intervals: ";
        carl::operator<<(std::cout,driftIntervals) << std::endl;


        // generate Constraint Set for initial state
	    int dimension = mPetrinet.getContinuousPlaces().size();
	    State s;
	    matrix_t<Number> constraints =  matrix_t<Number>::Identity(dimension,dimension);
	    vector_t<Number> constants = vector_t<Number>::Zero(dimension);
        std::cout << "State set constraint matrix: " << constraints;
        std::cout << "State set constraint (constants) vector: " << constants;
	    ConstraintSetT<Number,ConstraintSetSettings> constraintSet = ConstraintSetT<Number,ConstraintSetSettings>(constraints, constants);
	    assert(mType == representation_name::polytope_h);

	    // generate location and set name - location instanz liegt jetzt im heap; next step: location wird von Parametric Location gehalten
	    hypro::Location<Number> *location = new hypro::Location<Number>; // muss irgendwann deleted werden TODO mit delete location;
	    location->setName("location_0");

	    // set rectangular flow and create variables
        auto& vpool = VariablePool::getInstance();
        //carl::Variable t = vpool.newCarlVariable("t");
	    hypro::rectangularFlow<Number> rectangularFlow;
	    // vpool.carlVariablebyIndex(0)
        rectangularFlow.setFlowIntervalForDimension(carl::Interval<Number>{1,1},vpool.newCarlVariable("t"));
        for (int k = 0; k < dimension; ++k) {
            string varName = parser->getContinuousPlaceIDs()[k];
            //carl::Variable xp = vpool.newCarlVariable(varName); driftIntervals[k]
            rectangularFlow.setFlowIntervalForDimension(carl::Interval<Number>(Number(driftIntervals[k].first),Number(driftIntervals[k].second)),vpool.newCarlVariable(varName));
        }
        location->setRectangularFlow(rectangularFlow);
        std::cout << "Set rectangular flow of location " << location->getName() << " to: " << location->getRectangularFlow();

        // generate invariants TODO
        // deterministic clocks are set in gen. root loc. (to zero)

        // TODO: what to do about transitions?

        // set location for state, set constraint sets of state
	    s.setLocation(location); //location is a pointer -> no & needed
	    std::cout << s.getLocation()->getName();
	    s.setSetDirect(HPolytope<Number>(constraints, constants));
        //s.setSetDirect(HPolytope<Number>(locationConstraintPair.second.getMatrix(),locationConstraintPair.second.getVector()));
	    // TODO consider jumpDepth

	/*	TRACE("hypro.reacher.preprocessing","Have " << mPetrinet.getInitialStates().size() << " initial states.");
		for ( const std::map<const Location<Number>*, Condition<Number>>& locationConstraintPair : mPetrinet.getInitialStates() ) {
			if(int(mCurrentLevel) <= mSettings.jumpDepth || mSettings.jumpDepth < 0){
				// Convert representation in state from matrix and vector to used representation type.
				State s;
				s.setLocation(locationConstraintPair.first);
				switch(mType){
					case representation_name::box: {
						s.setSetDirect(Box<Number>(locationConstraintPair.second.getMatrix(),locationConstraintPair.second.getVector()));
						break;
					}
					case representation_name::polytope_h: {
						s.setSetDirect(HPolytope<Number>(locationConstraintPair.second.getMatrix(),locationConstraintPair.second.getVector()));
						break;
					}
					case representation_name::polytope_v: {
						s.setSetDirect(VPolytope<Number>(locationConstraintPair.second.getMatrix(),locationConstraintPair.second.getVector()));
						break;
					}
					case representation_name::support_function: {
						s.setSetDirect(SupportFunction<Number>(locationConstraintPair.second.getMatrix(),locationConstraintPair.second.getVector()));
						break;
					}
					case representation_name::zonotope: {
						s.setSetDirect(Zonotope<Number>(locationConstraintPair.second.getMatrix(),locationConstraintPair.second.getVector()));
						break;
					}
					case representation_name::constraint_set: {
						s.setSetDirect(ConstraintSet<Number>(locationConstraintPair.second.getMatrix(),locationConstraintPair.second.getVector()));
						break;
					}
					#ifdef HYPRO_USE_PPL
					case representation_name::ppl_polytope: {
						s.setSetDirect(Polytope<Number>(locationConstraintPair.second.getMatrix(),locationConstraintPair.second.getVector()));
						break;
					}
					#endif

					default: {
						assert(false);
					}
				}
				TRACE("hypro.reacher.preprocessing","convert.");
				//s.setSetDirect(boost::apply_visitor(genericConversionVisitor<typename State::repVariant, Number>(mType), s.getSet()));
				s.setSetType(mType);
				TRACE("hypro.reacher.preprocessing","Type after conversion is " << s.getSetType(0));

				DEBUG("hypro.reacher","Adding initial set of type " << mType << ", current queue size (before) is " << mWorkingQueue.size());
				assert(mType == s.getSetType());

				s.setTimestamp(carl::Interval<tNumber>(0));
				mWorkingQueue.enqueue(std::make_unique<TaskType>(std::make_pair(mCurrentLevel, s)));
			}
		}*/

        TRACE("hypro.reacher.preprocessing","convert.");
        //s.setSetDirect(boost::apply_visitor(genericConversionVisitor<typename State::repVariant, Number>(mType), s.getSet()));
        s.setSetType(mType);
        TRACE("hypro.reacher.preprocessing","Type after conversion is " << s.getSetType(0));

        DEBUG("hypro.reacher","Adding initial set of type " << mType << ", current queue size (before) is " << mWorkingQueue.size());
        assert(mType == s.getSetType());

        s.setTimestamp(carl::Interval<tNumber>(0));
        mWorkingQueue.enqueue(std::make_unique<TaskType>(std::make_pair(mCurrentLevel, s)));
//        TaskTypePtr nextInitialSet = mWorkingQueue.dequeueFront();
//        std::cout << nextInitialSet->second.getLocation()->getName();
//        mWorkingQueue.enqueue(std::move(nextInitialSet));
	}

	template<typename Number, typename ReacherSettings, typename State>
	std::vector<std::pair<unsigned, typename ReachRHPn<Number,ReacherSettings,State>::flowpipe_t>> ReachRHPn<Number,ReacherSettings,State>::computeForwardReachability() {
		// set up working queue -> add initial states
		initQueue();
		// collect all computed reachable states
		std::vector<std::pair<unsigned, typename ReachRHPn<Number,ReacherSettings,State>::flowpipe_t>> collectedReachableStates;

		if(ReacherSettings::printStatus) {
			std::cout << std::endl << "Queue size: " << mWorkingQueue.size() << std::flush;
		}

		TRACE("hypro.reacher","working queue size: " << mWorkingQueue.size());

		if(ReacherSettings::printStatus) {
			std::cout << std::endl;
		}
		while ( !mWorkingQueue.isEmpty() ) {
			TaskTypePtr nextInitialSet = mWorkingQueue.dequeueFront();
			TRACE("hypro.reacher","working queue after pop: " << mWorkingQueue.size());
			if(ReacherSettings::printStatus) {
				std::cout << "\rQueue size: " << mWorkingQueue.size() << std::flush;
			}

			mCurrentLevel = nextInitialSet->first;
			std::cout << "Depth " << mCurrentLevel << ", Location: " << nextInitialSet->second.getLocation()->getName();
			std::cout << "Dot repr. of loc:" << std::endl;
			//std::cout << nextInitialSet->second.getLocation()->getDotRepresentation

			assert(int(mCurrentLevel) <= mSettings.jumpDepth);
            std::cout <<"Obtained set of type " << nextInitialSet->second.getSetType() << ", requested type is " << mType;
			flowpipe_t newFlowpipe = computeForwardTimeClosure(nextInitialSet->second);

			collectedReachableStates.emplace_back(std::make_pair(nextInitialSet->second.getLocation()->hash(), newFlowpipe));
		}

		return collectedReachableStates;
	}


	template<typename Number, typename ReacherSettings, typename State>
	typename ReachRHPn<Number,ReacherSettings,State>::flowpipe_t ReachRHPn<Number,ReacherSettings,State>::computeForwardTimeClosure(const State& _state ) {
		assert(!_state.getTimestamp().isUnbounded());
#ifdef REACH_DEBUG
		INFO("hypro.reacher", "Location: " << _state.getLocation()->hash());
		INFO("hypro.reacher", "Location printed : " << *(_state.getLocation()));
		INFO("hypro.reacher", "Time step size: " << mSettings.timeStep);
		INFO("hypro.reacher", "Initial valuation: " << _state);
		//std::cout << boost::get<State>(_state) << std::endl;
		//std::cout << _state << std::endl;
#endif
		// new empty Flowpipe
		typename ReachRHPn<Number,ReacherSettings,State>::flowpipe_t flowpipe;
		std::vector<boost::tuple<hypro::Transition<Number>*, State>> nextInitialSets;

		boost::tuple<hypro::CONTAINMENT, State, matrix_t<Number>, vector_t<Number>, Box<Number>> initialSetup = computeFirstSegment<Number,State>(_state, mSettings.timeStep);
#ifdef REACH_DEBUG
		INFO("hypro.reacher", "Valuation fulfills Invariant?: " << boost::get<0>(initialSetup));
#endif
		if ( boost::get<0>(initialSetup) != CONTAINMENT::NO ) { // see convenienceOperators for details
			assert(!boost::get<1>(initialSetup).getTimestamp().isUnbounded());
			bool noFlow = false;

			// if the location does not have dynamic behaviour, check guards and exit loop.
			if(boost::get<2>(initialSetup) == matrix_t<Number>::Identity(boost::get<2>(initialSetup).rows(), boost::get<2>(initialSetup).cols()) &&
				boost::get<3>(initialSetup) == vector_t<Number>::Zero(boost::get<3>(initialSetup).rows())) {
				noFlow = true;
				// Collect potential new initial states from discrete behaviour.
				if(int(mCurrentLevel) <= mSettings.jumpDepth || mSettings.jumpDepth < 0) {
					INFO("hypro.reacher", "-- Checking Transitions from initial!");
					checkTransitions(_state, carl::Interval<tNumber>(tNumber(0),mSettings.timeBound), nextInitialSets);
				}
			}

			// insert first Segment into the empty flowpipe
			State currentSegment;
			if(noFlow) {
				currentSegment = _state;
			} else {
				currentSegment = boost::get<1>(initialSetup);
			}
			flowpipe.push_back( currentSegment );

			// Check for bad states intersection. The first segment is validated against the invariant, already.
			if(intersectBadStates(currentSegment)){
				// clear queue to stop whole algorithm
				while(!mWorkingQueue.isEmpty()){
					mWorkingQueue.dequeueFront();
				}
				return flowpipe;
			}

			// Set after linear transformation
			State nextSegment;
#ifdef USE_SYSTEM_SEPARATION
			State autonomPart = currentSegment;
#ifdef USE_ELLIPSOIDS
			// Easy to addapt to any State use ellipsoid for the idea of my masterthesis here
			Ellipsoid<Number> nonautonomPart(mBloatingFactor, currentSegment.dimension());
			Ellipsoid<Number> totalBloating = nonautonomPart;
#else
			Ellipsoid<Number> nonautonomPartAsEllispsoid(mBloatingFactor, currentSegment.dimension());
			State nonautonomPart = State(nonautonomPartAsEllispsoid);
			State totalBloating = nonautonomPart;
#endif
#endif
#ifdef REACH_DEBUG
			if(!noFlow){
				INFO("hypro.reacher", "--- Loop entered ---");
			}
#endif

			// the first segment covers one time step already
			tNumber currentLocalTime = mSettings.timeStep;
			// intersection of bad states and violation of invariant is handled inside the loop
			while( !noFlow && currentLocalTime <= mSettings.timeBound ) {
				INFO("hypro.reacher","Time: " << std::setprecision(4) << std::setw(8) << fixed << carl::toDouble(currentLocalTime));
				// Verify transitions on the current set.
				if(int(mCurrentLevel) < mSettings.jumpDepth || mSettings.jumpDepth < 0) {
					//State currentState = _state;
					State currentState = currentSegment;
					//std::cout << "-- Checking Transitions!" << std::endl;
					checkTransitions(currentState, currentState.getTimestamp(), nextInitialSets);
				}

				// perform linear transformation on the last segment of the flowpipe
#ifdef USE_SYSTEM_SEPARATION
				autonomPart = autonomPart.partiallyApplyTimeStep( ConstraintSet<Number>(boost::get<2>(initialSetup), boost::get<3>(initialSetup)), mSettings.timeStep, 0 );
#ifdef USE_ELLIPSOIDS
				if (mBloatingFactor != 0){
					Representation temp = Representation(totalBloating);
					nextSegment = autonomPart.minkowskiSum(temp);
				} else {
					nextSegment = autonomPart;
				}
#else
				if (mBloatingFactor != 0){
					nextSegment = autonomPart.minkowskiSum(totalBloating);
				} else {
					nextSegment = autonomPart;
				}
#endif
				// nonautonomPart = nonautonomPart.linearTransformation( boost::get<2>(initialSetup), vector_t<Number>::Zero(autonomPart.dimension()));
				nonautonomPart = nonautonomPart.linearTransformation(boost::get<2>(initialSetup));
				totalBloating = totalBloating.minkowskiSum(nonautonomPart);
#else
				nextSegment =  currentSegment.partiallyApplyTimeStep(ConstraintSet<Number>(boost::get<2>(initialSetup), boost::get<3>(initialSetup)), mSettings.timeStep, 0);
#endif
				// extend flowpipe (only if still within Invariant of location)
				std::pair<hypro::CONTAINMENT, State> newSegment = nextSegment.satisfies( _state.getLocation()->getInvariant());

#ifdef REACH_DEBUG
				INFO("hypro.reacher", "Next Flowpipe Segment: " << newSegment.second);
				INFO("hypro.reacher", "still within Invariant?: " << newSegment.first);
#endif
				if ( newSegment.first != CONTAINMENT::NO ) {
					flowpipe.push_back( newSegment.second );
					if(intersectBadStates(newSegment.second)){
						// clear queue to stop whole algorithm
						while(!mWorkingQueue.isEmpty()){
							mWorkingQueue.dequeueFront();
						}
						return flowpipe;
					}
					// update currentSegment
					currentSegment = newSegment.second;
				} else {
					// the flowpipe does not longer satisfy the invariant -> quit loop
					break;
				}
				currentLocalTime += mSettings.timeStep;
			}
#ifdef REACH_DEBUG
			if(!noFlow){
				INFO("hypro.reacher", "--- Loop left ---");
			}
			INFO("hypro.reacher", "flowpipe: " << flowpipe.size() << " Segments computed.");
			INFO("hypro.reacher", "Process " << nextInitialSets.size() << " new initial sets.");
			INFO("hypro.reacher", "current level: " << mCurrentLevel);
#endif
			// The loop terminated correctly (i.e. no bad states were hit), process discrete behavior.
			assert(nextInitialSets.empty() || (int(mCurrentLevel) < mSettings.jumpDepth || mSettings.jumpDepth < 0));
			if(int(mCurrentLevel) < mSettings.jumpDepth || mSettings.jumpDepth < 0){
				processDiscreteBehaviour(nextInitialSets);
			}
			return flowpipe;
		} else {
			// return an empty flowpipe
			return flowpipe;
		}
	}

//} // namespace reachability
} // namespace hypro
