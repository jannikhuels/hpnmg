#include "ParseHybridPetrinet.h"

using namespace std;
namespace hpnmg {
    ParseHybridPetrinet::ParseHybridPetrinet() {}

    ParseHybridPetrinet::~ParseHybridPetrinet() {}

    shared_ptr<ParametricLocationTree> ParseHybridPetrinet::parseHybridPetrinet(shared_ptr<HybridPetrinet> hybridPetrinet, int maxTime) {
        // TODO: all floats to double?
        // Add place IDs from map to vector, so the places have an order
        map<string, shared_ptr<DiscretePlace>> discretePlaces = hybridPetrinet->getDiscretePlaces();
        for (auto i = discretePlaces.begin(); i != discretePlaces.end(); ++i) {
            discretePlaceIDs.push_back(i->first);
        }
        map<string, shared_ptr<ContinuousPlace>> continuousPlaces = hybridPetrinet->getContinuousPlaces();
        for (auto i = continuousPlaces.begin(); i != continuousPlaces.end(); ++i) {
            continuousPlaceIDs.push_back(i->first);
        }
        map<string, shared_ptr<DeterministicTransition>> deterministicTransitions = hybridPetrinet->getDeterministicTransitions();
        for (auto i = deterministicTransitions.begin(); i != deterministicTransitions.end(); ++i) {
            deterministicTransitionIDs.push_back(i->first);
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

        return rootLocation;
    }

    void ParseHybridPetrinet::processNode(ParametricLocationTree::Node node, shared_ptr<HybridPetrinet> hybridPetrinet, int maxTime) {
        vector<int> discreteMarking = node.getParametricLocation().getDiscreteMarking();
        vector<vector<double>> continuousMarking = node.getParametricLocation().getContinuousMarking();

        // Immediate Transition have highest priority, so we consider them first
        vector<shared_ptr<ImmediateTransition>> enabledImmediateTransition;
        unsigned long highestPriority = 0;
        auto immediateTransitions = hybridPetrinet->getImmediateTransitions();
        for(auto i = immediateTransitions.begin(); i!=immediateTransitions.end(); ++i) {
            shared_ptr<ImmediateTransition> transition = i->second;
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
                if (transition->getPriority() > highestPriority) {
                    enabledImmediateTransition.clear(); // priority is higher so we don't consider transitions with lower priority
                    enabledImmediateTransition.push_back(transition);
                } else if (transition->getPriority() == highestPriority) {
                    enabledImmediateTransition.push_back(transition);
                }
            }
        }
        if (!enabledImmediateTransition.empty()) {
            for (shared_ptr<ImmediateTransition> &transition : enabledImmediateTransition)
                addFireEvent(transition, node, hybridPetrinet, maxTime);
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
        // get enabled transitions
        for(int pos=0; pos<deterministicTransitionIDs.size(); ++pos) {
            shared_ptr<DeterministicTransition> transition = hybridPetrinet->getDeterministicTransitions()[deterministicTransitionIDs[pos]];
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
                // get time when transitions are enabled
                double elapsedTime =  node.getParametricLocation().getDeterministicClock()[pos];
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
            } else if (remainingTime == minimumTime) {
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
        // continuous arcs are enabled when place has higher or equal marking than arcs weight
        // place is continuous todo: sind die continuous arcs überhaupt relevant?
//        for (pair<string, shared_ptr<ContinuousArc>> arc_entry : transition->getContinuousInputArcs()) {
//            shared_ptr<ContinuousArc> arc = arc_entry.second;
//            shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[arc->place->id];
//            long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), place->id) - continuousPlaceIDs.begin();
//            double level = continousMarking[pos][0];
//            if (arc->weight > level)
//                return false;
//        }
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

    void ParseHybridPetrinet::addFireEvent(shared_ptr<ImmediateTransition> transition, ParametricLocationTree::Node node,
                                           shared_ptr<HybridPetrinet> hybridPetrinet, int maxTime) {
        ParametricLocation parentLocation = node.getParametricLocation();
        // get new markings
        vector<int> markings = node.getParametricLocation().getDiscreteMarking();
        for(auto arcItem : transition->getDiscreteInputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos = find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            markings[pos] -= arc->weight;
        }
        for(auto arcItem : transition->getDiscreteInputArcs()) {
            shared_ptr<DiscreteArc> arc = arcItem.second;
            long pos = find(discretePlaceIDs.begin(), discretePlaceIDs.end(), arc->place->id) - discretePlaceIDs.begin();
            markings[pos] += arc->weight;
        }

        // get new drift
        vector<vector<double>> continuousMarking = parentLocation.getContinuousMarking();
        vector<double> drift = getDrift(markings, continuousMarking, hybridPetrinet);

        // add new Location
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();
        // todo: was muss ich bei den anderen parametern der events angeben?
        ParametricLocation newLocation = ParametricLocation(markings, continuousMarking, drift,
            static_cast<int>(numGeneralTransitions), Event(EventType::Immediate,vector<double>{0, 0}, 0),
            parentLocation.getGeneralIntervalBoundLeft(), parentLocation.getGeneralIntervalBoundRight());
        parametriclocationTree->setChildNode(node, newLocation);
    }

    vector<double> ParseHybridPetrinet::getDrift(vector<int> discreteMarking, vector<vector<double>> continuousMarking,
                                                 shared_ptr<HybridPetrinet> hybridPetrinet) {
        vector<double> drift(continuousMarking.size());
        fill(drift.begin(), drift.end(), 0);
        // get rate for every place
        auto continuousTransitions = hybridPetrinet->getContinuousTransitions();
        for(auto i = continuousTransitions.begin(); i!=continuousTransitions.end(); ++i) {
            shared_ptr<ContinuousTransition> transition = i->second;
            if (transitionIsEnabled(discreteMarking, continuousMarking, transition, hybridPetrinet)) {
                for (auto arcItem : transition->getContinuousInputArcs()) {
                    shared_ptr<ContinuousArc> arc = arcItem.second;
                    long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), arc->place->id) - continuousPlaceIDs.begin();
                    drift[pos] -= transition->getRate();
                }
                for (auto arcItem : transition->getContinuousOutputArcs()) {
                    shared_ptr<ContinuousArc> arc = arcItem.second;
                    long pos = find(continuousPlaceIDs.begin(), continuousPlaceIDs.end(), arc->place->id) - continuousPlaceIDs.begin();
                    drift[pos] += transition->getRate();
                }
            }
        }
        // check if place is empty or full
        for(int pos=0; pos<drift.size(); ++pos) {
            shared_ptr<ContinuousPlace> place = hybridPetrinet->getContinuousPlaces()[continuousPlaceIDs[pos]];
            if (drift[pos] > 0) {
                if (!place->getInfiniteCapacity() && continuousMarking[pos][0] == place->getCapacity())
                    drift[pos] = 0;
            } else if (drift[pos] < 0) {
                if (continuousMarking[pos][0] == 0.0)
                    drift[pos] = 0;
            }
        }
        return drift;
    }
}