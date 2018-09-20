#include "ParseHybridPetrinet.h"

using namespace std;
namespace hpnmg {
    ParseHybridPetrinet::ParseHybridPetrinet() = default;

    ParseHybridPetrinet::~ParseHybridPetrinet() = default;

    shared_ptr<ParametricLocationTree>
    ParseHybridPetrinet::parseHybridPetrinet(shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime) {
        // TODO: all floats to double?
        discretePlaceIDs = {};
        continuousPlaceIDs = {};
        deterministicTransitionIDs = {};
        generalTransitionIDs = {};

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

        while (!locationQueue.empty()) {
            processNode(locationQueue[0], hybridPetrinet, maxTime);
            locationQueue.erase(locationQueue.begin());
        }

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

        vector<double> drift = getDrift(rootDiscreteMarking, rootContinuousMarking, hybridPetrinet);

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
            gtEnabled[i] = transitionIsEnabled(rootDiscreteMarking, rootContinuousMarking, gt, hybridPetrinet);
        }
        rootLocation.setGeneralTransitionsEnabled(gtEnabled);

        return rootLocation;
    }

    void ParseHybridPetrinet::processNode(ParametricLocationTree::Node node, shared_ptr<HybridPetrinet> hybridPetrinet,
                                          double maxTime) {
        ParametricLocation location = node.getParametricLocation();
        vector<int> discreteMarking = location.getDiscreteMarking();
        vector<vector<double>> continuousMarking = location.getContinuousMarking();
        map<string, shared_ptr<ImmediateTransition>> immediateTransitions = hybridPetrinet->getImmediateTransitions();

        // step 1: Immediate Transition have highest priority, so we consider them first
        vector<shared_ptr<ImmediateTransition>> enabledImmediateTransition;
        unsigned long highestPriority = ULONG_MAX;
        for (auto &immediateTransition : immediateTransitions) {
            shared_ptr<ImmediateTransition> transition = immediateTransition.second;
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
                if (transition->getPriority() < highestPriority) {
                    highestPriority = transition->getPriority();
                    // priority is higher (number is smaller), so we don't consider transitions with lower priority
                    enabledImmediateTransition.clear();
                    enabledImmediateTransition.push_back(transition);
                } else if (transition->getPriority() == highestPriority) {
                    enabledImmediateTransition.push_back(transition);
                }
            }
        }
        if (!enabledImmediateTransition.empty()) {
            float sumWeight = 0;
            for (shared_ptr<ImmediateTransition> &transition : enabledImmediateTransition)
                sumWeight += transition->getWeight();
            for (shared_ptr<ImmediateTransition> &transition : enabledImmediateTransition)
                addLocationForImmediateEvent(transition, node, transition->getWeight() / sumWeight, hybridPetrinet);
            for (ParametricLocationTree::Node &childNode : parametriclocationTree->getChildNodes(node)) {
                locationQueue.push_back(childNode);
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
                if (!timeDelta.empty())
                    timeDeltas.push_back(timeDelta);
            }
        }
        for (auto transitionItem : generalTransitions) {
            shared_ptr<GeneralTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                if (!timeDelta.empty())
                    timeDeltas.push_back(timeDelta);
            }
        }
        for (auto transitionItem : deterministicTransitions) {
            shared_ptr<DeterministicTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                if (!timeDelta.empty())
                    timeDeltas.push_back(timeDelta);
            }
        }
        for (auto transitionItem : hybridPetrinet->getContinuousTransitions()) {
            shared_ptr<ContinuousTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                vector<double> timeDelta = getTimeDelta(arcItem.second, generalTransitionsFired,
                                                        generalIntervalBoundLeft, generalIntervalBoundRight, levels,
                                                        drifts);
                if (!timeDelta.empty())
                    timeDeltas.push_back(timeDelta);
            }
        }
        // 2.2 Deterministic transitions
        for (int pos = 0; pos < deterministicTransitionIDs.size(); ++pos) {
            shared_ptr<DeterministicTransition> transition = deterministicTransitions[deterministicTransitionIDs[pos]];
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
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
                if (duplicate)
                    timeDeltas.erase(timeDeltas.begin() + i);
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
                    if (timeDelta[0] < timeDelta2[0])
                        timeDeltas.erase(timeDeltas.begin() + j);
                    else
                        timeDeltas.erase(timeDeltas.begin() + i);

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
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
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
                double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                    generalIntervalBoundRight, timeDelta);
                if (minimumTime <= minimalMaximum) {
                    alreadyConsidered.push_back(timeDelta);
                    addLocationForBoundaryEvent(timeDelta, timeDeltas, node, hybridPetrinet);
                }
            }
        }

        // Order II: firing of deterministic transition
        // get enabled deterministic transitions (we ignore priority)
        vector<vector<double>> newConsidered; // todo: we should order them by time Delta
        vector<pair<shared_ptr<DeterministicTransition>, vector<double>>> nextDeterministicTransitions;
        for (int pos = 0; pos < deterministicTransitionIDs.size(); ++pos) {
            shared_ptr<DeterministicTransition> transition = deterministicTransitions[deterministicTransitionIDs[pos]];
            if (!transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet))
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
        double sumWeight = 0;
        for (pair<shared_ptr<DeterministicTransition>, vector<double>> transitionItem : nextDeterministicTransitions)
            sumWeight += transitionItem.first->getWeight();
        for (pair<shared_ptr<DeterministicTransition>, vector<double>> transitionItem : nextDeterministicTransitions) {
            double probability = transitionItem.first->getWeight() / sumWeight;
            addLocationForDeterministicEvent(transitionItem.first, probability, transitionItem.second, timeDeltas,
                                             node, hybridPetrinet);
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
            if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) != alreadyConsidered.end())
                continue;
            double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                generalIntervalBoundRight, timeDelta);
            if (minimumTime <= minimalMaximum) {
                newConsideredPlace.push_back(timeDelta);
                addLocationForBoundaryEvent(timeDelta, timeDeltas, node, hybridPetrinet);
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
                    if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) != alreadyConsidered.end())
                        continue;
                    double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                        generalIntervalBoundRight, timeDelta);
                    if (minimumTime <= minimalMaximum) {
                        newConsideredPlace.push_back(timeDelta);
                        addLocationForBoundaryEvent(timeDelta, timeDeltas, node, hybridPetrinet);
                    }
                }
            }
        }
        // add new considered
        for (vector<double> timeDelta : newConsideredPlace)
            if (find(alreadyConsidered.begin(), alreadyConsidered.end(), timeDelta) == alreadyConsidered.end())
                alreadyConsidered.push_back(timeDelta);

        // Order IV: guard arc for deterministic or general transition
        vector<pair<double, shared_ptr<DeterministicTransition>>> nextDeterministicGuards;
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
                    addLocationForBoundaryEvent(timeDelta, timeDeltas, node, hybridPetrinet);
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
                double minimumTime = getBoundedTime(generalTransitionsFired, generalIntervalBoundLeft,
                                                    generalIntervalBoundRight, timeDelta);
                if (minimumTime <= minimalMaximum) {
                    addLocationForBoundaryEvent(timeDelta, timeDeltas, node, hybridPetrinet);
                }
            }
        }

        for (ParametricLocationTree::Node &childNode : parametriclocationTree->getChildNodes(node)) {
            int nodeMax = 5000;
            if (childNode.getNodeID() <= nodeMax) { // to avoid zeno behavior
                locationQueue.push_back(childNode);
                if (childNode.getNodeID() == nodeMax)
                    cout << nodeMax << " locations or more, some locations may not be shown" << endl;
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
        // drift is negative and level is over arc weight
        if (drift < 0 && weight < minimumLevel) {
            // remaining time is (level - arc) / abs(drift)
            vector<double> levelDelta = level;
            levelDelta[0] -= weight;
            vector<double> timeDelta;
            for (double deltaPart : levelDelta) {
                timeDelta.push_back(deltaPart / (0 - drift));
            }
            return timeDelta;
            // drift is positive and level is under arc weight
        } else if (drift > 0 && weight > maximumLevel) {
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
                if (value >= 0)
                    time[i] += value * boundaries[i];
                else
                    time[i] += value * oppositeBoundaries[i];

        }
        return time[0];
    }

    bool ParseHybridPetrinet::transitionIsEnabled(vector<int> discreteMarking, vector<vector<double>> continousMarking,
                                                  shared_ptr<Transition> transition,
                                                  shared_ptr<HybridPetrinet> hybridPetrinet) {
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
                if ((marking > arc->weight && arc->getIsInhibitor()) ||
                    ((marking < arc->weight && !arc->getIsInhibitor())))
                    return false;
            } else { // place is continuous
                shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[arc->place->id];
                long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), place->id) -
                           continuousPlaceIDs.begin();
                // todo: can be dependent on random variables
                double level = continousMarking[pos][0];
                if ((level > arc->weight && arc->getIsInhibitor()) || ((level < arc->weight && !arc->getIsInhibitor())))
                    return false;
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
        vector<double> drift = getDrift(markings, continuousMarking, hybridPetrinet);

        // add new Location
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();
        double eventTime = parentNode.getParametricLocation().getSourceEvent().getTime();
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();

        Event event = Event(EventType::Immediate, generalDependecies, eventTime);
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
            gtEnabled[i] = transitionIsEnabled(markings, continuousMarking, gt, hybridPetrinet);
        }
        newLocation.setGeneralTransitionsEnabled(gtEnabled);

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
                                    hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet)) {
                for (int j = 0; j < generalIntervalBoundLeft[i].size() && j < timeDelta.size(); ++j)
                    generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1][j] += timeDelta[j];
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
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet);

        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();

        double eventTime = parentLocation.getSourceEvent().getTime() + timeDelta[0];
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();
        for (int i = 1; i < timeDelta.size(); ++i)
            generalDependecies[i - 1] += timeDelta[i];
        Event event = Event(EventType::Timed, generalDependecies, eventTime);
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
                                     hybridPetrinet))
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
                                     hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet))
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
            gtEnabled[i] = transitionIsEnabled(discreteMarking, continuousMarking, gt, hybridPetrinet);
        }
        newLocation.setGeneralTransitionsEnabled(gtEnabled);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    void ParseHybridPetrinet::addLocationForBoundaryEvent(vector<double> timeDelta, vector<vector<double>> timeDeltas,
                                                          ParametricLocationTree::Node parentNode,
                                                          shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();

        // adjust boundaries
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            if (transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                    hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet)) {
                for (int j = 0; j < generalIntervalBoundLeft[i].size() && j < timeDelta.size(); ++j)
                    generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1][j] += timeDelta[j];
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
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet);
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();

        double eventTime = parentLocation.getSourceEvent().getTime() + timeDelta[0];
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();
        for (int i = 1; i < timeDelta.size(); ++i)
            generalDependecies[i - 1] += timeDelta[i];
        Event event = Event(EventType::Continuous, generalDependecies, eventTime);
        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i) {
            if (!transitionIsEnabled(discreteMarking, parentContinuousMarking,
                                     hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]],
                                     hybridPetrinet))
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
                                     hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet))
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
            gtEnabled[i] = transitionIsEnabled(discreteMarking, continuousMarking, gt, hybridPetrinet);
        }
        newLocation.setGeneralTransitionsEnabled(gtEnabled);

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

        // adjust left boundaries for enabled transitions
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            string currentId = generalTransitionIDs[i];
            if (currentId == transition->id)
                continue;
            if (!transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                     hybridPetrinet->getGeneralTransitions()[currentId], hybridPetrinet))
                continue;
            generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1].push_back(1);
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
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet);

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
                    currentBoundRight[currentBoundRight.size() - 1] = generalClocks[i];
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

        Event event = Event(EventType::General, generalDependecies, eventTime);
        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i) {
            if (!transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                     hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[i]],
                                     hybridPetrinet)) {
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
                                     hybridPetrinet)) {
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
            gtEnabled[i] = transitionIsEnabled(discreteMarking, continuousMarking, gt, hybridPetrinet);
        }
        newLocation.setGeneralTransitionsEnabled(gtEnabled);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    vector<double> ParseHybridPetrinet::getDrift(vector<int> discreteMarking, vector<vector<double>> continuousMarking,
                                                 shared_ptr<HybridPetrinet> hybridPetrinet) {
        vector<double> drift(continuousMarking.size());
        vector<double> inputDrift(continuousMarking.size());
        vector<double> outputDrift(continuousMarking.size());
        fill(drift.begin(), drift.end(), 0);
        fill(inputDrift.begin(), inputDrift.end(), 0);
        fill(outputDrift.begin(), outputDrift.end(), 0);
        // get drift for every place
        auto continuousTransitions = hybridPetrinet->getContinuousTransitions();
        for (auto &continuousTransition : continuousTransitions) {
            shared_ptr<ContinuousTransition> transition = continuousTransition.second;
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
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
        //map<string, double> transitionRate;
        map<string, double> outputDriftNeeded;
        map<string, double> inputDriftNeeded;
        while (!placesToCheck.empty()) {
            shared_ptr<ContinuousPlace> place = placesToCheck[0];
            long pos =
                    find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), place->id) - continuousPlaceIDs.begin();
            // Check if we need to adapt the rate
            // Case 1: we have to adapt the output transition (place is empty and drift is negative)
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
            if (drift[pos] < 0 && placeEmpty) {
                double leftOutputRate = inputDrift[pos];
                outputDrift[pos] = inputDrift[pos];
                drift[pos] = 0;
                map<unsigned long, vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>>> transitionsByPrio;
                // Get Out-Transitions sorted by prio
                for (auto transitionItem : hybridPetrinet->getContinuousTransitions()) {
                    shared_ptr<ContinuousTransition> transition = transitionItem.second;
                    shared_ptr<ContinuousArc> arc;
                    bool isInputTransition = false;
                    for (auto arcItem : transition->getContinuousInputArcs())
                        if (arcItem.second->place->id == place->id) {
                            isInputTransition = true;
                            arc = arcItem.second;
                        }
                    if (isInputTransition) {
                        unsigned long priority = arc->getPriority();
                        transitionsByPrio[priority].push_back(make_tuple(transition, arc));
                    }
                }
                // Adapt rate for some transitions
                for (auto &iter : transitionsByPrio) {
                    vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>> prioTransitions = iter.second;
                    double sumOutRate = 0.0;
                    double sumShare = 0;
                    for (tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>> transItem : prioTransitions) {
                        shared_ptr<ContinuousTransition> transition = get<0>(transItem);
                        shared_ptr<ContinuousArc> arc = get<1>(transItem);
                        sumOutRate += arc->weight * transition->getRate();
                        sumShare += arc->getShare();
                    }

                    if (sumOutRate <= leftOutputRate) { // we have enough fluid left for this priority
                        leftOutputRate -= sumOutRate;
                    } else { // we do not have enough left
                        // adapt transition rates and add places to placesToCheck
                        for (tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>> transItem : prioTransitions) {
                            shared_ptr<ContinuousTransition> transition = get<0>(transItem);
                            shared_ptr<ContinuousArc> arc = get<1>(transItem);
                            double newRate = leftOutputRate * arc->getShare() / sumShare;
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
                                        long plasePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                             placeToCheck->id) - continuousPlaceIDs.begin();
                                        inputDrift[plasePos] -= rateDiff;
                                        drift[plasePos] -= rateDiff;
                                    }
                                }
                                for (auto arcItem : transition->getContinuousInputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(
                                                hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long plasePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                             placeToCheck->id) - continuousPlaceIDs.begin();
                                        outputDrift[plasePos] -= rateDiff;
                                        drift[plasePos] += rateDiff;
                                    }
                                }
                            }
                        }
                        leftOutputRate = 0;
                    }
                }
                placesToCheck.erase(placesToCheck.begin());
            }
                // Case 2: we have to adapt the input transitions (place is full and drift is positive)
            else if (!place->getInfiniteCapacity() && drift[pos] > 0 && placeFull) {
                double leftInputRate = outputDrift[pos];
                inputDrift[pos] = outputDrift[pos];
                drift[pos] = 0;
                map<unsigned long, vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>>> transitionsByPrio;
                // Get In-Transitions sorted by prio
                for (auto transitionItem : hybridPetrinet->getContinuousTransitions()) {
                    shared_ptr<ContinuousTransition> transition = transitionItem.second;
                    shared_ptr<ContinuousArc> arc;
                    bool isOutputTransition = false;
                    for (auto arcItem : transition->getContinuousOutputArcs())
                        if (arcItem.second->place->id == place->id) {
                            isOutputTransition = true;
                            arc = arcItem.second;
                        }
                    if (isOutputTransition) {
                        unsigned long priority = arc->getPriority();
                        transitionsByPrio[priority].push_back(make_tuple(transition, arc));
                    }
                }
                // Adapt rate for some transitions
                for (auto &iter : transitionsByPrio) {
                    vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>> prioTransitions = iter.second;
                    double sumInRate = 0.0;
                    double sumShare = 0;
                    for (tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>> transItem : prioTransitions) {
                        shared_ptr<ContinuousTransition> transition = get<0>(transItem);
                        shared_ptr<ContinuousArc> arc = get<1>(transItem);
                        sumInRate += arc->weight * transition->getRate();
                        sumShare += arc->getShare();
                    }

                    if (sumInRate <= leftInputRate) { // we have enough fluid left for this priority
                        leftInputRate -= sumInRate;
                    } else { // we do not have enough left
                        // adapt transition rates and add places to placesToCheck
                        for (tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>> transItem : prioTransitions) {
                            shared_ptr<ContinuousTransition> transition = get<0>(transItem);
                            shared_ptr<ContinuousArc> arc = get<1>(transItem);
                            double newRate = leftInputRate * arc->getShare() / sumShare;
                            if (transition->getRate() != newRate) {
                                double rateDiff = transition->getRate() - newRate;
                                if (inputDriftNeeded.find(transition->id) == inputDriftNeeded.end())
                                    inputDriftNeeded[transition->id] = rateDiff;
                                else
                                    inputDriftNeeded[transition->id] += rateDiff;
                                transition->setCurrentRate(newRate);
                                for (auto arcItem : transition->getContinuousOutputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(
                                                hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long plasePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                             placeToCheck->id) - continuousPlaceIDs.begin();
                                        inputDrift[plasePos] -= rateDiff;
                                        drift[plasePos] -= rateDiff;
                                    }
                                }
                                for (auto arcItem : transition->getContinuousInputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(
                                                hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long plasePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                             placeToCheck->id) - continuousPlaceIDs.begin();
                                        outputDrift[plasePos] -= rateDiff;
                                        drift[plasePos] += rateDiff;
                                    }
                                }
                            }
                        }
                        leftInputRate = 0;
                    }
                }
                placesToCheck.erase(placesToCheck.begin());
            } else {
                placesToCheck.erase(placesToCheck.begin());
            }
        }
        return drift;
    }
}