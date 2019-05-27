#pragma once

#include <vector>
#include "datastructures/HybridPetrinet.h"
#include "representations/GeometricObject.h"

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
        Event(const Event &event);

        EventType getEventType() const;
        void setEventType(const EventType &eventType);

        std::vector<double> getGeneralDependencies() const;
        void setGeneralDependencies(const std::vector<double> generalDependencies);

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
