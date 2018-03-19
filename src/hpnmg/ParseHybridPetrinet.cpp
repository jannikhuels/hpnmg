#include "ParseHybridPetrinet.h"

using namespace std;
namespace hpnmg {
    ParseHybridPetrinet::ParseHybridPetrinet() {}

    ParseHybridPetrinet::~ParseHybridPetrinet() {}

    shared_ptr<ParametricLocationTree>
    ParseHybridPetrinet::parseHybridPetrinet(shared_ptr<HybridPetrinet> hybridPetrinet, double maxTime) {
        // TODO: all floats to double?

        // Add place IDs from map to vector, so the places have an order
        map<string, shared_ptr<DiscretePlace>> discretePlaces = hybridPetrinet->getDiscretePlaces();
        for (auto i = discretePlaces.begin(); i != discretePlaces.end(); ++i)
            discretePlaceIDs.push_back(i->first);
        map<string, shared_ptr<ContinuousPlace>> continuousPlaces = hybridPetrinet->getContinuousPlaces();
        for (auto i = continuousPlaces.begin(); i != continuousPlaces.end(); ++i)
            continuousPlaceIDs.push_back(i->first);

        // Also for deterministic and general Transitions
        map<string, shared_ptr<DeterministicTransition>> deterministicTransitions = hybridPetrinet->getDeterministicTransitions();
        for (auto i = deterministicTransitions.begin(); i != deterministicTransitions.end(); ++i)
            deterministicTransitionIDs.push_back(i->first);
        map<string, shared_ptr<GeneralTransition>> generalTransitions = hybridPetrinet->getGeneralTransitions();
        for (auto i = generalTransitions.begin(); i != generalTransitions.end(); ++i)
            generalTransitionIDs.push_back(i->first);

        // Initialize aleatory variable counter for deterministic transitions and continuous places
        for (string id : deterministicTransitionIDs)
            deterTransAleatoryVariables[id] = {};
        for (string id : continuousPlaceIDs)
            contPlaceAleatoryVariables[id] = {};

        ParametricLocation rootLocation = generateRootParametricLocation(hybridPetrinet, maxTime);

        parametriclocationTree = make_shared<ParametricLocationTree>(rootLocation, maxTime);

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
            rootContinuousMarking.push_back({hybridPetrinet->getContinuousPlaces()[placeId]->getLevel()});
        }

        vector<double> drift = getDrift(rootDiscreteMarking, rootContinuousMarking, hybridPetrinet);

        // Fill left and right bounds
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();
        vector<vector<vector<double>>> leftBoundaries;
        vector<vector<vector<double>>> rightBoundaries;
        vector<vector<double>> generalClocks;
        for (int i = 0; i < numGeneralTransitions; ++i) {
            leftBoundaries.push_back({{0}});
            rightBoundaries.push_back({{maxTime}});
            generalClocks.push_back({0});
        }

        ParametricLocation rootLocation = ParametricLocation(rootDiscreteMarking, rootContinuousMarking, drift,
                                                             static_cast<int>(numGeneralTransitions),
                                                             Event(EventType::Root, {}, 0),
                                                             leftBoundaries, rightBoundaries);

        vector<vector<double>> deterministicClocks;
        for (string id : deterministicTransitionIDs)
            deterministicClocks.push_back({0});

        rootLocation.setDeterministicClock(deterministicClocks);
        rootLocation.setGeneralClock(generalClocks);
        rootLocation.setGeneralTransitionsFired({});
        rootLocation.setConflictProbability(1);

        return rootLocation;
    }

    void ParseHybridPetrinet::processNode(ParametricLocationTree::Node node, shared_ptr<HybridPetrinet> hybridPetrinet,
                                          double maxTime) {
        ParametricLocation location = node.getParametricLocation();
        vector<int> discreteMarking = location.getDiscreteMarking();
        vector<vector<double>> continuousMarking = location.getContinuousMarking();

        // Immediate Transition have highest priority, so we consider them first
        vector<shared_ptr<ImmediateTransition>> enabledImmediateTransition;
        unsigned long highestPriority = ULONG_MAX;
        auto immediateTransitions = hybridPetrinet->getImmediateTransitions();
        for (auto i = immediateTransitions.begin(); i != immediateTransitions.end(); ++i) {
            shared_ptr<ImmediateTransition> transition = i->second;
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
            for (ParametricLocationTree::Node childNode : parametriclocationTree->getChildNodes(node)) {
                locationQueue.push_back(childNode);
            }
            return; // no other event has to be considered
        }

        // Now we can consider timed transitions, guard arcs and boundaries of fluid places
        // we need all events that start first (their minimumTime of firing is less than the lowest maximumTime)
        double firstMaximumTime = maxTime; // todo: maybe change
//        vector<double> generalDependencies = node.getParametricLocation().getSourceEvent().getGeneralDependencies();
//        firstMaximumTime.push_back(maxTime - node.getParametricLocation().getSourceEvent().getTime());
//        firstMaximumTime.insert(firstMaximumTime.end(), generalDependencies.begin(), generalDependencies.end() );

        vector<vector<double>> levels = node.getParametricLocation().getContinuousMarking();
        vector<double> drift = node.getParametricLocation().getDrift();

        // Order I: guard arcs for immediate transitions
        // next immediate transitions with their minimumTime
        vector<pair<double, shared_ptr<ImmediateTransition>>> nextImmediateGuards;
        for (auto transitionItem : hybridPetrinet->getImmediateTransitions()) {
            shared_ptr<ImmediateTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                shared_ptr<GuardArc> arc = arcItem.second;
                string placeId = arc->place->id;
                long placePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeId) -
                                continuousPlaceIDs.begin();
                double minimumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                     location.getGeneralIntervalBoundLeft(), levels[placePos]);
                double maximumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                     location.getGeneralIntervalBoundRight(), levels[placePos]);
                // todo: ist es möglich, dass das arc weight zwischen minimum und maximum level liegt?
                // drift is negative and level is over arc weight
                if (drift[placePos] < 0 && arc->weight < minimumLevel) {
                    double remainingMinimumTime = (arc->weight - minimumLevel) / drift[placePos];
                    double remainingMaximumTime = (arc->weight - maximumLevel) / drift[placePos];
                    if (remainingMaximumTime < firstMaximumTime) {
                        firstMaximumTime = remainingMaximumTime;
                        nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                    }
                    if (remainingMinimumTime <= firstMaximumTime) {
                        nextImmediateGuards.push_back(make_pair(remainingMinimumTime, transition));
                    }
                    // drift is positive and level is under arc weight
                } else if (drift[placePos] > 0 && arc->weight > maximumLevel) {
                    double remainingMinimumTime = (arc->weight - maximumLevel) / drift[placePos];
                    double remainingMaximumTime = (arc->weight - minimumLevel) / drift[placePos];
                    if (remainingMaximumTime < firstMaximumTime) {
                        firstMaximumTime = remainingMaximumTime;
                        nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                    }
                    if (remainingMinimumTime <= firstMaximumTime) {
                        nextImmediateGuards.push_back(make_pair(remainingMinimumTime, transition));
                    }
                }
            }
        }

        // Order II: firing of deterministic transition
        vector<pair<double, shared_ptr<DeterministicTransition>>> nextDeterministicTransitions;
        // get enabled deterministic transitions (we ignore priority)
        for (int pos = 0; pos < deterministicTransitionIDs.size(); ++pos) {
            shared_ptr<DeterministicTransition> transition = hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[pos]];
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
                // get time when transitions are enabled
                double elapsedMinimumTime = getBoundedTime(location.getGeneralTransitionsFired(),
                                                           location.getGeneralIntervalBoundLeft(),
                                                           location.getDeterministicClock()[pos]);
                double remainingMaximumTime = transition->getDiscTime() - elapsedMinimumTime;
                if (remainingMaximumTime < firstMaximumTime) {
                    firstMaximumTime = remainingMaximumTime;
                    nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                    nextDeterministicTransitions = getNewNextDeterministicTransitions(firstMaximumTime,
                                                                                      nextDeterministicTransitions);
                }
                double elapsedMaximumTime = getBoundedTime(location.getGeneralTransitionsFired(),
                                                           location.getGeneralIntervalBoundRight(),
                                                           location.getDeterministicClock()[pos]);
                double remainingMinimumTime = transition->getDiscTime() - elapsedMaximumTime;
                if (remainingMinimumTime <= firstMaximumTime)
                    nextDeterministicTransitions.push_back(make_pair(remainingMinimumTime, transition));
            }
        }

        // Order III: ContinuousPlace reaches boundary or guard arc for continuous transition
        vector<pair<double, shared_ptr<ContinuousPlace>>> nextPlaces;
        for (int pos = 0; pos < continuousPlaceIDs.size(); ++pos) {
            shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[continuousPlaceIDs[pos]];
            double minimumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                 location.getGeneralIntervalBoundLeft(), levels[pos]);
            double maximumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                 location.getGeneralIntervalBoundRight(), levels[pos]);
            double remainingMaximumTime;
            double remainingMinimumTime;
            if (drift[pos] < 0) { // negative drift
                remainingMaximumTime = maximumLevel / drift[pos];
                if (remainingMaximumTime < firstMaximumTime) {
                    firstMaximumTime = remainingMaximumTime;
                    nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                    nextDeterministicTransitions = getNewNextDeterministicTransitions(firstMaximumTime,
                                                                                      nextDeterministicTransitions);
                    nextPlaces = getNewNextPlaces(firstMaximumTime, nextPlaces);
                }
                remainingMinimumTime = minimumLevel / drift[pos];
                if (remainingMinimumTime <= firstMaximumTime)
                    nextPlaces.push_back(make_pair(remainingMinimumTime, place));
            } else if (drift[pos] > 0 && !place->getInfiniteCapacity()) { // positive drift
                if (place->getInfiniteCapacity())
                    continue;
                remainingMaximumTime = (place->getCapacity() - minimumLevel) / drift[pos];
                if (remainingMaximumTime < firstMaximumTime) {
                    firstMaximumTime = remainingMaximumTime;
                    nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                    nextDeterministicTransitions = getNewNextDeterministicTransitions(firstMaximumTime,
                                                                                      nextDeterministicTransitions);
                    nextPlaces = getNewNextPlaces(firstMaximumTime, nextPlaces);
                }
                remainingMinimumTime = (place->getCapacity() - maximumLevel) / drift[pos];
                if (remainingMinimumTime <= firstMaximumTime)
                    nextPlaces.push_back(make_pair(remainingMinimumTime, place));
            }
        }
        vector<pair<double, shared_ptr<ContinuousTransition>>> nextContinuousGuards;
        for (auto transitionItem : hybridPetrinet->getContinuousTransitions()) {
            shared_ptr<ContinuousTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                shared_ptr<GuardArc> arc = arcItem.second;
                string placeId = arc->place->id;
                // arc is connected to discrete place (no event will happen)
                if (find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeId) == continuousPlaceIDs.end())
                    continue;
                long placePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeId) -
                                continuousPlaceIDs.begin();
                shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[placeId];
                double minimumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                     location.getGeneralIntervalBoundLeft(), levels[placePos]);
                double maximumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                     location.getGeneralIntervalBoundRight(), levels[placePos]);
                // drift is negative and level is over arc weight
                if (drift[placePos] < 0 && arc->weight < minimumLevel) {
                    double remainingMaximumTime = (arc->weight - maximumLevel) / drift[placePos];
                    if (remainingMaximumTime < firstMaximumTime) {
                        firstMaximumTime = remainingMaximumTime;
                        nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                        nextDeterministicTransitions = getNewNextDeterministicTransitions(firstMaximumTime,
                                                                                          nextDeterministicTransitions);
                        nextPlaces = getNewNextPlaces(firstMaximumTime, nextPlaces);
                        nextContinuousGuards = getNewNextContinuousGuards(firstMaximumTime, nextContinuousGuards);
                    }
                    double remainingMinimumTime = (arc->weight - minimumLevel) / drift[placePos];
                    if (remainingMinimumTime <= firstMaximumTime)
                        nextContinuousGuards.push_back(make_pair(remainingMinimumTime, transition));
                    // drift is positive and level is under arc weight
                } else if (drift[placePos] > 0 && arc->weight > maximumLevel) {
                    double remainingMaximumTime = (arc->weight - minimumLevel) / drift[placePos];
                    if (remainingMaximumTime < firstMaximumTime) {
                        firstMaximumTime = remainingMaximumTime;
                        nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                        nextDeterministicTransitions = getNewNextDeterministicTransitions(firstMaximumTime,
                                                                                          nextDeterministicTransitions);
                        nextPlaces = getNewNextPlaces(firstMaximumTime, nextPlaces);
                        nextContinuousGuards = getNewNextContinuousGuards(firstMaximumTime, nextContinuousGuards);
                    }
                    double remainingMinimumTime = (arc->weight - maximumLevel) / drift[placePos];
                    if (remainingMinimumTime <= firstMaximumTime)
                        nextContinuousGuards.push_back(make_pair(remainingMinimumTime, transition));
                }
            }
        }

        // Order IV: guard arc for deterministic or general transition
        vector<pair<double, shared_ptr<DeterministicTransition>>> nextDeterministicGuards;
        for (auto transitionItem : hybridPetrinet->getDeterministicTransitions()) {
            shared_ptr<DeterministicTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                shared_ptr<GuardArc> arc = arcItem.second;
                string placeId = arc->place->id;
                long placePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeId) -
                                continuousPlaceIDs.begin();
                shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[placeId];
                double minimumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                     location.getGeneralIntervalBoundLeft(), levels[placePos]);
                double maximumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                     location.getGeneralIntervalBoundRight(), levels[placePos]);
                // drift is negative and level is over arc weight
                if (drift[placePos] < 0 && arc->weight < minimumLevel) {
                    double remainingMaximumTime = (arc->weight - maximumLevel) / drift[placePos];
                    if (remainingMaximumTime < firstMaximumTime) {
                        firstMaximumTime = remainingMaximumTime;
                        nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                        nextDeterministicTransitions = getNewNextDeterministicTransitions(firstMaximumTime,
                                                                                          nextDeterministicTransitions);
                        nextPlaces = getNewNextPlaces(firstMaximumTime, nextPlaces);
                        nextContinuousGuards = getNewNextContinuousGuards(firstMaximumTime, nextContinuousGuards);
                        nextDeterministicGuards = getNewNextDeterministicGuards(firstMaximumTime,
                                                                                nextDeterministicGuards);
                    }
                    double remainingMinimumTime = (arc->weight - minimumLevel) / drift[placePos];
                    if (remainingMinimumTime <= firstMaximumTime)
                        nextDeterministicGuards.push_back(make_pair(remainingMinimumTime, transition));
                    // drift is positive and level is under arc weight
                } else if (drift[placePos] > 0 && arc->weight > maximumLevel) {
                    double remainingMaximumTime = (arc->weight - minimumLevel) / drift[placePos];
                    if (remainingMaximumTime < firstMaximumTime) {
                        firstMaximumTime = remainingMaximumTime;
                        nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                        nextDeterministicTransitions = getNewNextDeterministicTransitions(firstMaximumTime,
                                                                                          nextDeterministicTransitions);
                        nextPlaces = getNewNextPlaces(firstMaximumTime, nextPlaces);
                        nextContinuousGuards = getNewNextContinuousGuards(firstMaximumTime, nextContinuousGuards);
                        nextDeterministicGuards = getNewNextDeterministicGuards(firstMaximumTime,
                                                                                nextDeterministicGuards);
                    }
                    double remainingMinimumTime = (arc->weight - maximumLevel) / drift[placePos];
                    if (remainingMinimumTime <= firstMaximumTime)
                        nextDeterministicGuards.push_back(make_pair(remainingMinimumTime, transition));
                }
            }
        }
        vector<pair<double, shared_ptr<GeneralTransition>>> nextGeneralGuards;
        for (auto transitionItem : hybridPetrinet->getGeneralTransitions()) {
            shared_ptr<GeneralTransition> transition = transitionItem.second;
            for (auto arcItem : transition->getGuardInputArcs()) {
                shared_ptr<GuardArc> arc = arcItem.second;
                string placeId = arc->place->id;
                long placePos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeId) -
                                continuousPlaceIDs.begin();
                shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[placeId];
                double minimumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                     location.getGeneralIntervalBoundLeft(), levels[placePos]);
                double maximumLevel = getBoundedTime(location.getGeneralTransitionsFired(),
                                                     location.getGeneralIntervalBoundRight(), levels[placePos]);
                // drift is negative and level is over arc weight
                if (drift[placePos] < 0 && arc->weight < minimumLevel) {
                    double remainingMaximumTime = (arc->weight - maximumLevel) / drift[placePos];
                    if (remainingMaximumTime < firstMaximumTime) {
                        firstMaximumTime = remainingMaximumTime;
                        nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                        nextDeterministicTransitions = getNewNextDeterministicTransitions(firstMaximumTime,
                                                                                          nextDeterministicTransitions);
                        nextPlaces = getNewNextPlaces(firstMaximumTime, nextPlaces);
                        nextContinuousGuards = getNewNextContinuousGuards(firstMaximumTime, nextContinuousGuards);
                        nextDeterministicGuards = getNewNextDeterministicGuards(firstMaximumTime,
                                                                                nextDeterministicGuards);
                        nextGeneralGuards = getNewNextGeneralGuards(firstMaximumTime, nextGeneralGuards);
                    }
                    double remainingMinimumTime = (arc->weight - minimumLevel) / drift[placePos];
                    if (remainingMinimumTime <= firstMaximumTime)
                        nextGeneralGuards.push_back(make_pair(remainingMinimumTime, transition));
                    // drift is positive and level is under arc weight
                } else if (drift[placePos] > 0 && arc->weight > maximumLevel) {
                    double remainingMaximumTime = (arc->weight - minimumLevel) / drift[placePos];
                    if (remainingMaximumTime < firstMaximumTime) {
                        firstMaximumTime = remainingMaximumTime;
                        nextImmediateGuards = getNewNextImmediateGuards(firstMaximumTime, nextImmediateGuards);
                        nextDeterministicTransitions = getNewNextDeterministicTransitions(firstMaximumTime,
                                                                                          nextDeterministicTransitions);
                        nextPlaces = getNewNextPlaces(firstMaximumTime, nextPlaces);
                        nextContinuousGuards = getNewNextContinuousGuards(firstMaximumTime, nextContinuousGuards);
                        nextDeterministicGuards = getNewNextDeterministicGuards(firstMaximumTime,
                                                                                nextDeterministicGuards);
                        nextGeneralGuards = getNewNextGeneralGuards(firstMaximumTime, nextGeneralGuards);
                    }
                    double remainingMinimumTime = (arc->weight - maximumLevel) / drift[placePos];
                    if (remainingMinimumTime <= firstMaximumTime)
                        nextGeneralGuards.push_back(make_pair(remainingMinimumTime, transition));
                }
            }
        }


        // If no immediate transition is enabled we have to consider all enabled general transitions
        // todo: get real time for events, not just minimumTime
        vector<string> enabledGeneralTransitions;
        auto generalTransitions = hybridPetrinet->getGeneralTransitions();
        for (auto i = generalTransitions.begin(); i != generalTransitions.end(); ++i) {
            shared_ptr<GeneralTransition> transition = i->second;
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet))
                addLocationForGeneralEvent(transition, maxTime, firstMaximumTime, node, hybridPetrinet);
        }

        // now we have the minimumTime and all events that happen at this time
        // todo: if minimumTime is not equal maximumtime here can happen more events
        if (!nextImmediateGuards.empty()) {
            for (auto &transition : nextImmediateGuards)
                addLocationForGuardEvent(transition.first, node, hybridPetrinet);
        } else if (!nextDeterministicTransitions.empty()) {
            double sumWeight = 0;
            for (pair<double, shared_ptr<DeterministicTransition>> &transition : nextDeterministicTransitions)
                sumWeight += transition.second->getWeight();
            for (pair<double, shared_ptr<DeterministicTransition>> &transition : nextDeterministicTransitions) {
                double probability = transition.second->getWeight() / sumWeight;
                addLocationForDeterministicEvent(transition.second, probability, transition.first, node, hybridPetrinet);
            }
        } else if (!nextPlaces.empty() || !nextContinuousGuards.empty()) {
            for (auto &place : nextPlaces)
                addLocationForBoundaryEvent(place.first, node, hybridPetrinet);
            for (auto &transition : nextContinuousGuards)
                addLocationForGuardEvent(transition.first, node, hybridPetrinet);
        } else if (!nextDeterministicGuards.empty() || !nextGeneralGuards.empty()) {
            for (auto &transition : nextDeterministicGuards)
                addLocationForGuardEvent(transition.first, node, hybridPetrinet);
            for (auto &transition : nextGeneralGuards)
                addLocationForGuardEvent(transition.first, node, hybridPetrinet);
        }

        for (ParametricLocationTree::Node &childNode : parametriclocationTree->getChildNodes(node)) {
            locationQueue.push_back(childNode);
        }
    }

    double ParseHybridPetrinet::getBoundedTime(vector<int> generalTransitionsFired,
                                               vector<vector<vector<double>>> generalBounds, vector<double> time) {
        // todo: denken: muss man bei negativen values evtl den anderen bound betrachten?
        assert(time.size() == generalTransitionsFired.size() + 1);
        vector<int> counter = vector<int>(generalTransitionIDs.size());
        fill(counter.begin(), counter.end(), 0);
        for (int i = 0; i < generalTransitionsFired.size(); ++i)
            counter[generalTransitionsFired[i]] += 1;
        while (time.size() > 1) {
            // get last value in timevector
            double value = time.back();
            time.pop_back();

            // get boundaries of last value
            int transitionPos = generalTransitionsFired[time.size() - 1];
            int firingTime = counter[transitionPos];
            vector<double> boundaries = generalBounds[transitionPos][firingTime];
            counter[transitionPos] -= 1;

            // add boundaries multiplicated with value to time vector
//            assert(boundaries.size() <= time.size()); todo
            for (int i = 0; i < time.size(); ++i)
                time[i] += value * boundaries[i];
        }
        return time[0];
    }

    vector<pair<double, shared_ptr<ImmediateTransition>>> ParseHybridPetrinet::getNewNextImmediateGuards(
            double maximumTime, vector<pair<double, shared_ptr<ImmediateTransition>>> nextImmediateGuards) {
        vector<pair<double, shared_ptr<ImmediateTransition>>> newNextImmediateGuards;
        for (pair<double, shared_ptr<ImmediateTransition>> guard : nextImmediateGuards)
            if (guard.first <= maximumTime)
                newNextImmediateGuards.push_back(guard);
        return newNextImmediateGuards;
    }

    vector<pair<double, shared_ptr<DeterministicTransition>>> ParseHybridPetrinet::getNewNextDeterministicTransitions(
            double maximumTime,
            vector<pair<double, shared_ptr<DeterministicTransition>>> nextDeterministicTransitions) {
        vector<pair<double, shared_ptr<DeterministicTransition>>> newNextDeterministicTransitions;
        for (pair<double, shared_ptr<DeterministicTransition>> transition : nextDeterministicTransitions)
            if (transition.first <= maximumTime)
                newNextDeterministicTransitions.push_back(transition);
        return newNextDeterministicTransitions;
    }

    vector<pair<double, shared_ptr<ContinuousPlace>>> ParseHybridPetrinet::getNewNextPlaces(double maximumTime,
                                                                                            vector<pair<double, shared_ptr<ContinuousPlace>>> nextPlaces) {
        vector<pair<double, shared_ptr<ContinuousPlace>>> newNextPlaces;
        for (pair<double, shared_ptr<ContinuousPlace>> place : nextPlaces)
            if (place.first <= maximumTime)
                newNextPlaces.push_back(place);
        return newNextPlaces;
    }

    vector<pair<double, shared_ptr<ContinuousTransition>>> ParseHybridPetrinet::getNewNextContinuousGuards(
            double maximumTime, vector<pair<double, shared_ptr<ContinuousTransition>>> nextContinuousGuards) {
        vector<pair<double, shared_ptr<ContinuousTransition>>> newNextContinuousGuards;
        for (pair<double, shared_ptr<ContinuousTransition>> transition : nextContinuousGuards)
            if (transition.first <= maximumTime)
                newNextContinuousGuards.push_back(transition);
        return newNextContinuousGuards;
    }

    vector<pair<double, shared_ptr<DeterministicTransition>>> ParseHybridPetrinet::getNewNextDeterministicGuards(
            double maximumTime, vector<pair<double, shared_ptr<DeterministicTransition>>> nextDeterministicGuards) {
        vector<pair<double, shared_ptr<DeterministicTransition>>> newNextDeterministicGuards;
        for (pair<double, shared_ptr<DeterministicTransition>> guard : nextDeterministicGuards)
            if (guard.first <= maximumTime)
                newNextDeterministicGuards.push_back(guard);
        return newNextDeterministicGuards;
    }

    vector<pair<double, shared_ptr<GeneralTransition>>> ParseHybridPetrinet::getNewNextGeneralGuards(
            double maximumTime, vector<pair<double, shared_ptr<GeneralTransition>>> nextGeneralGuards) {
        vector<pair<double, shared_ptr<GeneralTransition>>> newNextGeneralGuards;
        for (pair<double, shared_ptr<GeneralTransition>> guard : nextGeneralGuards)
            if (guard.first <= maximumTime)
                newNextGeneralGuards.push_back(guard);
        return newNextGeneralGuards;
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

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    void ParseHybridPetrinet::addLocationForGuardEvent(double timeDelta, ParametricLocationTree::Node parentNode,
                                                       shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();

        // adjust boundaries
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i)
            if (transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                    hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet))
                generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1][0] += timeDelta;

        vector<vector<double>> parentContinuousMarking = parentLocation.getContinuousMarking();
        vector<double> parentDrift = parentLocation.getDrift();

        vector<int> discreteMarking = parentLocation.getDiscreteMarking();
        vector<vector<double>> continuousMarking;
        for (int pos = 0; pos < parentContinuousMarking.size(); ++pos) {
            vector<double> newMarking = {parentContinuousMarking[pos][0] + (parentDrift[pos] * timeDelta)};
            continuousMarking.push_back(newMarking);
        }
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet);
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();

        double eventTime = parentLocation.getSourceEvent().getTime() + timeDelta;
        Event event = Event(EventType::Guard, generalDependecies, eventTime);
        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i)
            deterministicClocks[i][0] += timeDelta;

        // get new general clocks
        vector<vector<double>> generalClocks = parentLocation.getGeneralClock();
        for (int i = 0; i < generalClocks.size(); ++i)
            generalClocks[i][0] += timeDelta;

        newLocation.setDeterministicClock(deterministicClocks);
        newLocation.setGeneralClock(generalClocks);
        newLocation.setGeneralTransitionsFired(parentNode.getParametricLocation().getGeneralTransitionsFired());
        newLocation.setConflictProbability(1);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    void ParseHybridPetrinet::addLocationForDeterministicEvent(shared_ptr<DeterministicTransition> transition,
                                                               double probability, double timeDelta,
                                                               ParametricLocationTree::Node parentNode,
                                                               shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();

        // adjust boundaries
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i)
            if (transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                    hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet))
                generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1][0] += timeDelta;

        // get new markings
        vector<int> discreteMarking = parentNode.getParametricLocation().getDiscreteMarking();
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
        vector<vector<double>> continuousMarking = parentLocation.getContinuousMarking();
        vector<double> parentDrift = parentLocation.getDrift();
        for (int pos = 0; pos < continuousMarking.size(); ++pos)
            continuousMarking[pos][0] += parentDrift[pos] * timeDelta;
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet);

        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();

        double eventTime = parentLocation.getSourceEvent().getTime() + timeDelta;
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();
        Event event = Event(EventType::Timed, generalDependecies, eventTime);
        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i) {
            if (deterministicTransitionIDs[i] == transition->id)
                deterministicClocks[i][0] = 0;
            else
                deterministicClocks[i][0] += timeDelta;
        }

        // get new general clocks
        vector<vector<double>> generalClocks = parentLocation.getGeneralClock();
        for (int i = 0; i < generalClocks.size(); ++i)
            generalClocks[i][0] += timeDelta;

        newLocation.setDeterministicClock(deterministicClocks);
        newLocation.setGeneralClock(generalClocks);
        newLocation.setGeneralTransitionsFired(parentNode.getParametricLocation().getGeneralTransitionsFired());
        newLocation.setConflictProbability(probability);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    void ParseHybridPetrinet::addLocationForBoundaryEvent(double timeDelta, ParametricLocationTree::Node parentNode,
                                                          shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();

        // adjust boundaries
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i)
            if (transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                    hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]], hybridPetrinet))
                generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1][0] += timeDelta;

        // get new markings
        vector<vector<double>> parentContinuousMarking = parentLocation.getContinuousMarking();
        vector<double> parentDrift = parentLocation.getDrift();
        vector<int> discreteMarking = parentLocation.getDiscreteMarking();
        vector<vector<double>> continuousMarking;
        for (int pos = 0; pos < parentContinuousMarking.size(); ++pos) {
            vector<double> newMarking = {parentContinuousMarking[pos][0] + (parentDrift[pos] * timeDelta)};
            continuousMarking.push_back(newMarking);
        }
        vector<double> drift = getDrift(discreteMarking, continuousMarking, hybridPetrinet);
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();

        double eventTime = parentLocation.getSourceEvent().getTime() + timeDelta;
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();
        Event event = Event(EventType::Continuous, generalDependecies, eventTime);
        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i)
            deterministicClocks[i][0] += timeDelta;

        // get new general clocks
        vector<vector<double>> generalClocks = parentLocation.getGeneralClock();
        for (int i = 0; i < generalClocks.size(); ++i)
            generalClocks[i][0] += timeDelta;

        newLocation.setDeterministicClock(deterministicClocks);
        newLocation.setGeneralClock(generalClocks);
        newLocation.setGeneralTransitionsFired(parentNode.getParametricLocation().getGeneralTransitionsFired());
        newLocation.setConflictProbability(1);

        parametriclocationTree->setChildNode(parentNode, newLocation);
    }

    void ParseHybridPetrinet::addLocationForGeneralEvent(shared_ptr<GeneralTransition> transition, double maxTime,
                                                         double timeDelta, ParametricLocationTree::Node parentNode,
                                                         shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();
        long transitionPos = find(generalTransitionIDs.begin(), generalTransitionIDs.end(), transition->id) -
                             generalTransitionIDs.begin();

        vector<vector<double>> generalClocks = parentLocation.getGeneralClock();
        vector<double> transitionClock = generalClocks[transitionPos];

        // adjust boundaries
        // (we have to do this first because we need the old markings)
        vector<vector<vector<double>>> generalIntervalBoundLeft = parentLocation.getGeneralIntervalBoundLeft();
        vector<vector<vector<double>>> generalIntervalBoundRight = parentLocation.getGeneralIntervalBoundRight();
        for (int i = 0; i < generalTransitionIDs.size(); ++i) {
            if (generalTransitionIDs[i] == transition->id) {
                vector<vector<double>> currentBoundLeft = generalIntervalBoundLeft[i];
                vector<vector<double>> currentBoundRight = generalIntervalBoundRight[i];
                currentBoundRight[currentBoundRight.size() - 1] = generalClocks[i];
                currentBoundRight[currentBoundRight.size() - 1][0] += timeDelta;
                vector<double> vectorLeft = vector<double>(currentBoundLeft[currentBoundLeft.size() - 1].size() + 1);
                vector<double> vectorRight = vector<double>(currentBoundRight[currentBoundRight.size() - 1].size() + 1);
                fill(vectorLeft.begin(), vectorLeft.end(), 0);
                fill(vectorRight.begin(), vectorRight.end(), 0);
                vectorRight[0] = maxTime;
                currentBoundLeft.push_back(vectorLeft);
                currentBoundRight.push_back(vectorRight);
                generalIntervalBoundLeft[i] = currentBoundLeft;
                generalIntervalBoundRight[i] = currentBoundRight;
            } else if (transitionIsEnabled(parentLocation.getDiscreteMarking(), parentLocation.getContinuousMarking(),
                                           hybridPetrinet->getGeneralTransitions()[generalTransitionIDs[i]],
                                           hybridPetrinet)) {
                generalIntervalBoundLeft[i][generalIntervalBoundLeft[i].size() - 1][0] += timeDelta;
            }
        }

        // get new discrete markings
        vector<int> discreteMarking = parentNode.getParametricLocation().getDiscreteMarking();
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
        vector<double> generalDependecies = parentNode.getParametricLocation().getSourceEvent().getGeneralDependencies();
        for (int pos = 0; pos < transitionClock.size(); ++pos) {
            if (pos == 0) {
                eventTime -= transitionClock[pos];
                continue;
            }
            generalDependecies[pos - 1] -= transitionClock[pos];
        }
        generalDependecies.push_back(1.0);

        Event event = Event(EventType::General, generalDependecies, eventTime);
        ParametricLocation newLocation = ParametricLocation(discreteMarking, continuousMarking, drift,
                                                            static_cast<int>(numGeneralTransitions), event,
                                                            generalIntervalBoundLeft, generalIntervalBoundRight);

        // get new deterministic clocks
        vector<vector<double>> deterministicClocks = parentLocation.getDeterministicClock();
        for (int i = 0; i < deterministicClocks.size(); ++i) {
            for (int j = 0; j < deterministicClocks.size(); ++j)
                deterministicClocks[i][j] -= transitionClock[j];
            deterministicClocks[i].push_back(1);
        }

        // get new general clocks
        for (int i = 0; i < generalClocks.size(); ++i) {
            if (i == transitionPos) {
                fill(generalClocks[transitionPos].begin(), generalClocks[transitionPos].end(), 0);
                generalClocks[transitionPos].push_back(0);
                continue;
            }
            for (int j = 0; j < generalClocks.size(); ++j)
                generalClocks[i][j] -= transitionClock[j];
            generalClocks[i].push_back(1);
        }

        vector<int> generalTransitionsFired = parentNode.getParametricLocation().getGeneralTransitionsFired();
        generalTransitionsFired.push_back(static_cast<int>(transitionPos));

        newLocation.setDeterministicClock(deterministicClocks);
        newLocation.setGeneralClock(generalClocks);
        newLocation.setGeneralTransitionsFired(generalTransitionsFired);
        newLocation.setConflictProbability(1);

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
        for (auto i = continuousTransitions.begin(); i != continuousTransitions.end(); ++i) {
            shared_ptr<ContinuousTransition> transition = i->second;
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
        map<string, double> transitionRate;
        map<string, double> outputDriftNeeded;
        map<string, double> inputDriftNeeded;
        for (auto transition : hybridPetrinet->getContinuousTransitions())
            transitionRate[transition.first] = transition.second->getRate();
        while (!placesToCheck.empty()) {
            shared_ptr<ContinuousPlace> place = placesToCheck[0];
            long pos =
                    find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), place->id) - continuousPlaceIDs.begin();
            // Check if we need to adapt the rate
            // Case 1: we have to adapt the output transition (place is empty and drift is negative)
            if (drift[pos] < 0 && continuousMarking[pos][0] == 0.0) {
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
                for (auto iter = transitionsByPrio.begin(); iter != transitionsByPrio.end(); ++iter) {
                    vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>> prioTransitions = iter->second;
                    double sumOutRate = 0.0;
                    double sumShare = 0;
                    for (tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>> transItem : prioTransitions) {
                        shared_ptr<ContinuousTransition> transition = get<0>(transItem);
                        shared_ptr<ContinuousArc> arc = get<1>(transItem);
                        sumOutRate += arc->weight * transitionRate[transition->id];
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
                            if (transitionRate[transition->id] != newRate) {
                                double rateDiff = transitionRate[transition->id] - newRate;
                                if (outputDriftNeeded.find(transition->id) == outputDriftNeeded.end())
                                    outputDriftNeeded[transition->id] = rateDiff;
                                else
                                    outputDriftNeeded[transition->id] += rateDiff;
                                transitionRate[transition->id] = newRate;
                                for (auto arcItem : transition->getContinuousOutputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(
                                                hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                        placeToCheck->id) - continuousPlaceIDs.begin();
                                        inputDrift[pos] -= rateDiff;
                                        drift[pos] -= rateDiff;
                                    }
                                }
                                for (auto arcItem : transition->getContinuousInputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(
                                                hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                        placeToCheck->id) - continuousPlaceIDs.begin();
                                        outputDrift[pos] -= rateDiff;
                                        drift[pos] += rateDiff;
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
            else if (!place->getInfiniteCapacity() && drift[pos] > 0 &&
                     continuousMarking[pos][0] == place->getCapacity()) {
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
                for (auto iter = transitionsByPrio.begin(); iter != transitionsByPrio.end(); ++iter) {
                    vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>> prioTransitions = iter->second;
                    double sumInRate = 0.0;
                    double sumShare = 0;
                    for (tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>> transItem : prioTransitions) {
                        shared_ptr<ContinuousTransition> transition = get<0>(transItem);
                        shared_ptr<ContinuousArc> arc = get<1>(transItem);
                        sumInRate += arc->weight * transitionRate[transition->id];
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
                            if (transitionRate[transition->id] != newRate) {
                                double rateDiff = transitionRate[transition->id] - newRate;
                                if (inputDriftNeeded.find(transition->id) == inputDriftNeeded.end())
                                    inputDriftNeeded[transition->id] = rateDiff;
                                else
                                    inputDriftNeeded[transition->id] += rateDiff;
                                transitionRate[transition->id] = newRate;
                                for (auto arcItem : transition->getContinuousOutputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(
                                                hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                        placeToCheck->id) - continuousPlaceIDs.begin();
                                        inputDrift[pos] -= rateDiff;
                                        drift[pos] -= rateDiff;
                                    }
                                }
                                for (auto arcItem : transition->getContinuousInputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(
                                                hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(),
                                                        placeToCheck->id) - continuousPlaceIDs.begin();
                                        outputDrift[pos] -= rateDiff;
                                        drift[pos] += rateDiff;
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