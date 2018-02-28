#include "ParseHybridPetrinet.h"

using namespace std;
namespace hpnmg {
    ParseHybridPetrinet::ParseHybridPetrinet() {}

    ParseHybridPetrinet::~ParseHybridPetrinet() {}

    shared_ptr<ParametricLocationTree> ParseHybridPetrinet::parseHybridPetrinet(shared_ptr<HybridPetrinet> hybridPetrinet, int maxTime) {
        // TODO: all floats to double?
        // todo: propabilities when imm trans fire at same time with same prio (weight)

        // Add place IDs from map to vector, so the places have an order
        map<string, shared_ptr<DiscretePlace>> discretePlaces = hybridPetrinet->getDiscretePlaces();
        for (auto i = discretePlaces.begin(); i != discretePlaces.end(); ++i) {
            discretePlaceIDs.push_back(i->first);
        }
        map<string, shared_ptr<ContinuousPlace>> continuousPlaces = hybridPetrinet->getContinuousPlaces();
        for (auto i = continuousPlaces.begin(); i != continuousPlaces.end(); ++i) {
            continuousPlaceIDs.push_back(i->first);
        }
        // Also for deterministic Transitions
        map<string, shared_ptr<DeterministicTransition>> deterministicTransitions = hybridPetrinet->getDeterministicTransitions();
        for (auto i = deterministicTransitions.begin(); i != deterministicTransitions.end(); ++i) {
            deterministicTransitionIDs.push_back(i->first);
        }

        // Initialize aleatory variable counter for deterministic transitions and continuous places
        for (string id : deterministicTransitionIDs) {
            deterTransAleatoryVariables[id] = {};
        }
        for (string id : continuousPlaceIDs) {
            contPlaceAleatoryVariables[id] = {};
        }

        ParametricLocation rootLocation = generateRootParametricLocation(hybridPetrinet, maxTime);

        parametriclocationTree = make_shared<ParametricLocationTree>(rootLocation, maxTime);

        locationQueue.push_back(parametriclocationTree->getRootNode());

        while(!locationQueue.empty()) {
            processNode(locationQueue[0], hybridPetrinet, maxTime);
            locationQueue.erase(locationQueue.begin());
        }

        return parametriclocationTree;
    }

    ParametricLocation ParseHybridPetrinet::generateRootParametricLocation(shared_ptr<HybridPetrinet> hybridPetrinet, int maxTime) {
        // Get root discrete markings
        vector<int> rootDiscreteMarking;
        for (string &placeId: discretePlaceIDs)
            rootDiscreteMarking.push_back(static_cast<int>(hybridPetrinet->getDiscretePlaces()[placeId]->getMarking()));

        // Get root continuous marking
        vector<vector<double>> rootContinuousMarking; // todo: warum eine Matrix?
        for (string &placeId: continuousPlaceIDs) {
            rootContinuousMarking.push_back({hybridPetrinet->getContinuousPlaces()[placeId]->getLevel()});
        }

        vector<double> drift = getDrift(rootDiscreteMarking, rootContinuousMarking, hybridPetrinet);

        // Fill left and right bounds
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();
        vector<double> leftBoundaries(numGeneralTransitions);
        fill(leftBoundaries.begin(), leftBoundaries.end(), 0);
        vector<double> rightBoundaries(numGeneralTransitions);
        fill(rightBoundaries.begin(), rightBoundaries.end(), maxTime);

        ParametricLocation rootLocation = ParametricLocation(rootDiscreteMarking, rootContinuousMarking, drift,
            static_cast<int>(numGeneralTransitions), Event(EventType::Root,vector<double>{0, 0}, 0),
            leftBoundaries, rightBoundaries);

        vector<double> deterministicClock(deterministicTransitionIDs.size());
        fill(deterministicClock.begin(), deterministicClock.end(), 0);

        rootLocation.setDeterministicClock(deterministicClock);
        rootLocation.setConflictProbability(1);

        return rootLocation;
    }

    void ParseHybridPetrinet::processNode(ParametricLocationTree::Node node, shared_ptr<HybridPetrinet> hybridPetrinet, int maxTime) {
        vector<int> discreteMarking = node.getParametricLocation().getDiscreteMarking();
        vector<vector<double>> continuousMarking = node.getParametricLocation().getContinuousMarking();

        // Immediate Transition have highest priority, so we consider them first
        vector<shared_ptr<ImmediateTransition>> enabledImmediateTransition;
        unsigned long highestPriority = ULONG_MAX;
        auto immediateTransitions = hybridPetrinet->getImmediateTransitions();
        for(auto i = immediateTransitions.begin(); i!=immediateTransitions.end(); ++i) {
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

        // If no immediate transition is enabled we have to consider all enabled general transitions
        vector<string> enabledGeneralTransitions;
        auto generalTransitions = hybridPetrinet->getGeneralTransitions();
        for(auto i = generalTransitions.begin(); i!=generalTransitions.end(); ++i) {
            shared_ptr<Transition> transition = i->second;
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet))
                enabledGeneralTransitions.push_back(transition->id); // todo: add fire event for every enabled transition
        }

        // Now we can consider timed transitions and fluid places, we need all events that start first
        double minimumTime = -1;
        vector<shared_ptr<DeterministicTransition>> nextTransitions;
        // get enabled deterministic transitions (we ignore priority)
        for(int pos=0; pos<deterministicTransitionIDs.size(); ++pos) {
            shared_ptr<DeterministicTransition> transition = hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[pos]];
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
                // get time when transitions are enabled
                double elapsedTime = node.getParametricLocation().getDeterministicClock()[pos];
                double remainingTime = transition->getDiscTime() - elapsedTime;
                if (nextTransitions.empty()) {
                    nextTransitions.push_back(transition);
                    minimumTime = remainingTime;
                } else if (remainingTime < minimumTime) {
                    nextTransitions.clear();
                    nextTransitions.push_back(transition);
                    minimumTime = remainingTime;
                } else if (remainingTime == minimumTime) {
                    nextTransitions.push_back(transition);
                }
            }
        }
        vector<shared_ptr<ContinuousPlace>> nextPlaces;
        vector<double> drift = node.getParametricLocation().getDrift();
        for(int pos=0; pos<continuousPlaceIDs.size(); ++pos) {
            if (drift[pos] == 0)
                continue;
            shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[continuousPlaceIDs[pos]];
            double level = node.getParametricLocation().getContinuousMarking()[pos][0];
            double remainingTime;
            // todo: consider guard arcs get enabled
            if (drift[pos] < 0) {
                remainingTime = level / drift[pos];
            } else {
                if (place->getInfiniteCapacity())
                    continue;
                remainingTime = (place->getCapacity() - level) / drift[pos];
            }
            if (nextTransitions.empty() && nextPlaces.empty()) {
                minimumTime = remainingTime;
                nextPlaces.push_back(place);
            } else if (remainingTime < minimumTime) {
                nextTransitions.clear();
                nextPlaces.clear();
                nextPlaces.push_back(place);
            }
        }

        // now we have the minimumTime and all events that happen at this time

    }

    bool ParseHybridPetrinet::transitionIsEnabled(vector<int> discreteMarking, vector<vector<double>> continousMarking,
                                                  shared_ptr<Transition> transition, shared_ptr<HybridPetrinet> hybridPetrinet) {
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
                if ((marking > arc->weight && arc->getIsInhibitor()) || ((marking < arc->weight && !arc->getIsInhibitor())))
                    return false;
            } else { // place is continuous
                shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[arc->place->id];
                long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), place->id) - continuousPlaceIDs.begin();
                double level = continousMarking[pos][0];
                if ((level > arc->weight && arc->getIsInhibitor()) || ((level < arc->weight && !arc->getIsInhibitor())))
                    return false;
            }
        }
        return true;
    }

    void ParseHybridPetrinet::addLocationForImmediateEvent(shared_ptr<ImmediateTransition> transition,
            ParametricLocationTree::Node parentNode, float probability, shared_ptr<HybridPetrinet> hybridPetrinet) {
        ParametricLocation parentLocation = parentNode.getParametricLocation();
        // get new markings
        vector<int> markings = parentNode.getParametricLocation().getDiscreteMarking();
        for(auto arcItem : transition->getDiscreteInputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos = find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            markings[pos] -= arc->weight;
        }
        for(auto arcItem : transition->getDiscreteOutputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos = find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            markings[pos] += arc->weight;
        }

        // get new drift
        vector<vector<double>> continuousMarking = parentLocation.getContinuousMarking();
        vector<double> drift = getDrift(markings, continuousMarking, hybridPetrinet);

        // add new Location
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();
        double eventTime = parentNode.getParametricLocation().getSourceEvent().getTime();
        // todo: was muss ich bei den anderen parametern der events angeben?
        Event event = Event(EventType::Immediate,vector<double>{0, 0}, eventTime);
        ParametricLocation newLocation = ParametricLocation(markings, continuousMarking, drift,
            static_cast<int>(numGeneralTransitions), event, parentLocation.getGeneralIntervalBoundLeft(),
            parentLocation.getGeneralIntervalBoundRight());

        newLocation.setDeterministicClock(parentNode.getParametricLocation().getDeterministicClock());
        newLocation.setConflictProbability(probability);

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
        // get rate for every place
        auto continuousTransitions = hybridPetrinet->getContinuousTransitions();
        for(auto i = continuousTransitions.begin(); i!=continuousTransitions.end(); ++i) {
            shared_ptr<ContinuousTransition> transition = i->second;
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
                for (auto arcItem : transition->getContinuousInputArcs()) {
                    shared_ptr<ContinuousArc> arc = arcItem.second;
                    long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), arc->place->id) - continuousPlaceIDs.begin();
                    drift[pos] -= arc->weight * transition->getRate();
                    outputDrift[pos] += arc->weight * transition->getRate(); // input for transition is output for place
                }
                for (auto arcItem : transition->getContinuousOutputArcs()) {
                    shared_ptr<ContinuousArc> arc = arcItem.second;
                    long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), arc->place->id) - continuousPlaceIDs.begin();
                    drift[pos] += arc->weight * transition->getRate();
                    inputDrift[pos] += arc->weight * transition->getRate(); // output for transition is input for place
                }
            }
        }

        // Rate Adaption
        vector<shared_ptr<ContinuousPlace>> placesToCheck;
        for (auto placeItem :  hybridPetrinet->getContinuousPlaces())
            placesToCheck.push_back(placeItem.second);
        map <string, double> transitionRate;
        for (auto transition : hybridPetrinet->getContinuousTransitions())
            transitionRate[transition.first] = transition.second->getRate();
        while (!placesToCheck.empty()) {
            shared_ptr<ContinuousPlace> place = placesToCheck[0];
            long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), place->id) - continuousPlaceIDs.begin();
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
                    for (auto arcItem : transition->getContinuousOutputArcs())
                        if (arcItem.second->place->id == place->id) {
                            isInputTransition = true;
                            arc = arcItem.second;
                        }
                    if (isInputTransition) {
                        unsigned long priority = arc->getPriority();
                        if (transitionsByPrio.find(priority) != transitionsByPrio.end())
                            transitionsByPrio[priority] = {make_tuple(transition, arc)};
                        else
                            transitionsByPrio[priority].push_back(make_tuple(transition, arc));
                    }
                }
                // Adapt rate for some transitions
                for (auto iter = transitionsByPrio.rbegin(); iter != transitionsByPrio.rend(); ++iter) {
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
                                transitionRate[transition->id] = newRate;
                                for (auto arcItem : transition->getContinuousOutputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeToCheck->id) - continuousPlaceIDs.begin();
                                        inputDrift[pos] -= rateDiff;
                                        drift[pos] -= rateDiff;
                                    }
                                }
                                for (auto arcItem : transition->getContinuousInputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeToCheck->id) - continuousPlaceIDs.begin();
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
            else if (!place->getInfiniteCapacity() && drift[pos] > 0 && continuousMarking[pos][0] == place->getCapacity()) {
                double leftInputRate = outputDrift[pos];
                inputDrift[pos] = outputDrift[pos];
                drift[pos] = 0;
                map<unsigned long, vector<tuple<shared_ptr<ContinuousTransition>, shared_ptr<ContinuousArc>>>> transitionsByPrio;
                // Get In-Transitions sorted by prio
                for (auto transitionItem : hybridPetrinet->getContinuousTransitions()) {
                    shared_ptr<ContinuousTransition> transition = transitionItem.second;
                    shared_ptr<ContinuousArc> arc;
                    bool isOutputTransition = false;
                    for (auto arcItem : transition->getContinuousInputArcs())
                        if (arcItem.second->place->id == place->id) {
                            isOutputTransition = true;
                            arc = arcItem.second;
                        }
                    if (isOutputTransition) {
                        unsigned long priority = arc->getPriority();
                        if (transitionsByPrio.find(priority) != transitionsByPrio.end())
                            transitionsByPrio[priority] = {make_tuple(transition, arc)};
                        else
                            transitionsByPrio[priority].push_back(make_tuple(transition, arc));
                    }
                }
                // Adapt rate for some transitions
                for (auto iter = transitionsByPrio.rbegin(); iter != transitionsByPrio.rend(); ++iter) {
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
                                transitionRate[transition->id] = newRate;
                                for (auto arcItem : transition->getContinuousOutputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeToCheck->id) - continuousPlaceIDs.begin();
                                        inputDrift[pos] -= rateDiff;
                                        drift[pos] -= rateDiff;
                                    }
                                }
                                for (auto arcItem : transition->getContinuousInputArcs()) {
                                    shared_ptr<Place> placeToCheck = arcItem.second->place;
                                    if (place->id != placeToCheck->id) {
                                        placesToCheck.push_back(hybridPetrinet->getContinuousPlaces()[placeToCheck->id]);
                                        long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), placeToCheck->id) - continuousPlaceIDs.begin();
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