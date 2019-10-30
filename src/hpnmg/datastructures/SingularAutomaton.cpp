#include "SingularAutomaton.h"

#include <algorithm>

namespace hpnmg {
    SingularAutomaton::Location::Location(LOCATION_ID id,
                                          vector<bool> activitiesDeterministic,
                                          SingularAutomaton::singleton activitiesContinuous,
                                          vector<bool> activitiesGeneral) :
            id(id),
            activitiesDeterministic(activitiesDeterministic),
            activitiesContinuous(activitiesContinuous),
            activitiesGeneral(activitiesGeneral) {
        invariantsDeterministic = rectangularSet(activitiesDeterministic.size(),
                                                 pair<invariantOperator, double>(UNLIMITED, -1));
        invariantsContinuous = rectangularSet(activitiesContinuous.size(),
                                              pair<invariantOperator, double>(UNLIMITED, -1));
    }

    const LOCATION_ID SingularAutomaton::Location::getLocationId() const { return id; }

    const vector<bool>
    SingularAutomaton::Location::getActivitiesDeterministic() const { return activitiesDeterministic; }

    const SingularAutomaton::singleton
    SingularAutomaton::Location::getActivitiesContinuous() const { return activitiesContinuous; }

    const vector<bool> SingularAutomaton::Location::getActivitiesGeneral() const { return activitiesGeneral; }

    const SingularAutomaton::rectangularSet
    SingularAutomaton::Location::getInvariantsDeterministic() const { return invariantsDeterministic; }

    const SingularAutomaton::rectangularSet
    SingularAutomaton::Location::getInvariantsContinuous() const { return invariantsContinuous; }

    const vector<shared_ptr<SingularAutomaton::Transition>>
    SingularAutomaton::Location::getOutgoingTransitions() const { return outgoingTransitions; }

    const vector<shared_ptr<SingularAutomaton::Transition>>
    SingularAutomaton::Location::getIncomingTransitions() const { return incomingTransitions; }

    void SingularAutomaton::Location::addToInvariantDeterministic(long variableIndex, double valuePreCompare,
                                                                  invariantOperator invOperator) {
        invariantsDeterministic[variableIndex] = pair<invariantOperator, double>(invOperator, valuePreCompare);
    }

    void SingularAutomaton::Location::addToInvariantContinuous(long variableIndex, double valuePreCompare,
                                                               invariantOperator invOperator) {
        invariantsContinuous[variableIndex] = pair<invariantOperator, double>(invOperator, valuePreCompare);
    }

    void SingularAutomaton::Location::addOutgoingTransition(shared_ptr<Transition> outgoingTransition) {
        outgoingTransitions.push_back(outgoingTransition);
    }

    void SingularAutomaton::Location::removeOutgoingTransition(shared_ptr<Transition> outgoingTransition) {
        outgoingTransitions.erase(remove(outgoingTransitions.begin(),outgoingTransitions.end(),outgoingTransition),
                outgoingTransitions.end());
    }

    void SingularAutomaton::Location::addIncomingTransition(shared_ptr<Transition> incomingTransition) {
        incomingTransitions.push_back(incomingTransition);
    }

    void SingularAutomaton::Location::removeIncomingTransition(shared_ptr<Transition> incomingTransition) {
        incomingTransitions.erase(remove(incomingTransitions.begin(),incomingTransitions.end(),incomingTransition),
                incomingTransitions.end());
    }

    bool SingularAutomaton::Location::isActInvOutgoingTransitionsIdentic(
            const shared_ptr<SingularAutomaton::Location> other) const {
        // check if the Act and Inv match
        if (this->activitiesDeterministic != other->activitiesDeterministic ||
        this->activitiesContinuous != other->activitiesContinuous ||
        this->activitiesGeneral != other->activitiesGeneral ||
        this->invariantsDeterministic != other->invariantsDeterministic ||
        this->invariantsContinuous != other->invariantsContinuous) {
            return false;
        }
        // check if for every outgoing transition from this, there exists a matching outgoing transition from other
        for (shared_ptr<SingularAutomaton::Transition> firstTransition : this->outgoingTransitions) {
            bool foundMatch = false;
            for (shared_ptr<SingularAutomaton::Transition> secondTransition : other->outgoingTransitions) {
                if (firstTransition->getType() == secondTransition->getType() &&
                        firstTransition->getVariableIndex() == secondTransition->getVariableIndex() &&
                        firstTransition->getValuePreCompare() == secondTransition->getValuePreCompare() &&
                        firstTransition->getSuccessorLocation()->getLocationId()
                        == secondTransition->getSuccessorLocation()->getLocationId()) {
                    foundMatch = true;
                    break;
                }
            }
            if (!foundMatch) {
                return false; // no secondTransition matches firstTransition
            }
        }
        // check the other way round
        for (shared_ptr<SingularAutomaton::Transition> firstTransition : other->outgoingTransitions) {
            bool foundMatch = false;
            for (shared_ptr<SingularAutomaton::Transition> secondTransition : this->outgoingTransitions) {
                if (firstTransition->getType() == secondTransition->getType() &&
                    firstTransition->getVariableIndex() == secondTransition->getVariableIndex() &&
                    firstTransition->getValuePreCompare() == secondTransition->getValuePreCompare() &&
                    firstTransition->getSuccessorLocation()->getLocationId()
                    == secondTransition->getSuccessorLocation()->getLocationId()) {
                    foundMatch = true;
                    break;
                }
            }
            if (!foundMatch) {
                return false; // no secondTransition matches firstTransition
            }
        }
        return true;
    }

    SingularAutomaton::Transition::Transition(shared_ptr<SingularAutomaton::Location> predecessorLocation,
                                              const TransitionType type, const long variableIndex,
                                              const double valuePreCompare,
                                              shared_ptr<SingularAutomaton::Location> successorLocation) :
            predecessorLocation(predecessorLocation),
            type(type),
            variableIndex(variableIndex),
            valuePreCompare(valuePreCompare),
            successorLocation(successorLocation) {}

    const shared_ptr<SingularAutomaton::Location> SingularAutomaton::Transition::getPredecessorLocation() const {return predecessorLocation; }

    const SingularAutomaton::Transition::TransitionType SingularAutomaton::Transition::getType() const { return type; }

    const long SingularAutomaton::Transition::getVariableIndex() const { return variableIndex; }

    const double SingularAutomaton::Transition::getValuePreCompare() const { return valuePreCompare; }

    const shared_ptr<SingularAutomaton::Location>
    SingularAutomaton::Transition::getSuccessorLocation() const { return successorLocation; }

    void SingularAutomaton::Transition::setSuccessorLocation(shared_ptr<Location> newSuccessorLocation) {
        this->successorLocation = newSuccessorLocation;
    };

    SingularAutomaton::SingularAutomaton(shared_ptr<Location> initialLocation,
                                         SingularAutomaton::singleton initialDeterministic,
                                         SingularAutomaton::singleton initialContinuous,
                                         SingularAutomaton::singleton initialGeneral) :
            initialLocation(initialLocation), initialDeterministic(initialDeterministic),
            initialContinuous(initialContinuous), initialGeneral(initialGeneral) {
        addLocation(initialLocation);
    }

    const shared_ptr<SingularAutomaton::Location>
    SingularAutomaton::getInitialLocation() const { return initialLocation; }

    const SingularAutomaton::singleton
    SingularAutomaton::getInitialDeterministic() const { return initialDeterministic; }

    const SingularAutomaton::singleton SingularAutomaton::getInitialContinuous() const { return initialContinuous; }

    const SingularAutomaton::singleton SingularAutomaton::getInitialGeneral() const { return initialGeneral; }

    const vector<shared_ptr<SingularAutomaton::Location>> SingularAutomaton::getLocations() const { return locations; }

    void SingularAutomaton::addLocation(shared_ptr<Location> newLocation) {
        locations.push_back(newLocation);
    }

    void SingularAutomaton::removeLocation(shared_ptr<Location> oldLocation) {
        locations.erase(remove(locations.begin(),locations.end(),oldLocation),locations.end());
    }

    void SingularAutomaton::insertTransition(shared_ptr<SingularAutomaton::Location> predecessorLocation,
                                             const SingularAutomaton::Transition::TransitionType type,
                                             const long variableIndex,
                                             const double valuePreCompare, const invariantOperator invOperator,
                                             shared_ptr<SingularAutomaton::Location> successorLocation) {
        shared_ptr<SingularAutomaton::Transition> newTransition = make_shared<SingularAutomaton::Transition>(
                predecessorLocation, type, variableIndex, valuePreCompare, successorLocation);

        // if transition is immediate, remove the predecessorLocation and instead go directly to successorLocation
        if(type == Immediate) {
            // skip predecessorLocation and go directly to successorLocation
            for (shared_ptr<Transition> incomingTransition : predecessorLocation->getIncomingTransitions()) {
                incomingTransition->setSuccessorLocation(successorLocation);
                successorLocation->addIncomingTransition(incomingTransition);
                predecessorLocation->removeIncomingTransition(incomingTransition);
            }
            if (initialLocation == predecessorLocation) {
                initialLocation = successorLocation;
            }
            // remove predecessorLocation from the automaton
            removeLocation(predecessorLocation);
        }
        else {
            predecessorLocation->addOutgoingTransition(newTransition);
            successorLocation->addIncomingTransition(newTransition);

            // update invariant of predecessor Location
            switch (type) {
                case Timed:
                    predecessorLocation->addToInvariantDeterministic(variableIndex, valuePreCompare, invOperator);
                    break;
                case Continuous:
                    predecessorLocation->addToInvariantContinuous(variableIndex, valuePreCompare, invOperator);
                    break;
                default:
                    // do nothing
                    break;
            }
        }
    }
}