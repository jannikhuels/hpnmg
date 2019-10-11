#pragma once

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <memory>
#include <limits>
#include "Event.h"


using namespace std;

namespace hpnmg {
    typedef int LOCATION_ID;

    enum invariantOperator {
        LOWER_EQUAL = -1,
        UNLIMITED = 0,
        GREATER_EQUAL = 1
    };

    class SingularAutomaton {
    public:
        typedef vector<pair<invariantOperator, double>> rectangularSet;
        typedef vector<double> singleton;

        class Transition;

        class Location {
        private:
            const LOCATION_ID id;
            const vector<bool> activitiesDeterministic;
            const singleton activitiesContinuous;
            const vector<bool> activitiesGeneral;
            rectangularSet invariantsDeterministic;
            rectangularSet invariantsContinuous;
            vector<shared_ptr<SingularAutomaton::Transition>> incomingTransitions;
            vector<shared_ptr<SingularAutomaton::Transition>> outgoingTransitions;

        public:
            // Generates a location
            //
            // @param id                            id of the location
            // @param activitiesDeterministic       activities for for variables, modeling deterministic clocks
            // @param activitiesContinuous          activities for for variables, modeling the continuous marking
            // @param activitiesGeneral             activities for for variables, modeling general clocks
            Location(LOCATION_ID id, vector<bool> activitiesDeterministic, singleton activitiesContinuous,
                     vector<bool> activitiesGeneral);

            const LOCATION_ID getLocationId() const;

            const vector<bool> getActivitiesDeterministic() const;

            const singleton getActivitiesContinuous() const;

            const vector<bool> getActivitiesGeneral() const;

            const rectangularSet getInvariantsDeterministic() const;

            const rectangularSet getInvariantsContinuous() const;

            const vector<shared_ptr<SingularAutomaton::Transition>> getOutgoingTransitions() const;

            const vector<shared_ptr<SingularAutomaton::Transition>> getIncomingTransitions() const;

            void addToInvariantDeterministic(long variableIndex, double valuePreCompare, invariantOperator invOperator);

            void addToInvariantContinuous(long variableIndex, double valuePreCompare, invariantOperator invOperator);

            void addOutgoingTransition(shared_ptr<SingularAutomaton::Transition> outgoingTransition);

            void removeOutgoingTransition(shared_ptr<SingularAutomaton::Transition> outgoingTransition);

            void addIncomingTransition(shared_ptr<SingularAutomaton::Transition> incomingTransition);

            void removeIncomingTransition(shared_ptr<SingularAutomaton::Transition> incomingTransition);

            bool isActInvOutgoingTransitionsIdentic(const shared_ptr<SingularAutomaton::Location> other) const;
        };

        class Transition {
        public:
            typedef EventType TransitionType;

        private:
            const shared_ptr<Location> predecessorLocation;
            const TransitionType type; // indicates, whether a deterministic clock, a continuous marking
                                       // or a general value is compared and/or reset
            const long variableIndex; // index of the variable, which is compared and/or reset
            const double valuePreCompare; // the value to compare to
            shared_ptr<Location> successorLocation;

        public:
            Transition(shared_ptr<SingularAutomaton::Location> predecessorLocation, const TransitionType type,
                       const long variableIndex, const double valuePreCompare,
                       shared_ptr<SingularAutomaton::Location> successorLocation);

            const shared_ptr<Location> getPredecessorLocation() const;

            const TransitionType getType() const;

            const long getVariableIndex() const;

            const double getValuePreCompare() const;

            const shared_ptr<Location> getSuccessorLocation() const;

            void setSuccessorLocation(shared_ptr<Location> newSuccessorLocation);
        };

    private:
        shared_ptr<Location> initialLocation;
        const singleton initialDeterministic;
        const singleton initialContinuous;
        const singleton initialGeneral;
        vector<shared_ptr<Location>> locations;

    public:
        // Generates a singular automaton with the given initial Location and initializes it
        //
        // @param initialLocation           initial Location
        // @param initialDeterministic      initial values for variables, modeling deterministic clocks
        // @param initialContinuous         initial values for variables, modeling the continuous marking
        // @param initialGeneral            initial values for variables, modeling general clocks
        SingularAutomaton(shared_ptr<Location> initialLocation, singleton initialDeterministic,
                          singleton initialContinuous, singleton initialGeneral);

        const shared_ptr<SingularAutomaton::Location> getInitialLocation() const;

        const SingularAutomaton::singleton getInitialDeterministic() const;

        const SingularAutomaton::singleton getInitialContinuous() const;

        const SingularAutomaton::singleton getInitialGeneral() const;

        const vector<shared_ptr<Location>> getLocations() const;

        // adds the location newLocation to the automaton
        void addLocation(shared_ptr<Location> newLocation);

        // removes the location oldLocation from the automaton
        void removeLocation(shared_ptr<Location> oldLocation);

        // inserts transition
        void insertTransition(shared_ptr<SingularAutomaton::Location> predecessorLocation,
                              const SingularAutomaton::Transition::TransitionType type, const long variableIndex,
                              const double valuePreCompare, const invariantOperator invOperator,
                              shared_ptr<SingularAutomaton::Location> successorLocation);
    };
}