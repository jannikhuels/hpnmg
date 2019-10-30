#pragma once

#include "datastructures/arcs/GuardArc.h"
#include "datastructures/places/ContinuousPlace.h"
#include "datastructures/transitions/ImmediateTransition.h"
#include "datastructures/transitions/GeneralTransition.h"
#include "datastructures/transitions/DeterministicTransition.h"

#include <memory>
#include <string>
#include <vector>

namespace hpnmg {

    enum EventType {
        Immediate = 1,
        General = 2,
        Timed = 3,
        Continuous = 4,
        Root = 5,
    };

    class Event {
    private:
        EventType type;
        std::vector<double> generalDependencies;
        std::vector<double> generalDependenciesNormed;
        double time;
        shared_ptr<ImmediateTransition> immediateTransitionMember;
        shared_ptr<GeneralTransition> generalTransitionMember;
        shared_ptr<DeterministicTransition> deterministicTransitionMember;
        shared_ptr<GuardArc> arcMember;
        shared_ptr<ContinuousPlace> placeMember;

    public:
        Event(EventType type, std::vector<double> generalDependencies, double time);
        Event();
        Event(int numberOfGeneralTransitions);
        Event(const Event&) = default;

        EventType getEventType() const;
        void setEventType(const EventType &eventType);

        std::vector<double> getGeneralDependencies() const;
        void setGeneralDependencies(const std::vector<double> generalDependencies);

        std::vector<double> getGeneralDependenciesNormed() const;
        void setGeneralDependenciesNormed(const std::vector<double> generalDependencies);

        double getTime() const;
        void setTime(double time);

        shared_ptr<ImmediateTransition> getImmediateTransitionMember() const;
        void setImmediateTransitionMember(shared_ptr<ImmediateTransition> immediateTransitionMember);

        shared_ptr<GeneralTransition> getGeneralTransitionMember() const;
        void setGeneralTransitionMember(shared_ptr<GeneralTransition> generalTransitionMember);

        shared_ptr<DeterministicTransition> getDeterministicTransitionMember() const;
        void setDeterministicTransitionMember(shared_ptr<DeterministicTransition> deterministicTransitionMember);

        shared_ptr<GuardArc> getArcMember() const;
        void setArcMember(shared_ptr<GuardArc> arcMember);

        shared_ptr<ContinuousPlace> getPlaceMember() const;
        void setPlaceMember(shared_ptr<ContinuousPlace> placeMember);

        std::vector<double> getTimeVector(int dimension);

        void print() const;

        std::string getMemberID();
    };
}
