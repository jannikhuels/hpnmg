#pragma once

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <memory>
#include <limits>
#include "Event.h"



namespace hpnmg {
    using namespace std;
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
        vector<pair<string, map<string, float>>> distributionsNormalized;

        class Transition;

        class Location {
        private:
            const LOCATION_ID id;
            const vector<bool> activitiesDeterministic;
            const singleton activitiesContinuous;
            vector<short int> activitiesGeneral;
            rectangularSet invariantsDeterministic;
            rectangularSet invariantsContinuous;
            rectangularSet invariantsGeneral;
            vector<shared_ptr<SingularAutomaton::Transition>> incomingTransitions;
            vector<shared_ptr<SingularAutomaton::Transition>> outgoingTransitions;
            bool toBeDeleted = false;


        public:
            // Generates a location
            //
            // @param id                            id of the location
            // @param activitiesDeterministic       activities for for variables, modeling deterministic clocks
            // @param activitiesContinuous          activities for for variables, modeling the continuous marking
            // @param activitiesGeneral             activities for for variables, modeling general clocks
            Location(LOCATION_ID id, vector<bool> activitiesDeterministic, singleton activitiesContinuous,
                     vector<short int> activitiesGeneral);

            const LOCATION_ID getLocationId() const;

            const vector<bool> getActivitiesDeterministic() const;

            const singleton getActivitiesContinuous() const;

            const vector<short int> getActivitiesGeneral() const;

            const rectangularSet getInvariantsDeterministic() const;

            const rectangularSet getInvariantsContinuous() const;

            const rectangularSet getInvariantsGeneral() const;

            const vector<shared_ptr<SingularAutomaton::Transition>> getOutgoingTransitions() const;

            const vector<shared_ptr<SingularAutomaton::Transition>> getIncomingTransitions() const;

            void addToInvariantDeterministic(long variableIndex, double valuePreCompare, invariantOperator invOperator);

            void addToInvariantContinuous(long variableIndex, double valuePreCompare, invariantOperator invOperator);

            void addToInvariantGeneral(long variableIndex, double valuePreCompare, invariantOperator invOperator);

            void addOutgoingTransition(shared_ptr<SingularAutomaton::Transition> outgoingTransition);

            void removeOutgoingTransition(shared_ptr<SingularAutomaton::Transition> outgoingTransition);

            void addIncomingTransition(shared_ptr<SingularAutomaton::Transition> incomingTransition);

            void removeIncomingTransition(shared_ptr<SingularAutomaton::Transition> incomingTransition);

            bool isActInvOutgoingTransitionsIdentic(const shared_ptr<SingularAutomaton::Location> other) const;

            bool isToBeDeleted();

            void setToBeDeleted();

            void overwriteActivitiesGeneral(vector<short int> actGen);
        };

        class Transition {
        public:
            typedef EventType TransitionType;

        private:
            const shared_ptr<Location> predecessorLocation;
            const TransitionType type; // indicates, whether a deterministic clock, a continuous marking
                                       // or a general value is compared and/or reset
            const long variableIndex; // indices of the variables, which is compared and/or reset
            const double valueGuardCompare; // the value to compare to
            shared_ptr<Location> successorLocation;
            vector<long> samplingVariables;


        public:
            Transition(shared_ptr<SingularAutomaton::Location> predecessorLocation, const TransitionType type,
                       const long variableIndex, const double valueGuardCompare,
                       shared_ptr<SingularAutomaton::Location> successorLocation);

            const shared_ptr<Location> getPredecessorLocation() const;

            const TransitionType getType() const;

            const long getVariableIndex() const;

            const double getValueGuardCompare() const;

            const vector<long> getSamplingVariables() const;

            const shared_ptr<Location> getSuccessorLocation() const;

            void setSuccessorLocation(shared_ptr<Location> newSuccessorLocation);

            void addSamplingVariable(long index);
        };

    private:
        vector<shared_ptr<Location>> initialLocations;
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
        SingularAutomaton(vector<shared_ptr<Location>> initialLocations, singleton initialDeterministic,
                          singleton initialContinuous, singleton initialGeneral);

        const vector<shared_ptr<SingularAutomaton::Location>> getInitialLocations() const;

        const SingularAutomaton::singleton getInitialDeterministic() const;

        const SingularAutomaton::singleton getInitialContinuous() const;

        const SingularAutomaton::singleton getInitialGeneral() const;

        const vector<shared_ptr<Location>> getLocations() const;

        // adds the location newLocation to the automaton
        void addLocation(shared_ptr<Location> newLocation);

        // removes the location oldLocation from the automaton
        void removeLocation(shared_ptr<Location> oldLocation);

        // inserts transition
        shared_ptr<SingularAutomaton::Transition> insertTransition(shared_ptr<SingularAutomaton::Location> predecessorLocation,
                              const SingularAutomaton::Transition::TransitionType type, const long variableIndex,
                              const double valuePreCompare, const invariantOperator invOperator,
                              shared_ptr<SingularAutomaton::Location> successorLocation);

        //checks if Location is initialLocation
        bool isInitialLocation(shared_ptr<SingularAutomaton::Location> Location);

        void overwriteInitialLocations(vector<shared_ptr<Location>> locations);

        vector<pair<string, map<string, float>>> getDistributionsNormalized();

        void setDistributionsNormalized(vector<pair<string, map<string, float>>> distributions);
    };
}