#include "ParseHybridPetrinet.h"
#include "util/logging/Logging.h"
#include "util/statistics/Statistics.h"

using namespace std;
namespace hpnmg {
    ParseHybridPetrinet::ParseHybridPetrinet() = default;

    ParseHybridPetrinet::~ParseHybridPetrinet() = default;

    shared_ptr<ParametricLocationTree>
    ParseHybridPetrinet::parseHybridPetrinet(shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime, int mode) {
        // TODO: all floats to double?
        discretePlaceIDs = {};
        continuousPlaceIDs = {};
        deterministicTransitionIDs = {};
        generalTransitionIDs = {};

        START_BENCHMARK_OPERATION("READ_PLT")
        // Add place IDs from map to vector, so the places have an order
        map<string, shared_ptr<DiscretePlace>> discretePlaces = hybridPetrinet->getDiscretePlaces();
        for (auto &discretePlace : discretePlaces)
            discretePlaceIDs.push_back(discretePlace.first);
        map<string, shared_ptr<ContinuousPlace>> continuousPlaces = hybridPetrinet->getContinuousPlaces();
        for (auto &continuousPlace : continuousPlaces)
            continuousPlaceIDs.push_back(continuousPlace.first);

        // Also for deterministic and general Transitions
        map<string, shared_ptr<DeterministicTransition>> deterministicTransitions = hybridPetrinet->getDeterministicTransitions();
        for (auto &deterministicTransition : deterministicTransitions)
            deterministicTransitionIDs.push_back(deterministicTransition.first);
        map<string, shared_ptr<GeneralTransition>> generalTransitions = hybridPetrinet->getGeneralTransitions();
        for (auto &generalTransition : generalTransitions)
            generalTransitionIDs.push_back(generalTransition.first);

        ParametricLocation rootLocation = generateRootParametricLocation(hybridPetrinet, maxTime);

        parametriclocationTree = make_shared<ParametricLocationTree>(rootLocation, maxTime);

        // Add distributions to plt
        vector<pair<string, map<string, float>>> distributions(generalTransitionIDs.size());
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            shared_ptr<GeneralTransition> transition = hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]];
            distributions[i] = make_pair(transition->getCdf(), transition->getParameter());
        }
        parametriclocationTree->setDistributions(distributions);

        locationQueue.push_back(parametriclocationTree->getRootNode());
        STOP_BENCHMARK_OPERATION("READ_PLT")

        while (!locationQueue.empty()) {
            processNode(locationQueue.front(), hybridPetrinet, maxTime, mode);
            locationQueue.erase(locationQueue.begin());
        }

        INFOLOG("hpnmg.ParseHybridPetrinet", "[Number of locations]: " << parametriclocationTree->getAllLocations().size())

        return parametriclocationTree;
    }

    ParametricLocation
    ParseHybridPetrinet::generateRootParametricLocation(shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime) {
        // Get root discrete markings
        vector<int> rootDiscreteMarking;
        for (string &placeId: discretePlaceIDs)
            rootDiscreteMarking.push_back(static_cast<int>(hybridPetrinet->getDiscretePlaces()[placeId]->getMarking()));

        // Get root continuous marking
        vector<vector<double>> rootContinuousMarking;
        for (string &placeId: continuousPlaceIDs) {
            rootContinuousMarking.push_back({hybridPetrinet->getContinuousPlaces()[placeId]->getLevel()}); // NOLINT
        }


        // Fill left and right bounds
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();
        vector<vector<vector<double>>> leftBoundaries;
        vector<vector<vector<double>>> rightBoundaries;
        vector<vector<double>> generalClocks;
        for (int i = 0; i < numGeneralTransitions; ++i) {
            leftBoundaries.push_back({{0}}); // NOLINT
            rightBoundaries.push_back({{maxTime}}); // NOLINT
            generalClocks.push_back({0}); // NOLINT
        }
        vector<double> drift = getDrift(rootDiscreteMarking, rootContinuousMarking, hybridPetrinet, leftBoundaries,
                                        rightBoundaries, {});
        ParametricLocation rootLocation = ParametricLocation(rootDiscreteMarking, rootContinuousMarking, drift,
                                                             static_cast<int>(numGeneralTransitions),
                                                             Event(EventType::Root, {}, 0),
                                                             leftBoundaries, rightBoundaries);

        vector<vector<double>> deterministicClocks;
        for (string &id : deterministicTransitionIDs)
            deterministicClocks.push_back({0}); // NOLINT

        rootLocation.setDeterministicClock(deterministicClocks);
        rootLocation.setGeneralClock(generalClocks);
        rootLocation.setGeneralTransitionsFired({});
        rootLocation.setConflictProbability(1);
        rootLocation.setAccumulatedProbability(1.0);


        // add bool vector of enabled general transitions
        vector<bool> gtEnabled(generalTransitionIDs.size());
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            shared_ptr<GeneralTransition> gt = hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]];
            gtEnabled[i] = transitionIsEnabled(rootDiscreteMarking, rootContinuousMarking, gt, hybridPetrinet,
                                               leftBoundaries, rightBoundaries, {});
        }
        rootLocation.setGeneralTransitionsEnabled(gtEnabled);

        // add bool vector for enabling status of deterministic transitions
        vector<bool> enablingStatusesOfDeterministicTransitions(deterministicTransitionIDs.size());
        for (int i = 0; i < deterministicTransitionIDs.size(); i++) {
            shared_ptr<DeterministicTransition> detTrans = hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]];
            enablingStatusesOfDeterministicTransitions[i] = transitionIsEnabled(rootDiscreteMarking, rootContinuousMarking, detTrans, hybridPetrinet, leftBoundaries, rightBoundaries, {});
        }
        rootLocation.setDeterministicTransitionsEnabled(enablingStatusesOfDeterministicTransitions);

        return rootLocation;
    }

    bool isEqualTimeDelta(vector<double> first, vector<double> second) {
        if (first.size() != second.size()) {
            return false;
        }
        for (int i = 0; i < first.size(); i++) {
            if (first[i] != second[i]) {
                return false;
            }
        }
        return true;
    }

    std::vector<std::vector<pair<shared_ptr<DeterministicTransition>, vector<double>>>> ParseHybridPetrinet::sortByEqualTimeDelta(std::vector<pair<shared_ptr<DeterministicTransition>, vector<double>>> deterministicTransitions) {
        std::vector<std::vector<pair<shared_ptr<DeterministicTransition>, vector<double>>>> equalTimeDeltaTransitions;
        for (pair<shared_ptr<DeterministicTransition>, vector<double>> dt : deterministicTransitions) {
            bool newTimeDelta = true;
            for (int i = 0; i < equalTimeDeltaTransitions.size(); i++) {
                if (isEqualTimeDelta(dt.second, equalTimeDeltaTransitions[i][0].second)) {
                    equalTimeDeltaTransitions[i].push_back(dt);
                    newTimeDelta = false;
                    break;
                }
            }
            if(newTimeDelta) {
                equalTimeDeltaTransitions.push_back({ dt });
            }
        }
        return equalTimeDeltaTransitions;
    }

    void ParseHybridPetrinet::processNode(ParametricLocationTree::Node node, shared_ptr<HybridPetrinet> hybridPetrinet,
                                          double maxTime, int mode) {

         /* --- Nondeterministic Conflicts ---
         Mode 0: Nondeterminism resolved by priorities and conflicts (default)
         Mode 1: Nondeterminism conflict set only includes transitions of highest priority (ignoring weights)
         Mode 2: Nondeterminism conflict set includes all transitions that might fire (ignoring priorities and weights)
          */

        START_BENCHMARK_OPERATION("CREATE_PARAMETRICLOCATION")
        COUNT_STATS("CREATE_PARAMETRICLOCATION")
        int nodeMax = 100000;
        ParametricLocation location = node.getParametricLocation();
        vector<int> discreteMarking = location.getDiscreteMarking();
        vector<vector<double>> continuousMarking = location.getContinuousMarking();
        map<string, shared_ptr<ImmediateTransition>> immediateTransitions = hybridPetrinet->getImmediateTransitions();

        // step 1: Immediate Transition have highest priority, so we consider them first
        vector<shared_ptr<ImmediateTransition>> enabledImmediateTransition;
        double highestPriority = -1.0;
        for (auto &immediateTransition : immediateTransitions) {
            shared_ptr<ImmediateTransition> transition = immediateTransition.second;

            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet, location.getGeneralIntervalBoundLeft(), location.getGeneralIntervalBoundRight(), location.getGeneralTransitionsFired())) {

                if (mode < 2 && transition->getPriority() > highestPriority) {

                    highestPriority = transition->getPriority();
                    // priority is higher (number is greater), so we don't consider transitions with lower priority
                    enabledImmediateTransition.clear();
                    enabledImmediateTransition.push_back(transition);
                } else if (mode == 2 || transition->getPriority() == highestPriority) {
                    enabledImmediateTransition.push_back(transition);
                }

            }
        }
        if (!enabledImmediateTransition.empty()) {
            if (mode == 0){
                float sumWeight = 0;
                for (shared_ptr<ImmediateTransition> &transition : enabledImmediateTransition)
                    sumWeight += transition->getWeight();
                for (shared_ptr<ImmediateTransition> &transition : enabledImmediateTransition)
                    addLocationForImmediateEvent(transition, node, transition->getWeight() / sumWeight, hybridPetrinet);
            } else  {
                for (shared_ptr<ImmediateTransition> &transition : enabledImmediateTransition)
                    addLocationForImmediateEvent(transition, node, 1.0, hybridPetrinet);
            }

            for (ParametricLocationTree::Node &childNode : parametriclocationTree->getChildNodes(node)) {
                if (childNode.getNodeID() <= nodeMax) { // to avoid zeno behavior
                    locationQueue.push_back(childNode);
                    if (childNode.getNodeID() == nodeMax)
                        WARNLOG("hpnmg.ParseHybridPetriNet", "Maximum number of locations (" << nodeMax << ") reached. Some locations may not be shown.")
                }
            }
            return; // no other event has to be considered
        }

        // extract important data from location
        vector<vector<double>> levels = location.getContinuousMarking();
        vector<double> drifts = location.getDrift();
        vector<int> generalTransitionsFired = location.getGeneralTransitionsFired();
        vector<vector<vector<double>>> generalIntervalBoundLeft = location.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = location.getGeneralIntervalBoundRight();
        map<string, shared_ptr<DeterministicTransition>> deterministicTransitions = hybridPetrinet->getDeterministicTransitions();
        map<string, shared_ptr<GeneralTransition>> generalTransitions = hybridPetrinet->getGeneralTransitions();
        vector<vector<double>> deterministicClocks = location.getDeterministicClock();
        vector<vector<double>> generalClock = location.getGeneralClock();

        // step 2: get all time-delta for timed events
        vector<vector<double>> timeDeltas;
        // 2.1 guard arcs
        for (auto transitionItem : immediateTransitions) {
            shared_ptr<ImmediateTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                //If timeDelta is zero, the guard arc condition event is source event of the current location (or condition was fulfilled at root).
                if (!timeDelta.empty() && (std::any_of(timeDelta.begin(), timeDelta.end(), [](double coefficient){ return coefficient != 0.0;})))
                    timeDeltas.push_back(timeDelta);
            }
        }
        for (auto transitionItem : generalTransitions) {
            shared_ptr<GeneralTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                //If timeDelta is zero, the guard arc condition event is source event of the current location (or condition was fulfilled at root).
                if (!timeDelta.empty() && (std::any_of(timeDelta.begin(), timeDelta.end(), [](double coefficient){ return coefficient != 0.0;})))
                    timeDeltas.push_back(timeDelta);
            }
        }
        for (auto transitionItem : deterministicTransitions) {
            shared_ptr<DeterministicTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                //If timeDelta is zero, the guard arc condition event is source event of the current location (or condition was fulfilled at root).
                if (!timeDelta.empty() && (std::any_of(timeDelta.begin(), timeDelta.end(), [](double coefficient){ return coefficient != 0.0;})))
                    timeDeltas.push_back(timeDelta);
            }
        }
        for (auto transitionItem : hybridPetrinet->getContinuousTransitions()) {
            shared_ptr<ContinuousTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                //If timeDelta is zero, the guard arc condition event is source event of the current location (or condition was fulfilled at root).
                if (!timeDelta.empty() && (std::any_of(timeDelta.begin(), timeDelta.end(), [](double coefficient){ return coefficient != 0.0;})))
                    timeDeltas.push_back(timeDelta);
            }
        }
        // 2.2 Deterministic transitions
        for (int pos = 0; pos < deterministicTransitionIDs.size(); ++pos) {
            shared_ptr<DeterministicTransition> transition = deterministicTransitions[deterministicTransitionIDs[pos]];
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet, location
                    .getGeneralIntervalBoundLeft(), location.getGeneralIntervalBoundRight(), location.getGeneralTransitionsFired())) {
                // get time when transitions are fired (disctime - clock)
                vector<double> timeDelta = deterministicClocks[pos];
                for (int i = 0; i < timeDelta.size(); ++i) {
                    timeDelta[i] = 0 - timeDelta[i];
                    if (i == 0) {
                        timeDelta[i] += transition->getDiscTime();
                    }
                }
                timeDeltas.push_back(timeDelta);
            }
        }
        // 2.3 Continuous Places
        for (int pos = 0; pos < continuousPlaceIDs.size(); ++pos) {
            shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[continuousPlaceIDs[pos]];
            vector<double> level = levels[pos];
            double drift = drifts[pos];
            if (drift < 0) { // negative drift
                // remaining time is level / abs(drift)
                vector<double> timeDelta;
                for (double levelPart : level)
                    timeDelta.push_back(levelPart / (0 - drift));
                timeDeltas.push_back(timeDelta);
            } else if (drift > 0 && !place->getInfiniteCapacity()) { // positive drift
                // remaining time is (maxLevel - level) / drift
                vector<double> timeDelta;
                for (int i = 0; i < level.size(); ++i) {
                    double value = 0 - level[i];
                    if (i == 0)
                        value += place->getCapacity();
                    timeDelta.push_back(value / drift);
                }

                timeDeltas.push_back(timeDelta);
            }
        }

        // step 3: get all minimal time-delta and save minimal maximum
        // 3.1 get minimal maximumTime (maxTime - minimal location time)
        vector<double> timeGenDep = location.getSourceEvent().getGeneralDependencies();
        vector<double> locTime(timeGenDep.size() + 1);
        locTime[0] = location.getSourceEvent().getTime();
        for (int i = 1; i < locTime.size(); ++i)
            locTime[i] = timeGenDep[i - 1];
        double minimalMaximum = maxTime -
                                getBoundedTime(generalTransitionsFired, location.getGeneralIntervalBoundLeft(),
                                               location.getGeneralIntervalBoundRight(), locTime);
        for (vector<double> &timeDelta : timeDeltas) {
            double maximalTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundRight,
                                                generalIntervalBoundLeft, timeDelta);
            if (maximalTime < minimalMaximum)
                minimalMaximum = maximalTime;
        }
        // 3.1 get all minimum times
        vector<vector<double>> minimalTimeDeltas;
        for (vector<double> &timeDelta : timeDeltas) {
            double minimalTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                generalIntervalBoundRight, timeDelta);
            if (minimalTime <= minimalMaximum)
                minimalTimeDeltas.push_back(timeDelta);
        }
        timeDeltas = minimalTimeDeltas;
        // 3.2 remove duplicates
        for (int i = 1; i < timeDeltas.size(); ++i) {
            vector<double> timeDelta = timeDeltas[i];
            for (int j = 0; j < i; ++j) {
                vector<double> timeDelta2 = timeDeltas[j];
                if (timeDelta2.size() != timeDelta.size())
                    continue;
                bool duplicate = true;
                for (int k = 0; k < timeDelta.size(); ++k)
                    if (timeDelta[k] != timeDelta2[k])
                        duplicate = false;
                if (duplicate) {
                    timeDeltas.erase(timeDeltas.begin() + i);
                    i--;
                }
            }
        }

        // Example: delta1 = 5-S0 and delta2 = 12 - S0
        // step 3.3: Remove minimal time delta with same general dependencies
        for (int i = 1; i < timeDeltas.size(); ++i) {
            vector<double> timeDelta = timeDeltas[i];
            for (int j = 0; j < i; ++j) {
                vector<double> timeDelta2 = timeDeltas[j];
                if (timeDelta2.size() != timeDelta.size())
                    continue;
                bool duplicate = true;
                for (int k = 1; k < timeDelta.size(); ++k)
                    if (timeDelta[k] != timeDelta2[k])
                        duplicate = false;
                if (duplicate) {
                    if (timeDelta[0] < timeDelta2[0]) {
                        timeDeltas.erase(timeDeltas.begin() + j);
                        i--;
                        j--;
                    }
                    else {
                        timeDeltas.erase(timeDeltas.begin() + i);
                        i--;
                        j--;
                    }

                }
            }
        }
        // Step 3.3: Remove minimal time delta that have too high minimum
        vector<vector<double>> allowedTimeDeltas;
        for (vector<double> timeDelta : timeDeltas) {
            vector<double> newLocTime;
            for (int i=0; i<timeDelta.size() || i < locTime.size(); ++i) {
                if (i >= timeDelta.size())
                    newLocTime.push_back(locTime[i]);
                else if (i >= locTime.size())
                    newLocTime.push_back(timeDelta[i]);
                else
                    newLocTime.push_back(locTime[i] + timeDelta[i]);
            }
            if (maxTime >= getBoundedTime(generalTransitionsFired, location.getGeneralIntervalBoundLeft(),
                                          location.getGeneralIntervalBoundRight(), newLocTime))
                allowedTimeDeltas.push_back(timeDelta);
        }
        timeDeltas = allowedTimeDeltas;

        // Step 3.4: Check for all time delta td if another time delta is smaller
        // Therefore, for every tdi in timeDeltas we check if max(ti -td) < 0
        // If max(ti - td) < 0 the timeDelta is too high
        vector<vector<double>> allowedTimeDeltas2;
        for (vector<double> timeDelta : timeDeltas) {
            bool valid = true;
            for (vector<double> timeDelta_i : timeDeltas) {
                vector<double> newLocTime;
                for (int i = 0; i < timeDelta.size() || i < timeDelta_i.size(); ++i) {
                    if (i >= timeDelta.size())
                        newLocTime.push_back(timeDelta_i[i]);
                    else if (i >= locTime.size())
                        newLocTime.push_back(-timeDelta[i]);
                    else
                        newLocTime.push_back(timeDelta_i[i] - timeDelta[i]);
                }
                if (0 > getBoundedTime(generalTransitionsFired, location.getGeneralIntervalBoundRight(),
                                       location.getGeneralIntervalBoundLeft(), newLocTime))
                    valid = false;
            }
            if (valid)
                allowedTimeDeltas2.push_back(timeDelta);
        }
        timeDeltas = allowedTimeDeltas2;

        // step 4: add parametric locations for every general transition
        for (auto transitionItem : generalTransitions) {
            shared_ptr<GeneralTransition> transition = transitionItem.second;
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet, location
                    .getGeneralIntervalBoundLeft(), location.getGeneralIntervalBoundRight(), location.getGeneralTransitionsFired())) {
                // if we have no minimal timeDelta we use maxTime
                if (timeDeltas.empty())
                    addLocationForGeneralEvent(transition, maxTime, {maxTime}, {{}}, node, hybridPetrinet);

                // for every general transition we need one parametric location per minimal timedelta
                for (int i = 0; i < timeDeltas.size(); ++i)
                    addLocationForGeneralEvent(transition, maxTime, timeDeltas[i], timeDeltas, node,
                                               hybridPetrinet);
            }
        }

        // step 5: add one parametric location for every timed event with minimal time delta
        // if minimum of time-delta equals minimalMaximum consider order
        // todo: das hier muss anders sein, pro minimal time delta muss die Order beachtet werden!
        vector<vector<double>> alreadyConsidered(timeDeltas.size()); // add all timeDeltas, that already were taken to consider order

        // Order I: guard arcs for immediate transitions
        for (auto transitionItem : immediateTransitions) {
            shared_ptr<ImmediateTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                if (timeDelta.empty())
                    continue;
                // timedelta is not minimal
                if (find(timeDeltas.begin(), timeDeltas.end(), timeDelta) == timeDeltas.end())
                    continue;
                if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) != alreadyConsidered.end())
                    continue;
                double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                    generalIntervalBoundRight, timeDelta);
                if (minimumTime <= minimalMaximum) {
                    alreadyConsidered.push_back(timeDelta);
                    addLocationForBoundaryEventByArcMember(arcItem.second, timeDelta, timeDeltas, node, hybridPetrinet);
                    //addLocationForBoundaryEvent(timeDelta, timeDeltas, node, hybridPetrinet, arcItem.second->place->id);
                }
            }
        }

        // Order II: firing of deterministic transition
        // get enabled deterministic transitions
        vector<vector<double>> newConsidered; // todo: we should order them by time Delta
        vector<pair<shared_ptr<DeterministicTransition>, vector<double>>> nextDeterministicTransitions;

        /* --- Nondeterministic Conflicts ---
        Mode 0: Nondeterminism resolved by priorities and conflicts (default)
        Mode 1: Nondeterminism conflict set only includes transitions of highest priority (ignoring weights)
        Mode 2: Nondeterminism conflict set includes all transitions that might fire (ignoring priorities and weights)
         */

        highestPriority = -1.0;
        for (int pos = 0; pos < deterministicTransitionIDs.size(); ++pos) {
            shared_ptr<DeterministicTransition> transition = deterministicTransitions[deterministicTransitionIDs[pos]];
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet, location.getGeneralIntervalBoundLeft(), location.getGeneralIntervalBoundRight(), location.getGeneralTransitionsFired())) {
                if (mode < 2 && transition->getPriority() > highestPriority)
                    highestPriority = transition->getPriority();
            }
        }


        for (int pos = 0; pos < deterministicTransitionIDs.size(); ++pos) {
            shared_ptr<DeterministicTransition> transition = deterministicTransitions[deterministicTransitionIDs[pos]];
            if (!transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet, location
                    .getGeneralIntervalBoundLeft(), location.getGeneralIntervalBoundRight(), location.getGeneralTransitionsFired()))
                continue;

            if (mode < 2 && transition->getPriority() < highestPriority)

                continue;
            if (transition->getPriority() < highestPriority)
                continue;
            vector<double> clock = location.getDeterministicClock()[pos];
            vector<double> timeDelta;
            for (int i = 0; i < clock.size(); ++i) {
                double value = 0 - clock[i];
                if (i == 0)
                    value += transition->getDiscTime();
                timeDelta.push_back(value);
            }
            // timedelta is not minimal
            if (find(timeDeltas.begin(), timeDeltas.end(), timeDelta) == timeDeltas.end())
                continue;
            // timedelta was used with higher order
            if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) != alreadyConsidered.end())
                continue;
            double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                generalIntervalBoundRight, timeDelta);
            if (minimumTime <= minimalMaximum) {
                newConsidered.push_back(timeDelta);
                nextDeterministicTransitions.push_back(make_pair(transition, timeDelta)); // NOLINT
            }
        }
        // Only resolve conflicts for next Deterministic events with equal time delta
        std::vector<std::vector<pair<shared_ptr<DeterministicTransition>, vector<double>>>> orderedNextDeterministicTransitions = this->sortByEqualTimeDelta(nextDeterministicTransitions);
        for (std::vector<pair<shared_ptr<DeterministicTransition>, vector<double>>> equalNextTransitions : orderedNextDeterministicTransitions) {

            if (mode == 0) {
                double sumWeight = 0;
                for (pair<shared_ptr<DeterministicTransition>, vector<double>> transitionItem : equalNextTransitions)
                    sumWeight += transitionItem.first->getWeight();
                for (pair<shared_ptr<DeterministicTransition>, vector<double>> transitionItem : equalNextTransitions) {
                    double probability = transitionItem.first->getWeight() / sumWeight;
                    addLocationForDeterministicEvent(transitionItem.first, probability, transitionItem.second, timeDeltas, node, hybridPetrinet);
                }
            } else {
                for (pair<shared_ptr<DeterministicTransition>, vector<double>> transitionItem : equalNextTransitions) {
                    addLocationForDeterministicEvent(transitionItem.first, 1.0, transitionItem.second, timeDeltas, node,
                                                     hybridPetrinet);
                }

            }
        }

        // add new considered
        for (vector<double> timeDelta : newConsidered)
            if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) == alreadyConsidered.end())
                alreadyConsidered.push_back(timeDelta);

        // Order III: ContinuousPlace reaches boundary or guard arc for continuous transition
        vector<vector<double>> newConsideredPlace;
        for (int pos = 0; pos < continuousPlaceIDs.size(); ++pos) {
            shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[continuousPlaceIDs[pos]];
            vector<double> level = levels[pos];

            vector<double> timeDelta;
            double drift = drifts[pos];
            if (drift < 0) { // negative drift
                // remaining time is level / abs(drift)
                for (double levelPart : level)
                    timeDelta.push_back(levelPart / (0 - drift));
            } else if (drift > 0 && !place->getInfiniteCapacity()) { // positive drift
                // remaining time is (maxLevel - level) / drift
                for (int i = 0; i < level.size(); ++i) {
                    double value = 0 - level[i];
                    if (i == 0)
                        value += place->getCapacity();
                    timeDelta.push_back(value / drift);
                }
            } else {
                continue;
            }
            // timedelta is not minimal
            if (find(timeDeltas.begin(), timeDeltas.end(), timeDelta) == timeDeltas.end())
                continue;
            // timedelta was used with higher order
            if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) != alreadyConsidered.end() ||
            find(newConsideredPlace.begin(), newConsideredPlace.end(), timeDelta) != newConsideredPlace.end())
                continue;
            double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                generalIntervalBoundRight, timeDelta);
            if (minimumTime <= minimalMaximum) {
                newConsideredPlace.push_back(timeDelta);
                addLocationForBoundaryEventByContinuousPlaceMember(place, timeDelta, timeDeltas, node, hybridPetrinet);
                //addLocationForBoundaryEvent(timeDelta, timeDeltas, node, hybridPetrinet, place->id);
            }
            vector<pair<double, shared_ptr<ContinuousTransition>>> nextContinuousGuards;
            for (auto transitionItem : hybridPetrinet->getContinuousTransitions()) {
                shared_ptr<ContinuousTransition> transition = transitionItem.second;
                for (auto arcItem : transition->getGuardInputArcs()) {
                    vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                            drifts);
                    if (timeDelta.empty())
                        continue;
                    // timedelta is not minimal
                    if (find(timeDeltas.begin(), timeDeltas.end(), timeDelta) == timeDeltas.end())
                        continue;
                    // timedelta was used with higher order
                    if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) != alreadyConsidered.end()||
                        find(newConsideredPlace.begin(), newConsideredPlace.end(), timeDelta) != newConsideredPlace.end())
                        continue;
                    double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                        generalIntervalBoundRight, timeDelta);
                    if (minimumTime <= minimalMaximum) {
                        newConsideredPlace.push_back(timeDelta);
                        addLocationForBoundaryEventByArcMember(arcItem.second, timeDelta, timeDeltas, node, hybridPetrinet);
                        //addLocationForBoundaryEvent(timeDelta, timeDeltas, node, hybridPetrinet, arcItem.second->id);
                    }
                }
            }
        }
        // add new considered
        for (vector<double> timeDelta : newConsideredPlace)
            if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) == alreadyConsidered.end())
                alreadyConsidered.push_back(timeDelta);

        // Order IV: guard arc for deterministic or general transition
        vector<vector<double>> nextDeterministicGuards;
        for (auto transitionItem : hybridPetrinet->getDeterministicTransitions()) {
            shared_ptr<DeterministicTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                if (timeDelta.empty())
                    continue;
                // timedelta is not minimal
                if (find(timeDeltas.begin(), timeDeltas.end(), timeDelta) == timeDeltas.end())
                    continue;
                // timedelta was used with higher order
                if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) != alreadyConsidered.end())
                    continue;
                double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                    generalIntervalBoundRight, timeDelta);
                if (minimumTime <= minimalMaximum) {
                    alreadyConsidered.push_back(timeDelta);
                    addLocationForBoundaryEventByArcMember(arcItem.second, timeDelta, timeDeltas, node, hybridPetrinet);
                }
            }
        }
        vector<pair<double, shared_ptr<GeneralTransition>>> nextGeneralGuards;
        for (auto transitionItem : hybridPetrinet->getGeneralTransitions()) {
            shared_ptr<GeneralTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                if (timeDelta.empty())
                    continue;
                if (find(timeDeltas.begin(), timeDeltas.end(), timeDelta) == timeDeltas.end())
                    continue;
                if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) != alreadyConsidered.end())
                    continue;
                double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                    generalIntervalBoundRight, timeDelta);
                if (minimumTime <= minimalMaximum) {
                    alreadyConsidered.push_back(timeDelta);
                    addLocationForBoundaryEventByArcMember(arcItem.second, timeDelta, timeDeltas, node, hybridPetrinet);
                }
            }
        }

        STOP_BENCHMARK_OPERATION("CREATE_PARAMETRICLOCATION")

        for (ParametricLocationTree::Node &childNode : parametriclocationTree->getChildNodes(node)) {
            if (childNode.getNodeID() <= nodeMax) { // to avoid zeno behavior
                locationQueue.push_back(childNode);
                if (childNode.getNodeID() == nodeMax) {
                    WARNLOG("hpnmg.ParseHybridPetriNet",
                            "Maximum number of locations (" << nodeMax << ") reached. Some locations may not be shown.")
                }
            }
        }
    }

    vector<double> ParseHybridPetrinet::getTimeDelta(shared_ptr<GuardArc> arc, vector<int> generalTransitionsFired,
                                                     vector<vector<vector<double>>> generalIntervalBoundLeft,
                                                     vector<vector<vector<double>>> generalIntervalBoundRight,
                                                     vector<vector<double>> levels, vector<double> drifts) {
        double weight = arc->weight;
        string placeId = arc->place->id;
        long placePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeId) -
                        continuousPlaceIDs.begin();
        if (placePos >= continuousPlaceIDs.size())
            return {};
        vector<double> level = levels[placePos];
        double drift = drifts[placePos];
        double minimumLevel = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                             generalIntervalBoundRight, level);
        double maximumLevel = getBoundedTime(generalTransitionsFired, generalIntervalBoundRight,
                                             generalIntervalBoundLeft, level);

        // an inhibited guard arc should not have an effect when drift is positive
        if (drift > 0 && arc->getIsInhibitor()) {
            return {};
        }
        // a guard arc should not have an effect when drift is negative
        if (drift < 0 && !arc->getIsInhibitor()) {
            return {};
        }


        // drift is negative and level is over arc weight
        if (drift < 0 && weight <= minimumLevel) {
            // remaining time is (level - arc) / abs(drift)
            vector<double> levelDelta = level;

            levelDelta[0] -= weight;
            vector<double> timeDelta;
            for (double deltaPart : levelDelta) {
                timeDelta.push_back(deltaPart / (0 - drift));
            }
            return timeDelta;
        // drift is positive and level is under arc weight
        } else if (drift > 0 && weight >= maximumLevel) {
            // remaining time is (arc - level) / drift
            vector<double> timeDelta;
            for (int i = 0; i < level.size(); ++i) {
                double value = 0 - level[i];
                if (i == 0)
                    value += weight;
                timeDelta.push_back(value / drift);
            }
            return timeDelta;
        } else {
            return {};
        }
    }

    vector<double> ParseHybridPetrinet::computeUnequationCut(vector<double> time1, vector<double> time2) {
        unsigned long minSize = min(time1.size(), time2.size());
        long k = -1;
        double diffK = 0.0;
        for (long i = minSize - 1; i >= 0; --i) {
            if (time1[i] != time2[i]) {
                k = i;
                diffK = time2[i] - time1[i];
                break;
            }
        }
        if (k < 1 || diffK == 0.0)
            return {};
        vector<double> unequationCut;
        for (int i = 0; i < k; ++i)
            unequationCut.push_back((time1[i] - time2[i]) / diffK);
        return unequationCut;
    }

    double ParseHybridPetrinet::getBoundedTime(vector<int> generalTransitionsFired,
                                               vector<vector<vector<double>>> generalBounds,
                                               vector<vector<vector<double>>> oppositeGeneralBounds,
                                               vector<double> time) {
        assert(time.size() == generalTransitionsFired.size() + 1);
        vector<int> counter = vector<int>(generalTransitionIDs.size());
        fill(counter.begin(), counter.end(), 0);
        for (int &firedTransition : generalTransitionsFired)
            counter[firedTransition] += 1;
        while (time.size() > 1) {
            // get last value in timevector
            double value = time.back();
            time.pop_back();

            // get boundaries of last value
            int transitionPos = generalTransitionsFired[time.size() - 1];
            int firingTime = counter[transitionPos] - 1;
            vector<double> boundaries = generalBounds[transitionPos][firingTime];
            vector<double> oppositeBoundaries = oppositeGeneralBounds[transitionPos][firingTime];
            counter[transitionPos] -= 1;

            // add boundaries multiplicated with value to time vector
            for (int i = 0; i < time.size(); ++i)
                if (value >= 0) {
                    time[i] += i < boundaries.size() ? value * boundaries[i] : 0;
                }
                else {
                    time[i] += i < oppositeBoundaries.size() ? value * oppositeBoundaries[i] : 0;
                }

        }
        return time[0];
    }


    bool ParseHybridPetrinet::transitionIsEnabled(vector<int> discreteMarking, vector<vector<double>> continousMarking,
                                                  shared_ptr<Transition> transition,
                                                  shared_ptr<HybridPetrinet> hybridPetrinet,
        vector<vector<vector<double>>> lowerBounds, vector<vector<vector<double>>>
            upperBounds, vector<int> generalTransitionsFired) {
        // discrete arcs are enabled when place has higher or equal marking than arcs weight
        // place is discrete Place
        for (pair<string, shared_ptr<DiscreteArc>> arc_entry : transition->getDiscreteInputArcs()) {
            shared_ptr<DiscreteArc> arc = arc_entry.second;
            shared_ptr<DiscretePlace> place = hybridPetrinet->getDiscretePlaces()[arc->place->id];
            long pos = find(discretePlaceIDs.begin(), discretePlaceIDs.end(), place->id) - discretePlaceIDs.begin();
            double marking = discreteMarking[pos];
            if (arc->weight > marking)
                return false;
        }
        for (pair<string, shared_ptr<GuardArc>> arc_entry : transition->getGuardInputArcs()) {
            shared_ptr<GuardArc> arc = arc_entry.second;
            // place is discrete
            if (hybridPetrinet->getDiscretePlaces().count(arc->place->id)) {
                shared_ptr<DiscretePlace> place = hybridPetrinet->getDiscretePlaces()[arc->place->id];
                long pos = find(discretePlaceIDs.begin(), discretePlaceIDs.end(), place->id) - discretePlaceIDs.begin();
                double marking = discreteMarking[pos];
                if ((marking >= arc->weight && arc->getIsInhibitor()) || ((marking < arc->weight && !arc->getIsInhibitor())))
                    return false;
            }  else { // place is continuous
                shared_ptr <ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[arc->place->id];
                long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), place->id) -
                           continuousPlaceIDs.begin();

                vector<double> level = continousMarking[pos];
                double drift = getDrift(discreteMarking, continousMarking, hybridPetrinet,
                                        lowerBounds,
                                        upperBounds, generalTransitionsFired)[pos];


                double maxLevel = getBoundedTime(generalTransitionsFired, upperBounds, lowerBounds, level);
                double minLevel = getBoundedTime(generalTransitionsFired, lowerBounds, upperBounds, level);

                if (arc->getIsInhibitor()) {

                    if (arc->weight > 0.0) {

                        if (drift >= 0.0 && (maxLevel >= arc->weight || minLevel >= arc->weight))
                            return false;

                        if (maxLevel > arc->weight || minLevel > arc->weight)
                            return false;

                    } else {

                        if (drift > 0.0)
                            return false;

                        if (drift >= 0.0 && (maxLevel > 0.0 || minLevel > 0.0))
                            return false;
                    }

                } else {


                    if (arc->weight > 0.0){

                        if (drift < 0.0 && (minLevel <= arc->weight || maxLevel <= arc->weight))
                            return false;

                        if (minLevel < arc->weight || maxLevel < arc->weight)
                            return false;

                    } else {

                        if (drift <= 0.0 && (minLevel <= 0.0 || maxLevel <= 0.0))
                            return false;

                    }
                }
            }
        }
        return true;
    }

    void ParseHybridPetrinet::addLocationForImmediateEvent(shared_ptr<ImmediateTransition> transition,
                                                           ParametricLocationTree::Node parentNode, float probability,
                                                           shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();
        // get new markings
        vector<int> markings = parentNode.getParametricLocation().getDiscreteMarking();
        for (auto arcItem : transition->getDiscreteInputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos =
                    find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            markings[pos] -= arc->weight;
        }
        for (auto arcItem : transition->getDiscreteOutputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos =
                    find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            markings[pos] += arc->weight;
        }

        // get new drift
        vector<vector<double>> continuousMarking = parentLocation.getContinuousMarking();
        vector<double> drift = getDrift(markings, continuousMarking, hybridPetrinet,
                                        parentLocation.getGeneralIntervalBoundLeft(),
                                        parentLocation.getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired() );

        // add new Location
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();
        double eventTime = parentNode.getParametricLocation().getSourceEvent().getTime();
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();

        Event event = Event(EventType::Immediate, generalDependecies, eventTime);
        event.setImmediateTransitionMember(transition);

        ParametricLocation newLocation = ParametricLocation(markings, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            parentLocation.getGeneralIntervalBoundLeft(),
                                                            parentLocation.getGeneralIntervalBoundRight());

        newLocation.setDeterministicClock(parentNode.getParametricLocation().getDeterministicClock());
        newLocation.setGeneralClock(parentNode.getParametricLocation().getGeneralClock());
        newLocation.setGeneralTransitionsFired(parentNode.getParametricLocation().getGeneralTransitionsFired());
        newLocation.setConflictProbability(probability);
        newLocation.setAccumulatedProbability(1);

        // add bool vector of enabled general transitions
        vector<bool> gtEnabled(generalTransitionIDs.size());
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            shared_ptr<GeneralTransition> gt = hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]];
            gtEnabled[i] = transitionIsEnabled(markings, continuousMarking, gt, hybridPetrinet, parentLocation
                    .getGeneralIntervalBoundLeft(), parentLocation.getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired());
        }
        newLocation.setGeneralTransitionsEnabled(gtEnabled);

        // add bool vector for enabling status of deterministic transitions
        vector<bool> enablingStatusesOfDeterministicTransitions(deterministicTransitionIDs.size());
        for (int i = 0; i < deterministicTransitionIDs.size(); i++) {
            shared_ptr<DeterministicTransition> detTrans = hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]];
            enablingStatusesOfDeterministicTransitions[i] = transitionIsEnabled(markings, continuousMarking, detTrans, hybridPetrinet, parentLocation.getGeneralIntervalBoundLeft(), parentLocation.getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired());
        }
        newLocation.setDeterministicTransitionsEnabled(enablingStatusesOfDeterministicTransitions);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    void ParseHybridPetrinet::addLocationForDeterministicEvent(shared_ptr<DeterministicTransition> transition,
                                                               double probability, vector<double> timeDelta,
                                                               vector<vector<double>> timeDeltas,
                                                               ParametricLocationTree::Node parentNode,
                                                               shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();

        // adjust boundaries
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            if (transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                    hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet,
                                    parentLocation.getGeneralIntervalBoundLeft(), parentLocation
                                                                                         .getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired())) {
                for (int j = 0; j < timeDelta.size(); ++j) {
                    if (generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1].size() <= j)
                        generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1].push_back(timeDelta[j]);
                    else
                        generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1][j] += timeDelta[j];
                }
            }
        }
        // adjust general bounds for every other timeDelta, than timeDelta
        vector<int> generalTransitionsFired = parentLocation.getGeneralTransitionsFired();
        if (timeDeltas.size() > 1) {
            for (vector<double> oppositeTimeDelta : timeDeltas) {
                if (oppositeTimeDelta == timeDelta)
                    continue;
                vector<double> unequationCut = computeUnequationCut(timeDelta, oppositeTimeDelta);
                long k = unequationCut.size();
                // adjust bounds for random variable that fired as kth general transition
                if (k > 0) {
                    int generalTransitionPos = generalTransitionsFired[k - 1];
                    int counter = 0;
                    for (int j = 0; j < k - 1; ++j)
                        if (generalTransitionsFired[j] == generalTransitionPos)
                            ++counter;
                    if (timeDelta[k] > oppositeTimeDelta[k])
                        generalIntervalBoundRight[generalTransitionPos][counter] = unequationCut;
                    else
                        generalIntervalBoundLeft[generalTransitionPos][counter] = unequationCut;
                }
            }
        }

        // get new markings
        vector<int> discreteMarking = parentLocation.getDiscreteMarking();
        for (auto arcItem : transition->getDiscreteInputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos =
                    find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            discreteMarking[pos] -= arc->weight;
        }
        for (auto arcItem : transition->getDiscreteOutputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos =
                    find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            discreteMarking[pos] += arc->weight;
        }

        // get new drift
        vector<vector<double>> parentContinuousMarking = parentLocation.getContinuousMarking();
        vector<double> parentDrift = parentLocation.getDrift();
        vector<vector<double>> continuousMarking;
        for (int pos = 0; pos < parentContinuousMarking.size(); ++pos) {
            double drift = parentDrift[pos];
            vector<double> oldMarking = parentContinuousMarking[pos];
            vector<double> newMarking;
            for (int j = 0; j < oldMarking.size() || j < timeDelta.size(); ++j) {
                if (j >= oldMarking.size())
                    newMarking.push_back(timeDelta[j] * drift);
                else if (j >= timeDelta.size())
                    newMarking.push_back(oldMarking[j]);
                else
                    newMarking.push_back(oldMarking[j] + timeDelta[j] * drift);
            }
            continuousMarking.push_back(newMarking);
        }
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet,
                                        generalIntervalBoundLeft, generalIntervalBoundRight, generalTransitionsFired);

        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();

        double eventTime = parentLocation.getSourceEvent().getTime() + timeDelta[0];
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();
        for (int i = 1; i < timeDelta.size(); ++i)
            generalDependecies[i - 1] += timeDelta[i];

        Event event = Event(EventType::Timed, generalDependecies, eventTime);
        event.setDeterministicTransitionMember(transition);

        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i) {
            if (deterministicTransitionIDs[i] == transition->id) {
                vector<double> initVector{0.0};
                for (int j = 0; j < generalTransitionsFired.size(); ++j) {
                    initVector.push_back(0.0);
                }
                deterministicClocks[i] = initVector;
                continue;
            }
            if (!transitionIsEnabled(parentLocation.getDiscreteMarking(), parentContinuousMarking,
                                     hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]],
                                     hybridPetrinet, parentLocation.getGeneralIntervalBoundLeft(),
                                     parentLocation.getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired()))
                continue;
            for (int j = 0; j < deterministicClocks[i].size() || j < timeDelta.size(); ++j) {
                if (j >= deterministicClocks[i].size())
                    deterministicClocks[i].push_back(timeDelta[j]);
                else if (j >= timeDelta.size())
                    continue;
                else
                    deterministicClocks[i][j] = deterministicClocks[i][j] + timeDelta[j];
            }
        }

        // get new general clocks
        vector<vector<double>> generalClocks = parentLocation.getGeneralClock();
        for (int i = 0; i < generalClocks.size(); ++i) {
            if (!transitionIsEnabled(parentLocation.getDiscreteMarking(), parentContinuousMarking,
                                     hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]],
                                     hybridPetrinet,parentLocation.getGeneralIntervalBoundLeft(),
                        parentLocation.getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired()))
                continue;
            for (int j = 0; j < generalClocks[i].size() || j < timeDelta.size(); ++j) {
                if (j >= generalClocks[i].size())
                    generalClocks[i].push_back(timeDelta[j]);
                else if (j >= timeDelta.size())
                    continue;
                else
                    generalClocks[i][j] = generalClocks[i][j] + timeDelta[j];
            }
        }

        newLocation.setDeterministicClock(deterministicClocks);
        newLocation.setGeneralClock(generalClocks);
        newLocation.setGeneralTransitionsFired(parentNode.getParametricLocation().getGeneralTransitionsFired());
        newLocation.setConflictProbability(probability);
        newLocation.setAccumulatedProbability(1);

        // add bool vector of enabled general transitions
        vector<bool> gtEnabled(generalTransitionIDs.size());
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            shared_ptr<GeneralTransition> gt = hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]];
            gtEnabled[i] = transitionIsEnabled(discreteMarking, continuousMarking, gt, hybridPetrinet,
                                               generalIntervalBoundLeft, generalIntervalBoundRight, parentLocation.getGeneralTransitionsFired());
        }
        newLocation.setGeneralTransitionsEnabled(gtEnabled);

        // add bool vector for enabling status of deterministic transitions
        vector<bool> enablingStatusesOfDeterministicTransitions(deterministicTransitionIDs.size());
        for (int i = 0; i < deterministicTransitionIDs.size(); i++) {
            shared_ptr<DeterministicTransition> detTrans = hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]];
            enablingStatusesOfDeterministicTransitions[i] = transitionIsEnabled(discreteMarking, continuousMarking, detTrans, hybridPetrinet, generalIntervalBoundLeft, generalIntervalBoundRight, parentLocation.getGeneralTransitionsFired());
        }
        newLocation.setDeterministicTransitionsEnabled(enablingStatusesOfDeterministicTransitions);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    void ParseHybridPetrinet::addLocationForBoundaryEventByArcMember(shared_ptr<GuardArc> arcMember, vector<double> timeDelta, vector<vector<double>> timeDeltas,
                                                          ParametricLocationTree::Node parentNode,
                                                          shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();

        // adjust boundaries
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            if (transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                                hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet,
                                                parentLocation.getGeneralIntervalBoundLeft(), parentLocation.getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired()
                        )) {
                for (int j = 0; j < timeDelta.size(); ++j) {
                    if (j >= generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1].size())
                        generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1].push_back(timeDelta[j]);
                    else
                        generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1][j] += timeDelta[j];
                }
            }
        }
        // adjust general bounds for every other timeDelta, than timeDelta
        vector<int> generalTransitionsFired = parentLocation.getGeneralTransitionsFired();
        if (timeDeltas.size() > 1) {
            for (vector<double> &oppositeTimeDelta : timeDeltas) {
                if (oppositeTimeDelta == timeDelta)
                    continue;
                vector<double> unequationCut = computeUnequationCut(timeDelta, oppositeTimeDelta);
                long k = unequationCut.size();
                // adjust bounds for random variable that fired as kth general transition
                if (k > 0) {
                    int generalTransitionPos = generalTransitionsFired[k - 1];
                    int counter = 0;
                    for (int j = 0; j < k - 1; ++j)
                        if (generalTransitionsFired[j] == generalTransitionPos)
                            ++counter;
                    if (timeDelta[k] > oppositeTimeDelta[k])
                        generalIntervalBoundRight[generalTransitionPos][counter] = unequationCut;
                    else
                        generalIntervalBoundLeft[generalTransitionPos][counter] = unequationCut;
                }
            }
        }

        // get new markings
        vector<vector<double>> parentContinuousMarking = parentLocation.getContinuousMarking();
        vector<double> parentDrift = parentLocation.getDrift();
        vector<int> discreteMarking = parentLocation.getDiscreteMarking();
        vector<vector<double>> continuousMarking;
        for (int pos = 0; pos < parentContinuousMarking.size(); ++pos) {
            double drift = parentDrift[pos];
            vector<double> oldMarking = parentContinuousMarking[pos];
            vector<double> newMarking;
            for (int j = 0; j < oldMarking.size() || j < timeDelta.size(); ++j) {
                if (j >= oldMarking.size())
                    newMarking.push_back(timeDelta[j] * drift);
                else if (j >= timeDelta.size())
                    newMarking.push_back(oldMarking[j]);
                else
                    newMarking.push_back(oldMarking[j] + timeDelta[j] * drift);
            }
            continuousMarking.push_back(newMarking);
        }
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet,
                                                generalIntervalBoundLeft, generalIntervalBoundRight, parentLocation.getGeneralTransitionsFired());
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();

        double eventTime = parentLocation.getSourceEvent().getTime() + timeDelta[0];
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();
        for (int i = 1; i < timeDelta.size(); ++i)
            generalDependecies[i - 1] += timeDelta[i];
        Event event = Event(EventType::Continuous, generalDependecies, eventTime);
        event.setArcMember(arcMember);
        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i) {
            if (!transitionIsEnabled(discreteMarking, parentContinuousMarking,
                                                 hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]],
                                                 hybridPetrinet, parentLocation.getGeneralIntervalBoundLeft(),parentLocation
                                                         .getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired()))
                continue;
            for (int j = 0; j < deterministicClocks[i].size() || j < timeDelta.size(); ++j) {
                if (j >= deterministicClocks[i].size())
                    deterministicClocks[i].push_back(timeDelta[j]);
                else if (j >= timeDelta.size())
                    continue;
                else
                    deterministicClocks[i][j] = deterministicClocks[i][j] + timeDelta[j];
            }
        }

        // get new general clocks
        vector<vector<double>> generalClocks = parentLocation.getGeneralClock();
        for (int i = 0; i < generalClocks.size(); ++i) {
            if (!transitionIsEnabled(discreteMarking, parentContinuousMarking,
                                                 hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]],
                                                 hybridPetrinet, parentLocation.getGeneralIntervalBoundLeft(),parentLocation
                                                         .getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired()))
                continue;
            for (int j = 0; j < generalClocks[i].size() || j < timeDelta.size(); ++j) {
                if (j >= generalClocks[i].size())
                    generalClocks[i].push_back(timeDelta[j]);
                else if (j >= timeDelta.size())
                    continue;
                else
                    generalClocks[i][j] = generalClocks[i][j] + timeDelta[j];
            }
        }

        newLocation.setDeterministicClock(deterministicClocks);
        newLocation.setGeneralClock(generalClocks);
        newLocation.setGeneralTransitionsFired(parentNode.getParametricLocation().getGeneralTransitionsFired());
        newLocation.setConflictProbability(1);
        newLocation.setAccumulatedProbability(1);

        // add bool vector of enabled general transitions
        vector<bool> gtEnabled(generalTransitionIDs.size());
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            shared_ptr<GeneralTransition> gt = hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]];
            gtEnabled[i] = transitionIsEnabled(discreteMarking, continuousMarking, gt, hybridPetrinet,
                                                           generalIntervalBoundLeft,generalIntervalBoundRight, parentLocation.getGeneralTransitionsFired());
        }
        newLocation.setGeneralTransitionsEnabled(gtEnabled);

        // add bool vector for enabling status of deterministic transitions
        vector<bool> enablingStatusesOfDeterministicTransitions(deterministicTransitionIDs.size());
        for (int i = 0; i < deterministicTransitionIDs.size(); i++) {
            shared_ptr<DeterministicTransition> detTrans = hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]];
            enablingStatusesOfDeterministicTransitions[i] = transitionIsEnabled(discreteMarking, continuousMarking, detTrans, hybridPetrinet, generalIntervalBoundLeft,generalIntervalBoundRight, parentLocation.getGeneralTransitionsFired());
        }
        newLocation.setDeterministicTransitionsEnabled(enablingStatusesOfDeterministicTransitions);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    void ParseHybridPetrinet::addLocationForBoundaryEventByContinuousPlaceMember(shared_ptr<ContinuousPlace> placeMember, vector<double> timeDelta, vector<vector<double>> timeDeltas,
                                                          ParametricLocationTree::Node parentNode,
                                                          shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();

        // adjust boundaries
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            if (transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                    hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet,
                                    parentLocation.getGeneralIntervalBoundLeft(), parentLocation.getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired()
            )) {
                for (int j = 0; j < timeDelta.size(); ++j) {
                    if (generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1].size() <= j)
                        generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1].push_back(timeDelta[j]);
                    else
                        generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1][j] += timeDelta[j];
                }
            }
        }
        // adjust general bounds for every other timeDelta, than timeDelta
        vector<int> generalTransitionsFired = parentLocation.getGeneralTransitionsFired();
        if (timeDeltas.size() > 1) {
            for (vector<double> &oppositeTimeDelta : timeDeltas) {
                if (oppositeTimeDelta == timeDelta)
                    continue;
                vector<double> unequationCut = computeUnequationCut(timeDelta, oppositeTimeDelta);
                long k = unequationCut.size();
                // adjust bounds for random variable that fired as kth general transition
                if (k > 0) {
                    int generalTransitionPos = generalTransitionsFired[k - 1];
                    int counter = 0;
                    for (int j = 0; j < k - 1; ++j)
                        if (generalTransitionsFired[j] == generalTransitionPos)
                            ++counter;
                    if (timeDelta[k] > oppositeTimeDelta[k])
                        generalIntervalBoundRight[generalTransitionPos][counter] = unequationCut;
                    else
                        generalIntervalBoundLeft[generalTransitionPos][counter] = unequationCut;
                }
            }
        }

        // get new markings
        vector<vector<double>> parentContinuousMarking = parentLocation.getContinuousMarking();
        vector<double> parentDrift = parentLocation.getDrift();
        vector<int> discreteMarking = parentLocation.getDiscreteMarking();
        vector<vector<double>> continuousMarking;
        for (int pos = 0; pos < parentContinuousMarking.size(); ++pos) {
            double drift = parentDrift[pos];
            vector<double> oldMarking = parentContinuousMarking[pos];
            vector<double> newMarking;
            for (int j = 0; j < oldMarking.size() || j < timeDelta.size(); ++j) {
                if (j >= oldMarking.size())
                    newMarking.push_back(timeDelta[j] * drift);
                else if (j >= timeDelta.size())
                    newMarking.push_back(oldMarking[j]);
                else
                    newMarking.push_back(oldMarking[j] + timeDelta[j] * drift);
            }
            continuousMarking.push_back(newMarking);
        }
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet,
                                        generalIntervalBoundLeft, generalIntervalBoundRight, parentLocation.getGeneralTransitionsFired());
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();

        double eventTime = parentLocation.getSourceEvent().getTime() + timeDelta[0];
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();
        for (int i = 1; i < timeDelta.size(); ++i)
            generalDependecies[i - 1] += timeDelta[i];

        Event event = Event(EventType::Continuous, generalDependecies, eventTime);
        event.setPlaceMember(placeMember);

        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i) {
            if (!transitionIsEnabled(discreteMarking, parentContinuousMarking,
                                     hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]],
                                     hybridPetrinet, parentLocation.getGeneralIntervalBoundLeft(),parentLocation
                                             .getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired()))
                continue;
            for (int j = 0; j < deterministicClocks[i].size() || j < timeDelta.size(); ++j) {
                if (j >= deterministicClocks[i].size())
                    deterministicClocks[i].push_back(timeDelta[j]);
                else if (j >= timeDelta.size())
                    continue;
                else
                    deterministicClocks[i][j] = deterministicClocks[i][j] + timeDelta[j];
            }
        }

        // get new general clocks
        vector<vector<double>> generalClocks = parentLocation.getGeneralClock();
        for (int i = 0; i < generalClocks.size(); ++i) {
            if (!transitionIsEnabled(discreteMarking, parentContinuousMarking,
                                     hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]],
                                     hybridPetrinet, parentLocation.getGeneralIntervalBoundLeft(),parentLocation
                                             .getGeneralIntervalBoundRight(), parentLocation.getGeneralTransitionsFired()))
                continue;
            for (int j = 0; j < generalClocks[i].size() || j < timeDelta.size(); ++j) {
                if (j >= generalClocks[i].size())
                    generalClocks[i].push_back(timeDelta[j]);
                else if (j >= timeDelta.size())
                    continue;
                else
                    generalClocks[i][j] = generalClocks[i][j] + timeDelta[j];
            }
        }

        newLocation.setDeterministicClock(deterministicClocks);
        newLocation.setGeneralClock(generalClocks);
        newLocation.setGeneralTransitionsFired(parentNode.getParametricLocation().getGeneralTransitionsFired());
        newLocation.setConflictProbability(1);
        newLocation.setAccumulatedProbability(1);

        // add bool vector of enabled general transitions
        vector<bool> gtEnabled(generalTransitionIDs.size());
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            shared_ptr<GeneralTransition> gt = hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]];
            gtEnabled[i] = transitionIsEnabled(discreteMarking, continuousMarking, gt, hybridPetrinet,
                                               generalIntervalBoundLeft,generalIntervalBoundRight, parentLocation.getGeneralTransitionsFired());
        }
        newLocation.setGeneralTransitionsEnabled(gtEnabled);

        // add bool vector for enabling status of deterministic transitions
        vector<bool> enablingStatusesOfDeterministicTransitions(deterministicTransitionIDs.size());
        for (int i = 0; i < deterministicTransitionIDs.size(); i++) {
            shared_ptr<DeterministicTransition> detTrans = hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]];
            enablingStatusesOfDeterministicTransitions[i] = transitionIsEnabled(discreteMarking, continuousMarking, detTrans, hybridPetrinet, generalIntervalBoundLeft,generalIntervalBoundRight, parentLocation.getGeneralTransitionsFired());
        }
        newLocation.setDeterministicTransitionsEnabled(enablingStatusesOfDeterministicTransitions);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    void ParseHybridPetrinet::addLocationForGeneralEvent(shared_ptr<GeneralTransition> transition, double maxTime,
                                                         vector<double> timeDelta, vector<vector<double>> timeDeltas,
                                                         ParametricLocationTree::Node parentNode,
                                                         shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();
        long transitionPos = find(generalTransitionIDs.begin(), generalTransitionIDs.end(), transition->id) -
                             generalTransitionIDs.begin();

        vector<vector<double>> generalClocks = parentLocation.getGeneralClock();
        vector<double> transitionClock = generalClocks[transitionPos];

        vector<int> generalTransitionsFired = parentLocation.getGeneralTransitionsFired();
        vector<int> newGeneralTransitionsFired = generalTransitionsFired;
        newGeneralTransitionsFired.push_back(static_cast<int>(transitionPos));

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i) {
            if (!transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                     hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]],
                                     hybridPetrinet, parentLocation.getGeneralIntervalBoundLeft(),parentLocation
                                         .getGeneralIntervalBoundRight(),generalTransitionsFired)) {
                deterministicClocks[i].push_back(0);
                continue;
            }
            for (int j = 0; j < deterministicClocks[i].size(); ++j)
                deterministicClocks[i][j] -= transitionClock[j];
            deterministicClocks[i].push_back(1);
        }

        // get new general clocks
        for (int i = 0; i < generalClocks.size(); ++i) {
            if (generalTransitionIDs[i] == transition->id) {
                vector<double> initVector = {0.0};
                for (int j = 0; j < generalTransitionsFired.size() + 1; ++j)
                    initVector.push_back(0.0);
                generalClocks[i] = initVector;
                continue;
            }
            if (!transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                     hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]],
                                     hybridPetrinet, parentLocation.getGeneralIntervalBoundLeft(),parentLocation
                                         .getGeneralIntervalBoundRight(),generalTransitionsFired)) {
                generalClocks[i].push_back(0);
                continue;
            }
            if (i == transitionPos) {
                fill(generalClocks[transitionPos].begin(), generalClocks[transitionPos].end(), 0);
                generalClocks[transitionPos].push_back(0);
                continue;
            }
            for (int j = 0; j < generalClocks[i].size(); ++j)
                generalClocks[i][j] -= transitionClock[j];
            generalClocks[i].push_back(1);
        }

        // adjust left boundaries for enabled transitions
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            string currentId = generalTransitionIDs[i];
            if (currentId == transition->id)
                continue; // current transition's bounds will be updated later
            if (!transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                     hybridPetrinet->getGeneralTransitions()[currentId], hybridPetrinet, parentLocation.getGeneralIntervalBoundLeft(),parentLocation
                                             .getGeneralIntervalBoundRight(),generalTransitionsFired))
                continue;
            generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1] = generalClocks[i];
            generalIntervalBoundRight[i][generalIntervalBoundRight[i].size() - 1].push_back(0);
        }

        // get new discrete markings
        vector<int> discreteMarking = parentLocation.getDiscreteMarking();
        for (auto arcItem : transition->getDiscreteInputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos =
                    find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            discreteMarking[pos] -= arc->weight;
        }
        for (auto arcItem : transition->getDiscreteOutputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos =
                    find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            discreteMarking[pos] += arc->weight;
        }

        // get drift and new continuous markings
        vector<double> parentDrift = parentLocation.getDrift();
        vector<vector<double>> continuousMarking = parentLocation.getContinuousMarking();
        for (int pos = 0; pos < continuousMarking.size(); ++pos) {
            for (int timePos = 0; timePos < continuousMarking[pos].size(); ++timePos)
                continuousMarking[pos][timePos] -= transitionClock[timePos] * parentDrift[pos];
            continuousMarking[pos].push_back(parentDrift[pos]);
        }
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet,
                                        generalIntervalBoundLeft, generalIntervalBoundRight,
                                        newGeneralTransitionsFired);

        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();

        double eventTime = parentLocation.getSourceEvent().getTime();
        vector<double> generalDependecies = parentLocation.getSourceEvent().getGeneralDependencies();
        for (int pos = 0; pos < transitionClock.size(); ++pos) {
            if (pos == 0) {
                eventTime -= transitionClock[pos];
                continue;
            }
            generalDependecies[pos - 1] -= transitionClock[pos];
        }
        generalDependecies.push_back(1.0);

        vector<double> maxTimeVector{maxTime};
        // adjust general bounds
        // adjust fired transition bounds
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            if (generalTransitionIDs[i] == transition->id) {
                vector<vector<double>> currentBoundLeft = generalIntervalBoundLeft[i];
                vector<vector<double>> currentBoundRight = generalIntervalBoundRight[i];
                if (timeDelta != maxTimeVector) {
                    currentBoundRight[currentBoundRight.size() - 1] = transitionClock;
                    for (int j = 0;
                         j < currentBoundRight[currentBoundRight.size() - 1].size() && j < timeDelta.size(); ++j)
                        currentBoundRight[currentBoundRight.size() - 1][j] += timeDelta[j];
                }
                vector<double> vectorLeft = vector<double>(currentBoundLeft[currentBoundLeft.size() - 1].size() + 1);
                vector<double> vectorRight = vector<double>(currentBoundRight[currentBoundRight.size() - 1].size() + 1);
                fill(vectorLeft.begin(), vectorLeft.end(), 0);
                fill(vectorRight.begin(), vectorRight.end(), 0);
                vectorRight[0] = maxTime;
                currentBoundLeft.push_back(vectorLeft);
                currentBoundRight.push_back(vectorRight);
                generalIntervalBoundLeft[i] = currentBoundLeft;
                generalIntervalBoundRight[i] = currentBoundRight;
            }
        }
        // adjust bounds for every other timeDelta, than timeDelta
        if (timeDeltas.size() > 1) {
            for (vector<double> &oppositeTimeDelta : timeDeltas) {
                if (oppositeTimeDelta == timeDelta)
                    continue;
                vector<double> unequationCut = computeUnequationCut(timeDelta, oppositeTimeDelta);
                long k = unequationCut.size();
                // adjust bounds for random variable that fired as kth general transition
                if (k > 0) {
                    int generalTransitionPos = generalTransitionsFired[k - 1];
                    int counter = 0;
                    for (int j = 0; j < k - 1; ++j)
                        if (generalTransitionsFired[j] == generalTransitionPos)
                            ++counter;
                    if (timeDelta[k] > oppositeTimeDelta[k])
                        generalIntervalBoundRight[generalTransitionPos][counter] = unequationCut;
                    else
                        generalIntervalBoundLeft[generalTransitionPos][counter] = unequationCut;
                }
            }
        }

        Event event = Event(EventType::General, generalDependecies, eventTime);
        event.setGeneralTransitionMember(transition);
        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        generalTransitionsFired.push_back(static_cast<int>(transitionPos));

        newLocation.setDeterministicClock(deterministicClocks);
        newLocation.setGeneralClock(generalClocks);
        newLocation.setGeneralTransitionsFired(generalTransitionsFired);
        newLocation.setConflictProbability(1);
        newLocation.setAccumulatedProbability(1);

        // add bool vector of enabled general transitions
        vector<bool> gtEnabled(generalTransitionIDs.size());
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            shared_ptr<GeneralTransition> gt = hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]];
            gtEnabled[i] = transitionIsEnabled(discreteMarking, continuousMarking, gt, hybridPetrinet,
                                               generalIntervalBoundLeft,generalIntervalBoundRight,generalTransitionsFired);
        }
        newLocation.setGeneralTransitionsEnabled(gtEnabled);

        // add bool vector for enabling status of deterministic transitions
        vector<bool> enablingStatusesOfDeterministicTransitions(deterministicTransitionIDs.size());
        for (int i = 0; i < deterministicTransitionIDs.size(); i++) {
            shared_ptr<DeterministicTransition> detTrans = hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]];
            enablingStatusesOfDeterministicTransitions[i] = transitionIsEnabled(discreteMarking, continuousMarking, detTrans, hybridPetrinet, generalIntervalBoundLeft,generalIntervalBoundRight, generalTransitionsFired);
        }
        newLocation.setDeterministicTransitionsEnabled(enablingStatusesOfDeterministicTransitions);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    vector<double> ParseHybridPetrinet::getDrift(vector<int> discreteMarking, vector<vector<double>> continuousMarking, shared_ptr<HybridPetrinet> hybridPetrinet, vector<vector<vector<double>>> lowerBounds, vector<vector<vector<double>>> upperBounds, vector<int> generalTransitionsFired) {
        vector<double> drift(continuousMarking.size());
        vector<double> inputDrift(continuousMarking.size());
        vector<double> outputDrift(continuousMarking.size());
        fill(drift.begin(), drift.end(), 0);
        fill(inputDrift.begin(), inputDrift.end(), 0);
        fill(outputDrift.begin(), outputDrift.end(), 0);

        //TODO when adding dynamic transition: first update all static continuous transitions and then set supposed dynamic rates, before computing the drift. For non-piecewise-linear behavior, the derivatives of rates will have to be considered as well.

        // get drift for every place
        auto continuousTransitions = hybridPetrinet->getContinuousTransitions();
        for (auto &continuousTransition : continuousTransitions) {
            shared_ptr<ContinuousTransition> transition = continuousTransition.second;
            transition->resetOriginalRate();
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet, lowerBounds,
                                    upperBounds, generalTransitionsFired)) {
                for (auto arcItem : transition->getContinuousInputArcs()) {
                    shared_ptr<ContinuousArc> arc = arcItem.second;
                    long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), arc->place->id) -
                               continuousPlaceIDs.begin();
                    double driftChange = arc->weight * transition->getRate();
                    string transitionId = transition->id;
                    drift[pos] -= driftChange;
                    outputDrift[pos] += driftChange; // input for transition is output for place
                }
                for (auto arcItem : transition->getContinuousOutputArcs()) {
                    shared_ptr<ContinuousArc> arc = arcItem.second;
                    long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), arc->place->id) -
                               continuousPlaceIDs.begin();
                    double driftChange = arc->weight * transition->getRate();
                    drift[pos] += driftChange;
                    inputDrift[pos] += driftChange; // output for transition is input for place
                }
            }
        }

        // Rate Adaption
        vector<shared_ptr<ContinuousPlace>> placesToCheck;
        for (auto placeItem :  hybridPetrinet->getContinuousPlaces())
            placesToCheck.push_back(placeItem.second);

        map<string, double> outputDriftNeeded;
        map<string, double> inputDriftNeeded;
        while (!placesToCheck.empty()) {
            shared_ptr<ContinuousPlace> place = placesToCheck[0];
            long pos =
                    find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), place->id) - continuousPlaceIDs.begin();

            // Check if we need to adapt the rate
            bool placeEmpty = true;
            bool placeFull = false;
            for (int i = 0; i < continuousMarking[pos].size(); ++i) {
                double levelAtI = continuousMarking[pos][i];
                if (levelAtI > 0.0) {
                    placeEmpty = false; // minimum 1 time is not 0
                    placeFull = false; // place is only full when first level is capacity and rest is 0
                }
                if (i == 0 && levelAtI == place->getCapacity()) {
                    placeFull = true;
                }
            }

            if ((drift[pos] < 0 && placeEmpty) || (!place->getInfiniteCapacity() && drift[pos] > 0 && placeFull)) {

                double rateLeft;
                if (placeEmpty) {
                    rateLeft = inputDrift[pos];
                    outputDrift[pos] = inputDrift[pos];
                } else {
                    rateLeft = outputDrift[pos];
                    inputDrift[pos] = outputDrift[pos];
                }

                drift[pos] = 0;
                map<unsigned long, vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>>> transitionsByPrio;
                // Get Out-Transitions sorted by prio
                for (auto transitionItem : hybridPetrinet->getContinuousTransitions()) {
                    shared_ptr<ContinuousTransition> transition = transitionItem.second;
                    shared_ptr<ContinuousArc> arc;

                    bool isInputOrOutputTransition = false;
                    if (placeEmpty) {
                        for (auto arcItem : transition->getContinuousInputArcs())
                            if (arcItem.second->place->id == place->id) {
                                isInputOrOutputTransition = true;
                                arc = arcItem.second;
                                break;
                            }
                    } else {
                        for (auto arcItem : transition->getContinuousOutputArcs())
                            if (arcItem.second->place->id == place->id) {
                                isInputOrOutputTransition = true;
                                arc = arcItem.second;
                                break;
                            }
                    }
                    if (isInputOrOutputTransition) {
                        unsigned long priority = arc->getPriority();
                        transitionsByPrio[priority].push_back(make_tuple(transition, arc));
                    }
                }
                // Adapt rate for some transitions
                map<unsigned long, vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>>>::reverse_iterator iter = transitionsByPrio.rbegin();
                while (iter != transitionsByPrio.rend()) {
                    vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>> prioTransitions = iter->second;
                    double rateRequired = 0.0;
                    double sumSharedRate = 0.0;
                    for (tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>> transItem : prioTransitions) {
                        shared_ptr<ContinuousTransition> transition = get<0>(transItem);
                        shared_ptr<ContinuousArc> arc = get<1>(transItem);
                        rateRequired += arc->weight * transition->getRate();
                        sumSharedRate += arc->weight * transition->getRate() * arc->getShare();
                    }

                    if (rateRequired <= rateLeft) { // we have enough fluid left for this priority
                        rateLeft -= rateRequired;
                    } else if (rateLeft > 0.0 && prioTransitions.size() > 1){ // we do not have enough left to supply more than one transition

                        bool change = true;
                        while (change)  {
                            change = false;
                            vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>>::iterator  it = prioTransitions.begin();
                            while (it != prioTransitions.end()){
                                shared_ptr<ContinuousTransition> transition = get<0>(*it);
                                shared_ptr<ContinuousArc> arc = get<1>(*it);
                                if (sumSharedRate > 0.0 && rateLeft > 0.0 && (arc->getShare() / sumSharedRate >= 1 / rateLeft)) {
                                    rateLeft -= arc->weight * transition->getRate();
                                    sumSharedRate -= arc->weight * transition->getRate() * arc->getShare();
                                    change = true;
                                    it = prioTransitions.erase(it);
                                    break;
                                } else
                                    ++it;
                            }

                        }


                        // adapt transition rates and add places to placesToCheck
                        for (tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>> transItem : prioTransitions) {
                            shared_ptr <ContinuousTransition> transition = get<0>(transItem);
                            shared_ptr <ContinuousArc> arc = get<1>(transItem);

                            if (sumSharedRate > 0.0 && rateLeft > 0.0 && (arc->getShare() / sumSharedRate < 1 / rateLeft)) {
                                double newRate = transition->getRate() * arc->getShare() * rateLeft / sumSharedRate;
                                rateLeft -= newRate;
                                if (transition->getRate() != newRate) {
                                    double rateDiff = transition->getRate() - newRate;
                                    if (outputDriftNeeded.find(transition->id) == outputDriftNeeded.end())
                                        outputDriftNeeded[transition->id] = rateDiff;
                                    else
                                        outputDriftNeeded[transition->id] += rateDiff;
                                    transition->setCurrentRate(newRate);
                                    for (auto arcItem : transition->getContinuousOutputArcs()) {
                                        shared_ptr <Place> placeToCheck = arcItem.second->place;
                                        if (place->id != placeToCheck->id) {
                                            placesToCheck.push_back(
                                                    hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                            long placePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                                 placeToCheck->id) - continuousPlaceIDs.begin();
                                            inputDrift[placePos] -= rateDiff;
                                            drift[placePos] -= rateDiff;
                                        }
                                    }
                                    for (auto arcItem : transition->getContinuousInputArcs()) {
                                        shared_ptr <Place> placeToCheck = arcItem.second->place;
                                        if (place->id != placeToCheck->id) {
                                            placesToCheck.push_back(
                                                    hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                            long placePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                                 placeToCheck->id) - continuousPlaceIDs.begin();
                                            outputDrift[placePos] -= rateDiff;
                                            drift[placePos] += rateDiff;
                                        }
                                    }
                                }
                            }
                        }

                    } else if (rateLeft <= 0.0 || (prioTransitions.size() == 1)) {   //nothing left or only one transition to supply
                    // adapt transition rates and add places to placesToCheck
                        for (tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>> transItem : prioTransitions) {
                           shared_ptr<ContinuousTransition> transition = get<0>(transItem);
                           shared_ptr<ContinuousArc> arc = get<1>(transItem);

                           double newRate = 0.0;
                           if (rateLeft > 0.0 && prioTransitions.size() == 1)
                                newRate = rateLeft / arc->weight;

                           if (transition->getRate() != newRate) {
                               double rateDiff = transition->getRate() - newRate;
                               if (outputDriftNeeded.find(transition->id) == outputDriftNeeded.end())
                                   outputDriftNeeded[transition->id] = rateDiff;
                               else
                                   outputDriftNeeded[transition->id] += rateDiff;
                               transition->setCurrentRate(newRate);
                               for (auto arcItem : transition->getContinuousOutputArcs()) {
                                   shared_ptr<Place> placeToCheck = arcItem.second->place;
                                   if (place->id != placeToCheck->id) {
                                       placesToCheck.push_back(
                                               hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                       long placePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                            placeToCheck->id) - continuousPlaceIDs.begin();
                                       inputDrift[placePos] -= rateDiff;
                                       drift[placePos] -= rateDiff;
                                   }
                               }
                               for (auto arcItem : transition->getContinuousInputArcs()) {
                                   shared_ptr<Place> placeToCheck = arcItem.second->place;
                                   if (place->id != placeToCheck->id) {
                                       placesToCheck.push_back(
                                               hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                       long placePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                            placeToCheck->id) - continuousPlaceIDs.begin();
                                       outputDrift[placePos] -= rateDiff;
                                       drift[placePos] += rateDiff;
                                   }
                               }
                           }
                        }
                    }
                    iter++;
                }
                placesToCheck.erase(placesToCheck.begin());

            } else {
                placesToCheck.erase(placesToCheck.begin());
            }
        }
        return drift;
    }

    long ParseHybridPetrinet::getIndexOfModelMember(string id, vector<string> vectorOfIDs) const {
        return find(vectorOfIDs.begin(), vectorOfIDs.end(), id) - vectorOfIDs.begin();
    };

    long ParseHybridPetrinet::getIndexOfDiscretePlace(shared_ptr<DiscretePlace> discretePlace) const {return getIndexOfModelMember(discretePlace->id, discretePlaceIDs);};

    long ParseHybridPetrinet::getIndexOfContinuousPlace(shared_ptr<ContinuousPlace> continuousPlace) const {return getIndexOfModelMember(continuousPlace->id, continuousPlaceIDs);};

    long ParseHybridPetrinet::getIndexOfDeterministicTransition(shared_ptr<DeterministicTransition> deterministicTransition) const {return getIndexOfModelMember(deterministicTransition->id, deterministicTransitionIDs);};

    long ParseHybridPetrinet::getIndexOfGeneralTransition(shared_ptr<GeneralTransition> generalTransition) const {return getIndexOfModelMember(generalTransition->id, generalTransitionIDs);};
}