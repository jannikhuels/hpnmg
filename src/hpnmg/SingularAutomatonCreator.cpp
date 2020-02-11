#include "SingularAutomatonCreator.h"
#include <datastructures/SingularAutomaton.h>
#include "ParseHybridPetrinet.h"
#include <algorithm>

namespace hpnmg {


    pair<shared_ptr<ParametricLocationTree>, shared_ptr<SingularAutomaton>>
    SingularAutomatonCreator::transformIntoSingularAutomaton(shared_ptr<HybridPetrinet> hybridPetriNet,
                                                             double maxTime, int mode) {

        // create parametric location tree
        shared_ptr<ParseHybridPetrinet> hybridPetriNetParser = make_shared<ParseHybridPetrinet>();
        shared_ptr<ParametricLocationTree> parametricLocationTree = hybridPetriNetParser->parseHybridPetrinet(
                move(hybridPetriNet), maxTime, mode);

        // start at root
        vector<ParametricLocationTree::Node> currentNodes = {parametricLocationTree->getRootNode()};
        vector<ParametricLocationTree::Node> rootNodes;
        //


        //For initially enabled immediate transitions, skip location(s)
        bool immediate;
        int i = 0;
        while (i < currentNodes.size()){
            immediate = false;
            for (ParametricLocationTree::Node &childNode : parametricLocationTree->getChildNodes(currentNodes[i])) {
                if (childNode.getParametricLocation().getSourceEvent().getEventType() == Immediate) {
                    immediate = true;
                        currentNodes.push_back(childNode);
                }
            }
            if (!immediate) {
                rootNodes.push_back(currentNodes[i]);
            }
            i++;
        }

        //get total number of general transition firings
        int totalGeneral = parametricLocationTree->getDistributionsNormalized().size();

        bool first = true;
        SingularAutomaton::singleton initialDeterministic;
        SingularAutomaton::singleton initialContinuous;
        SingularAutomaton::singleton initialGeneral(totalGeneral, 0.0);
        vector<shared_ptr<SingularAutomaton::Location>> initialLocations;


        for (ParametricLocationTree::Node rootNode : rootNodes) {

            ParametricLocation rootParametricLocation = rootNode.getParametricLocation();

            // create initial location
            LOCATION_ID id = rootNode.getNodeID();
            vector<bool> activitiesDeterministic = rootParametricLocation.getDeterministicTransitionsEnabled();
            SingularAutomaton::singleton activitiesContinuous = rootParametricLocation.getDrift();

            // Adaption for general transition firing order
            vector<short int> activitiesGeneral(totalGeneral, false); // = rootParametricLocation.getGeneralTransitionsEnabled();
            vector<bool> enabledTransitions = rootParametricLocation.getGeneralTransitionsEnabled();
            for (int j = 0; j < enabledTransitions.size(); j++) {
                if (enabledTransitions[j]) {
                    int index = parametricLocationTree->getNormalizedIndexOfTransitionFiring(j, 0);
                    activitiesGeneral[index] = 1;
                    mapNormalizedIndexToGeneralTransitionFiring[index] = make_pair(j,0);
                }
            }

            shared_ptr<SingularAutomaton::Location> initialLocation = make_shared<SingularAutomaton::Location>(id, activitiesDeterministic,  activitiesContinuous, activitiesGeneral);


            if (first) {
                // get Init
                for (vector<double> clock : rootParametricLocation.getDeterministicClock()) {
                    initialDeterministic.push_back(clock[0]);
                }

                for (vector<double> marking : rootParametricLocation.getContinuousMarking()) {
                    initialContinuous.push_back(marking[0]);
                }

                int k = 0;
                for (vector<double> clock : rootParametricLocation.getGeneralClock()) {
                    initialGeneral[parametricLocationTree->getNormalizedIndexOfTransitionFiring(k, 0)] = clock[0];
                    k++;
                }
            }

            initialLocations.push_back(initialLocation);

            first = false;
        }

        // create singular automaton with initial location and Init
        shared_ptr<SingularAutomaton> automaton = make_shared<SingularAutomaton>(initialLocations, initialDeterministic, initialContinuous, initialGeneral);

        // add locations to singular automaton for all children of all root locations

        for (int j = 0; j < initialLocations.size(); j++) {
            automaton->addLocation(initialLocations[j]);
            addLocationsForChildren(hybridPetriNetParser, parametricLocationTree, automaton, &rootNodes[j], initialLocations[j]);
        }

        // merge duplicate locations
        mergeIdenticalLocations(automaton);

        return pair<shared_ptr<ParametricLocationTree>, shared_ptr<SingularAutomaton>>(parametricLocationTree,automaton);
    }



    void SingularAutomatonCreator::addLocationsForChildren(shared_ptr<ParseHybridPetrinet> hybridPetriNetParser,
                                                           shared_ptr<ParametricLocationTree> parametricLocationTree,
                                                           shared_ptr<SingularAutomaton> singularAutomaton,
                                                           ParametricLocationTree::Node *parentNode,
                                                           shared_ptr<SingularAutomaton::Location> locationForParent) {


        // variable to check whether non-leaf parametric locations are considered, in which only non-stochastic events occur
        bool allEventsAreGeneral = true;

        //get total number of general transition firings
        int totalGeneral = parametricLocationTree->getDistributionsNormalized().size();

        // for each child node
        for (ParametricLocationTree::Node &childNode : parametricLocationTree->getChildNodes(*parentNode)) {
            ParametricLocation parentParametricLocation = parentNode->getParametricLocation();
            ParametricLocation childParametricLocation = childNode.getParametricLocation();

            // remember, if a non-general event occurred in this parametric location
            if (childParametricLocation.getSourceEvent().getEventType() != General) {
                allEventsAreGeneral = false;
            }

              // determine activities in the new location for this child
            LOCATION_ID id = childNode.getNodeID();
            vector<bool> activitiesDeterministic = childParametricLocation.getDeterministicTransitionsEnabled();
            SingularAutomaton::singleton activitiesContinuous = childParametricLocation.getDrift();

            vector<short int> activitiesGeneral (totalGeneral, false); // = childParametricLocation.getGeneralTransitionsEnabled();
            vector<bool> enabledTransitions = childParametricLocation.getGeneralTransitionsEnabled();
            for (int i = 0; i < enabledTransitions.size();i++){
                if (enabledTransitions[i]) {
                    int firing = 0;
                    for (int transition : childParametricLocation.getGeneralTransitionsFired())
                        if (transition == i)
                            firing++;

                    int index = parametricLocationTree->getNormalizedIndexOfTransitionFiring(i, firing);
                    activitiesGeneral[index] = 1;
                    //mapNormalizedIndexToGeneralTransitionFiring.try_emplace(index, mypair);//make_pair(index, make_pair(i, firing)));
                    mapNormalizedIndexToGeneralTransitionFiring[index] = make_pair(i,firing);
                }
            }


                // create new location for this child
                shared_ptr<SingularAutomaton::Location> locationForThisChild = make_shared<SingularAutomaton::Location>(
                        id, activitiesDeterministic, activitiesContinuous, activitiesGeneral);

                // add new location to map and automaton
                //mapStateToLocation[state] = locationForThisChild;
                singularAutomaton->addLocation(locationForThisChild);

                // insert a transition between the two locations for parent and this child
                transformEventIntoTransition(hybridPetriNetParser, parametricLocationTree, singularAutomaton, &parentParametricLocation, locationForParent,
                                             &childParametricLocation, locationForThisChild);

                // also add locations for the children of the current node
                addLocationsForChildren(hybridPetriNetParser, parametricLocationTree, singularAutomaton, &childNode,
                                        locationForThisChild);

        }

        if (locationForParent->isToBeDeleted()){

            // skip predecessorLocation and go directly to successorLocations
            for (shared_ptr<SingularAutomaton::Transition> incomingTransition : locationForParent->getIncomingTransitions()) {
                shared_ptr<SingularAutomaton::Location> predecessorLocation = incomingTransition->getPredecessorLocation();

                for (shared_ptr<SingularAutomaton::Transition> outgoingTransition : locationForParent->getOutgoingTransitions()){
                    shared_ptr<SingularAutomaton::Location> successorLocation = outgoingTransition->getSuccessorLocation();

                    shared_ptr<SingularAutomaton::Transition> newTransition = make_shared<SingularAutomaton::Transition>(predecessorLocation, incomingTransition->getType(),
                    incomingTransition->getVariableIndex(), incomingTransition->getValueGuardCompare(), successorLocation);

                    predecessorLocation->addOutgoingTransition(newTransition);
                    successorLocation->addIncomingTransition(newTransition);

                }
                locationForParent->removeIncomingTransition(incomingTransition);
                predecessorLocation->removeOutgoingTransition(incomingTransition);
            }

            for (shared_ptr<SingularAutomaton::Transition> outgoingTransition : locationForParent->getOutgoingTransitions()){

                locationForParent->removeOutgoingTransition(outgoingTransition);
                outgoingTransition->getSuccessorLocation()->removeIncomingTransition(outgoingTransition);
            }

            // remove locationForParent from the automaton
            singularAutomaton->removeLocation(locationForParent);

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
            shared_ptr<ParametricLocationTree> parametricLocationTree,
            shared_ptr<SingularAutomaton> singularAutomaton,
            ParametricLocation* parentParametricLocation,
            shared_ptr<SingularAutomaton::Location> locationForParent,
            ParametricLocation* childParametricLocation,
            shared_ptr<SingularAutomaton::Location> locationForChild) {

        Event childSourceEvent = childParametricLocation->getSourceEvent();
        EventType childSourceEventType = childSourceEvent.getEventType();

        SingularAutomaton::Transition::TransitionType type = childSourceEventType;
        long variableIndex = -1;
        int variableIndexNormalized = -1;
        long guardIndex = -1;
        double valuePreCompare = -1; // the value to compare to
        invariantOperator invOperator = UNLIMITED; // indicates whether the relational operator is <= or >=

        switch (childSourceEventType) {
            case Immediate:
                break;
            case General: {
                // get caused model member
                shared_ptr<GeneralTransition> transitionMember = childSourceEvent.getGeneralTransitionMember();

                // determine index of model member
                int transitionID = hybridPetriNetParser->getIndexOfGeneralTransition(transitionMember);
                int firing = 0;
                for (int transition : parentParametricLocation->getGeneralTransitionsFired())
                    if (transition == transitionID)
                        firing++;

                variableIndex = parametricLocationTree->getNormalizedIndexOfTransitionFiring(transitionID, firing);
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


    shared_ptr<SingularAutomaton> SingularAutomatonCreator::addDistributions(shared_ptr<SingularAutomaton> automaton, vector<pair<string, map<string, float>>> distributionsNormalized){

        vector<shared_ptr<SingularAutomaton::Location>> initialLocations;
        shared_ptr<SingularAutomaton::Location> previousLocation;
        automaton->setDistributionsNormalized(distributionsNormalized);

        vector<bool> activitiesDeterministic(automaton->getInitialDeterministic().size(), false);
        vector<double> activitiesContinuous(automaton->getInitialContinuous().size(), 0.0);
        int id = 0;


        for ( shared_ptr<SingularAutomaton::Location> initialLocation : automaton->getInitialLocations()){

            bool first = true;
            int counter = 0;
            int index = 0;

            for (bool enabled : initialLocation->getActivitiesGeneral()){

                if (enabled){
                    vector<short int> activitiesGeneral(automaton->getInitialGeneral().size(), false);
                    activitiesGeneral[index] = 1;
                    shared_ptr<SingularAutomaton::Location> newLocation = make_shared<SingularAutomaton::Location>(id, activitiesDeterministic,  activitiesContinuous, activitiesGeneral);
                    newLocation->addToInvariantGeneral(index, 0.0, LOWER_EQUAL);
                    id--;

                    automaton->addLocation(newLocation);
                    if (first)
                        initialLocations.push_back(newLocation);
                    else {
                        shared_ptr<SingularAutomaton::Transition> transition = automaton->insertTransition(move(previousLocation), Root, index-1, -1, UNLIMITED, newLocation);
                        transition->addSamplingVariable((long)(index-1));
                    }

                    first = false;
                    counter++;
                    previousLocation = newLocation;
                }
                index++;

            }

            shared_ptr<SingularAutomaton::Transition> transition = automaton->insertTransition(move(previousLocation), Root, index-1, -1, UNLIMITED, initialLocation);
            transition->addSamplingVariable((long)(index-1));
        }

        automaton->overwriteInitialLocations(initialLocations);

        for (shared_ptr<SingularAutomaton::Location> location : automaton->getLocations()){

            if (location->getLocationId()>0) {

                vector<short int> actGen = location->getActivitiesGeneral();
                for (int i = 0; i < actGen.size(); i++) {

                    if (actGen[i] == 1) {
                        actGen[i] = -1;
                        location->addToInvariantGeneral(i, 0.0, GREATER_EQUAL);

                       for (shared_ptr<SingularAutomaton::Transition> transition : location->getIncomingTransitions()){

                           if (transition->getPredecessorLocation()->getActivitiesGeneral()[i] == 0){
                                transition->addSamplingVariable((long)(i));
                           }
                       }
                    }
                }

                location->overwriteActivitiesGeneral(actGen);
            }
        }


        return automaton;
    }

    map<int,pair<int,int>> SingularAutomatonCreator::getMapNormalizedIndexToGeneralTransitionFiring() {
        return mapNormalizedIndexToGeneralTransitionFiring;
    }


}

