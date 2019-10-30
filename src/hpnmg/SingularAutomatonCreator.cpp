#include "SingularAutomatonCreator.h"

#include "datastructures/SingularAutomaton.h"
#include "ParseHybridPetrinet.h"

#include <algorithm>

namespace hpnmg {

    bool SingularAutomatonCreator::compareOrderedVectors(const vector<vector<vector<double>>> &lhs, const vector<vector<vector<double>>> &rhs) {
        int coincidingTransitionNumber = min(lhs.size(),rhs.size()); // the maximal general transition that fired in both states
        // for each coinciding transition (and the firing-independent values at index 0)
        for(int transitionNumber = 0; transitionNumber < coincidingTransitionNumber; transitionNumber++) {
            int coincidingFiringAmount = min(lhs[transitionNumber].size(),rhs[transitionNumber].size()); // number of firing of that general transition that already took place in both states
            // for each coinciding firing (of that transition)
            for(int firing = 0; firing < coincidingFiringAmount; firing++) {
                // if one state contains more variables
                if(lhs[transitionNumber][firing].size() != rhs[transitionNumber][firing].size()) {
                    return lhs[transitionNumber][firing].size() < rhs[transitionNumber][firing].size();
                }
                // for each coinciding variable (of that firing)
                for(int posOfVar = 0; posOfVar < lhs[transitionNumber][firing].size(); posOfVar++) {
                    if(lhs[transitionNumber][firing][posOfVar] != rhs[transitionNumber][firing][posOfVar]) {
                        return lhs[transitionNumber][firing][posOfVar] < rhs[transitionNumber][firing][posOfVar];
                    }
                }
            }
            const vector<vector<double>>& largerVector = lhs[transitionNumber].size() > rhs[transitionNumber].size() ? lhs[transitionNumber] : rhs[transitionNumber];
            // for each firing (of that transition) that took place in only one state
            for(int firing = coincidingFiringAmount; firing < largerVector.size(); firing++) {
                for(int j = 0; j < largerVector[firing].size(); j++) {
                    if(largerVector[firing][j] != 0) {
                        return lhs[transitionNumber].size() < rhs[transitionNumber].size();
                    }
                }
            }
        }
        const vector<vector<vector<double>>>& largerVector = lhs.size() > rhs.size() ? lhs : rhs;
        // for each transition that fired in only one state
        for(int transitionNumber = coincidingTransitionNumber; transitionNumber < largerVector.size(); transitionNumber++) {
            for(int firing = 0; firing < largerVector[transitionNumber].size(); firing++) {
                for(int k = 0; k < largerVector[transitionNumber][firing].size(); k++) {
                    if(largerVector[transitionNumber][firing][k] != 0) {
                        return lhs.size() < rhs.size();
                    }
                }
            }
        }
        return false; // vectors are equal
    }

    pair<shared_ptr<ParametricLocationTree>, shared_ptr<SingularAutomaton>>
    SingularAutomatonCreator::transformIntoSingularAutomaton(shared_ptr<HybridPetrinet> hybridPetriNet,
                                                             double maxTime) {

        // create parametric location tree
        shared_ptr<ParseHybridPetrinet> hybridPetriNetParser = make_shared<ParseHybridPetrinet>();
        shared_ptr<ParametricLocationTree> parametricLocationTree = hybridPetriNetParser->parseHybridPetrinet(
                move(hybridPetriNet), maxTime);

        // start at root
        ParametricLocationTree::Node rootNode = parametricLocationTree->getRootNode();
        ParametricLocation rootParametricLocation = rootNode.getParametricLocation();

        PetriNetState state;
        state.discreteMarking = rootParametricLocation.getDiscreteMarking();
        state.orderedContinuousMarking = sortByOrder(rootParametricLocation.getContinuousMarking(),
                rootParametricLocation.getGeneralTransitionsFired());
        state.orderedDeterministicClock = sortByOrder(rootParametricLocation.getDeterministicClock(),
                                                      rootParametricLocation.getGeneralTransitionsFired());

        // create initial location
        LOCATION_ID id = rootNode.getNodeID();
        vector<bool> activitiesDeterministic = rootParametricLocation.getDeterministicTransitionsEnabled();
        SingularAutomaton::singleton activitiesContinuous = rootParametricLocation.getDrift();
        vector<bool> activitiesGeneral = rootParametricLocation.getGeneralTransitionsEnabled();

        shared_ptr<SingularAutomaton::Location> initialLocation = make_shared<SingularAutomaton::Location>(id,
                                                                                                           activitiesDeterministic,
                                                                                                           activitiesContinuous,
                                                                                                           activitiesGeneral);

        // add initial location to a new empty map
        mapStateToLocation.clear();
        mapStateToLocation[state] = initialLocation;

        // get Init
        SingularAutomaton::singleton initialDeterministic;
        for (vector<double> clock : rootParametricLocation.getDeterministicClock()) {
            initialDeterministic.push_back(clock[0]);
        }
        SingularAutomaton::singleton initialContinuous;
        for (vector<double> marking : rootParametricLocation.getContinuousMarking()) {
            initialContinuous.push_back(marking[0]);
        }
        SingularAutomaton::singleton initialGeneral;
        for (vector<double> clock : rootParametricLocation.getGeneralClock()) {
            initialGeneral.push_back(clock[0]);
        }

        // create singular automaton with initial location and Init
        shared_ptr<SingularAutomaton> automaton = make_shared<SingularAutomaton>(initialLocation, initialDeterministic, initialContinuous, initialGeneral);

        // add locations to singular automaton for all children of root
        addLocationsForChildren(hybridPetriNetParser, parametricLocationTree, automaton, &rootNode, initialLocation, state);

        // merge duplicate locations
        mergeIdenticalLocations(automaton);

        return pair<shared_ptr<ParametricLocationTree>, shared_ptr<SingularAutomaton>>(parametricLocationTree,automaton);
    }

    void SingularAutomatonCreator::addLocationsForChildren(shared_ptr<ParseHybridPetrinet> hybridPetriNetParser,
                                                           shared_ptr<ParametricLocationTree> parametricLocationTree,
                                                           shared_ptr<SingularAutomaton> singularAutomaton,
                                                           ParametricLocationTree::Node *parentNode,
                                                           shared_ptr<SingularAutomaton::Location> locationForParent,
                                                           PetriNetState stateOfParent) {

        // every time a leaf is reached, print an information
        if (parametricLocationTree->getChildNodes(*parentNode).empty()) {
            cout << "Location with no outgoing transition added to singular automaton. "
                 << "You might increase the time horizon. (This does not necessarily help.)" << endl;
        }

        // variable to check whether non-leaf parametric locations are considered, in which only non-stochastic events occur
        bool allEventsAreGeneral = true;

        // for each child node
        for (ParametricLocationTree::Node &childNode : parametricLocationTree->getChildNodes(*parentNode)) {
            ParametricLocation parentParametricLocation = parentNode->getParametricLocation();
            ParametricLocation childParametricLocation = childNode.getParametricLocation();

            // remember, if a non-general event occurred in this parametric location
            if (childParametricLocation.getSourceEvent().getEventType() != General) {
                allEventsAreGeneral = false;
            }

            // determine represented state (m, x, c, g)
            PetriNetState state;
            state.discreteMarking = childParametricLocation.getDiscreteMarking();
            state.orderedContinuousMarking = sortByOrder(childParametricLocation.getContinuousMarking(),
                                                         childParametricLocation.getGeneralTransitionsFired());
            state.orderedDeterministicClock = sortByOrder(childParametricLocation.getDeterministicClock(),
                                                          childParametricLocation.getGeneralTransitionsFired());

            // search for location in singular automaton with same state (m, x, c, g)
            auto iteratorLocation = mapStateToLocation.find(state);

            if (iteratorLocation == mapStateToLocation.end()) { // if no location found
                // determine activities in the new location for this child
                LOCATION_ID id = childNode.getNodeID();
                vector<bool> activitiesDeterministic = childParametricLocation.getDeterministicTransitionsEnabled();
                SingularAutomaton::singleton activitiesContinuous = childParametricLocation.getDrift();
                vector<bool> activitiesGeneral = childParametricLocation.getGeneralTransitionsEnabled();

                // create new location for this child
                shared_ptr<SingularAutomaton::Location> locationForThisChild = make_shared<SingularAutomaton::Location>(
                        id, activitiesDeterministic, activitiesContinuous, activitiesGeneral);

                // add new location to map and automaton
                mapStateToLocation[state] = locationForThisChild;
                singularAutomaton->addLocation(locationForThisChild);

                // insert a transition between the two locations for parent and this child
                transformEventIntoTransition(hybridPetriNetParser, singularAutomaton, &parentParametricLocation, locationForParent, stateOfParent,
                                             &childParametricLocation, locationForThisChild);

                // also add locations for the children of the current node
                addLocationsForChildren(hybridPetriNetParser, parametricLocationTree, singularAutomaton, &childNode,
                                        locationForThisChild,state);
            } else { // if location found
                // insert a transition between the two locations
                transformEventIntoTransition(hybridPetriNetParser, singularAutomaton, &parentParametricLocation, locationForParent, stateOfParent,
                                             &childParametricLocation, iteratorLocation->second);
            }
        }

        // all occurring events in this parametric location are general
        if (allEventsAreGeneral) {
            cout << "The parametric location tree contains a (considered) parametric location where only general events occur. "
                 << "This might indicate, that the non-stochastic events first occur after the time horizon "
                 << "and were cut by the time horizon. You might rerun with an increased time horizon." << endl;
        }
    }

    void SingularAutomatonCreator::transformEventIntoTransition(
            shared_ptr<ParseHybridPetrinet> hybridPetriNetParser,
            shared_ptr<SingularAutomaton> singularAutomaton,
            ParametricLocation* parentParametricLocation,
            shared_ptr<SingularAutomaton::Location> locationForParent,
            PetriNetState stateOfParent,
            ParametricLocation* childParametricLocation,
            shared_ptr<SingularAutomaton::Location> locationForChild) {

        Event childSourceEvent = childParametricLocation->getSourceEvent();
        EventType childSourceEventType = childSourceEvent.getEventType();

        SingularAutomaton::Transition::TransitionType type = childSourceEventType;
        long variableIndex = -1;
        double valuePreCompare = -1; // the value to compare to
        invariantOperator invOperator = UNLIMITED; // indicates whether the relational operator is <= or >=

        switch (childSourceEventType) {
            case Immediate:
                break;
            case General: {
                // get caused model member
                shared_ptr<GeneralTransition> transitionMember = childSourceEvent.getGeneralTransitionMember();

                // determine index of model member
                variableIndex = hybridPetriNetParser->getIndexOfGeneralTransition(transitionMember);
            }
                break;
            case Timed: {
                // get caused model member
                shared_ptr<DeterministicTransition> transitionMember = childSourceEvent.getDeterministicTransitionMember();

                // determine index of model member
                variableIndex = hybridPetriNetParser->getIndexOfDeterministicTransition(transitionMember);

                // the value to compare with is the firing time of the deterministic transition that fired
                valuePreCompare = transitionMember->getDiscTime();

                // invariant operator for deterministic clocks is <=
                invOperator = LOWER_EQUAL;
            }
                break;
            case Continuous: {
                // get caused model member
                shared_ptr<GuardArc> arcMember = childSourceEvent.getArcMember();
                shared_ptr<ContinuousPlace> placeMember = childSourceEvent.getPlaceMember();

                // distinguish, whether a guard arc or a continuous place is involved
                if (arcMember != nullptr) {
                    // determine index of model member
                    variableIndex = hybridPetriNetParser->getIndexOfContinuousPlace(
                            static_pointer_cast<ContinuousPlace>(arcMember->place));

                    // the value to compare with is the arcs weight that was reached
                    valuePreCompare = arcMember->weight;

                    // if the guard arc is an inhibitor arc
                    if (arcMember->getIsInhibitor()) {
                        // if the arc condition gets fulfilled
                        if (parentParametricLocation->getContinuousMarking()[variableIndex][0] >= valuePreCompare) {
                            invOperator = GREATER_EQUAL;
                        }
                        // if the arc condition stops being fulfilled
                        else {
                            invOperator = LOWER_EQUAL;
                        }
                    }
                    // if the guard arc is a test arc
                    else {
                        // if the arc condition gets fulfilled
                        if (parentParametricLocation->getContinuousMarking()[variableIndex][0] < valuePreCompare) {
                            invOperator = LOWER_EQUAL;
                        }
                            // if the arc condition stops being fulfilled
                        else {
                            invOperator = GREATER_EQUAL;
                        }
                    }
                } else if (placeMember != nullptr) {
                    // determine index of model member
                    variableIndex = hybridPetriNetParser->getIndexOfContinuousPlace(placeMember);

                    // the current continuous marking of this place is the boundary, that was reached
                    // and thus the value to compare with
                    valuePreCompare = childParametricLocation->getContinuousMarking()[variableIndex][0];

                    // if the lower boundary was reached, than the invariant operator is >=, else it is <=
                    invOperator = (valuePreCompare == 0 ? GREATER_EQUAL : LOWER_EQUAL);
                }
            }
                break;
            default:
                break; // do nothing
        }

        // insert transition in automaton
        singularAutomaton->insertTransition(move(locationForParent), type, variableIndex, valuePreCompare, invOperator,
                                            locationForChild);
        if (type == Immediate) {
            // locationForParent was removed by the previous insertTransition
            // and its state is represented by locationForChild
            mapStateToLocation[stateOfParent] = locationForChild;
        }
    }

    void SingularAutomatonCreator::mergeIdenticalLocations(shared_ptr<SingularAutomaton> singularAutomaton) {
        for (int i = 0; i < singularAutomaton->getLocations().size()-1; i++) {
            shared_ptr<SingularAutomaton::Location> firstLocation = singularAutomaton->getLocations()[i];
            for (int j = i+1; j < singularAutomaton->getLocations().size(); j++) {
                shared_ptr<SingularAutomaton::Location> secondLocation = singularAutomaton->getLocations()[j];
                if (firstLocation->isActInvOutgoingTransitionsIdentic(secondLocation)) {
                    // remove the outgoing transitions from secondLocation
                    for (const shared_ptr<SingularAutomaton::Transition>& transition : secondLocation->getOutgoingTransitions()) {
                        transition->getSuccessorLocation()->removeIncomingTransition(transition);
                        secondLocation->removeOutgoingTransition(transition);
                    }
                    // change the incoming transitions from secondLocation to be incoming transitions from firstLocation
                    for (const shared_ptr<SingularAutomaton::Transition>& transition : secondLocation->getIncomingTransitions()) {
                        transition->setSuccessorLocation(firstLocation);
                        firstLocation->addIncomingTransition(transition);
                        secondLocation->removeIncomingTransition(transition);
                    }
                    // remove the secondLocation
                    singularAutomaton->removeLocation(secondLocation);

                    // restart the search for identical locations
                    mergeIdenticalLocations(singularAutomaton);
                    return;
                }
            }
        }
    }

    vector<vector<vector<double>>> SingularAutomatonCreator::sortByOrder(const vector<vector<double>>& values,
                                                                         const vector<int>& order) {
        // get amount of different general transitions that fired and create vector accordingly
        unsigned long amount = order.empty() ? 0 : *max_element(order.begin(), order.end())+1;
        vector<vector<vector<double>>> orderedValues(amount+1);

        vector<double> valuesAtThisTime(values.size());
        for(int pointInTime = 0; pointInTime < values[0].size(); pointInTime++) {
            for(int posOfVar = 0; posOfVar < values.size(); posOfVar++) {
                valuesAtThisTime[posOfVar] = values[posOfVar][pointInTime];
            }
            if(pointInTime == 0) {
                orderedValues[0] = {valuesAtThisTime};
            }
            else {
                orderedValues[order[pointInTime-1]+1].push_back(valuesAtThisTime);
            }
        }
        return orderedValues;
    }
}